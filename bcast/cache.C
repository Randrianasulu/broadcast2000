#include <string.h>
#include "assets.h"
#include "cache.h"
#include "datatype.h"
#include "file.h"
#include "mainwindow.h"
#include "preferences.h"

Cache::Cache(MainWindow *mwindow)
 : List<CacheItem>()
{
	this->mwindow = mwindow;
}

Cache::~Cache()
{
	while(last) delete last;
}



File* Cache::check_out(Asset *asset)
{
	File *result = 0;

	if(!asset->silence)
	{
		check_out_lock.lock();

// search for it in the cache
		CacheItem *current, *new_item = 0;

		for(current = first; current && !new_item; current = NEXT)
		{
			if(current->asset == asset)
			{
				current->counter = 0;
				new_item = current;
			}
		}

// didn't find it so create a new one
		if(!new_item) new_item = append(new CacheItem(this, asset));

		if(new_item)
		{
			if(new_item->file)
			{
// opened successfully
				new_item->item_lock.lock();
				new_item->checked_out = 1;

				result = new_item->file;
			}
			else
			{
// failed
				delete new_item;
				new_item = 0;
			}
		}

		check_out_lock.unlock();
	}

	return result;
}

int Cache::check_in(Asset *asset)
{
	check_in_lock.lock();

	CacheItem *current;
	int result = 0;
	total_lock.lock();
	for(current = first; current && !result; current = NEXT)
	{
		if(current->asset == asset)
		{
			current->checked_out = 0;
			current->item_lock.unlock();
			result = 1;
		}
	}
	total_lock.unlock();

	check_in_lock.unlock();
	return result;
return 0;
}


int Cache::delete_entry(Asset *asset)
{
	lock_all();
	int result = 0;
	CacheItem *current, *temp;

	for(current = first; current; current = temp)
	{
		temp = NEXT;
		if(current->asset == asset && !current->checked_out) delete current;
		current = temp;
	}
	unlock_all();
return 0;
}

int Cache::age_audio()
{
	age_type(TRACK_AUDIO);
return 0;
}


int Cache::age_video()
{
	age_type(TRACK_VIDEO);
return 0;
}

int Cache::age_type(int data_type)
{
	check_out_lock.lock();
	CacheItem *current;

	for(current = first; current; current = NEXT)
	{
		switch(data_type)
		{
			case TRACK_AUDIO:
				if(current->asset->audio_data)
				{
					current->counter++;
				}
				break;
			
			case TRACK_VIDEO:	
				if(current->asset->video_data)
				{
					current->counter++;
				}
				break;
		}
	}

// delete old assets if memory usage is exceeded
	long memory_usage;
	int result = 0;
	do
	{
		memory_usage = get_memory_usage();
		
		if(memory_usage > mwindow->preferences->cache_size)
		{
			result = delete_oldest();
		}
	}while(memory_usage > mwindow->preferences->cache_size && !result);

	check_out_lock.unlock();
return 0;
}

long Cache::get_memory_usage()
{
	CacheItem *current;
	long result = 0;
	
	for(current = first; current; current = NEXT)
	{
		result++;
	}
	
	return result;
}

int Cache::delete_oldest()
{
	CacheItem *current;
	int highest_counter = 1;
	CacheItem *oldest = 0;

	for(current = last; current; current =  PREVIOUS)
	{
		if(current->counter >= highest_counter)
		{
			oldest = current;
			highest_counter = current->counter;
		}
	}

	if(highest_counter > 1 && oldest && !oldest->checked_out)
	{
		total_lock.lock();
		delete oldest;
		total_lock.unlock();
		return 0;    // success
	}
	else
	{
		return 1;    // nothing was old enough to delete
	}
return 0;
}

int Cache::dump()
{
	lock_all();
	CacheItem *current;

	for(current = first; current; current = NEXT)
	{
		printf("cache item %x\n", current);
		printf("	asset %x\n", current->asset);
		printf("	%s\n", current->asset->path);
		printf("	size %ld\n", current->get_memory_usage());
		printf("	counter %ld\n", current->counter);
	}
	
	printf("total size %ld\n", get_memory_usage());
	unlock_all();
return 0;
}

int Cache::lock_all()
{
	check_in_lock.lock();
	check_out_lock.lock();
return 0;
}

int Cache::unlock_all()
{
	check_in_lock.unlock();
	check_out_lock.unlock();
return 0;
}












CacheItem::CacheItem(Cache *cache, Asset *asset)
 : ListItem<CacheItem>()
{
	int result = 0;
	counter = 0;
	this->asset = asset;
	this->cache = cache;
	checked_out = 0;

	file = new File;
	file->set_processors(cache->mwindow->preferences->smp ? 2: 1);
	file->set_preload(cache->mwindow->preferences->playback_preload);
	if(result = file->try_to_open_file(cache->mwindow->plugindb, asset, 1, 0))
	{
		delete file;
		file = 0;
	}
}

CacheItem::~CacheItem()
{
	delete file;
}

long CacheItem::get_memory_usage()
{
	return file->get_memory_usage();
}
