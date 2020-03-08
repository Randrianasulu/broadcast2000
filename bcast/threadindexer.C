#include <string.h>
#include "assets.h"
#include "filesystem.h"
#include "indexfile.h"
#include "loadfile.h"
#include "bcbase.h"
#include "mainwindow.h"
#include "progressbox.h"
#include "threadindexer.h"


ThreadIndexer::ThreadIndexer(MainWindow *mwindow, Assets *assets)
 : Thread()
{
	this->mwindow = mwindow;
	this->assets = assets;
	synchronous = 0;
	indexfile = new IndexFile(mwindow);
}

ThreadIndexer::~ThreadIndexer()
{
	delete indexfile;
}

int ThreadIndexer::start_build()
{
	interrupt_lock.lock();
	interrupt_flag = 0;
	start();
return 0;
}

// build all here to allow editing during build
void ThreadIndexer::run()
{
// check locations of each asset
	FILE *test_file;
	Asset *current_asset;
	char new_filename[1024], old_filename[1024];
	int result = 0;
	ProgressBox *progress = 0;

	for(current_asset = assets->first; current_asset; current_asset = current_asset->next)
	{
		if(!current_asset->silence)
		{
			if(!(test_file = fopen(current_asset->path, "rb")))
			{
// file doesn't exist
				strcpy(old_filename, current_asset->path);

				result = 1;
// get location from user
				char directory[1024];
				sprintf(directory, "~");
				mwindow->defaults->get("DIRECTORY", directory);

				char string[1024], name[1024];
				FileSystem dir;
				dir.extract_name(name, old_filename);
				sprintf(string, "Where is %s?", name);

				LocateFileWindow window(mwindow, mwindow->display, directory, string);
				window.create_objects();
				int result2 = window.run_window();

				mwindow->defaults->update("DIRECTORY", window.get_directory());

				if(result2 == 1 )
				{
					strcpy(new_filename, SILENCE);
				}
				else
				{
					strcpy(new_filename, window.get_filename());
				}

// update assets containing old location
				assets->update_old_filename(old_filename, new_filename);
			}
			else
			{
				fclose(test_file);
			}
		}
	}

// need to redraw with new file locations before testing indexes
	if(result)
	{
		mwindow->draw();
	}

// test index of each asset
	for(current_asset = assets->first; 
		current_asset && !interrupt_flag; 
		current_asset = current_asset->next)
	{
// test for an index file already built before creating progress bar
		if(!current_asset->silence && current_asset->index_status == 1 && current_asset->audio_data)
		{
			if(indexfile->open_index(mwindow, current_asset))
			{
// doesn't exist
// try to create now
				if(!progress)
				{
					progress = new ProgressBox("", "Building Indexes...", 1, 1);
					progress->start();
				}

				indexfile->create_index(mwindow, current_asset, progress);
				if(progress->cancelled()) interrupt_flag = 1;
			}
			else
			{
				if(current_asset->index_status == 1) current_asset->index_status = 0;   // index has been tested
				indexfile->close_index();
			}
		}
	}

	if(progress)     // progress box is only createdd when an index is built
	{	
		progress->stop_progress();
		delete progress;
		progress = 0;
	}

	assets->thread_indexer_running = 0;
	interrupt_lock.unlock();
}

int ThreadIndexer::interrupt_build()
{
	interrupt_flag = 1;
	indexfile->interrupt_index();
	interrupt_lock.lock();
	interrupt_lock.unlock();
return 0;
}

