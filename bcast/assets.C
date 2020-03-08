#include <string.h>
#include "assets.h"
#include "cache.h"
#include "file.h"
#include "filehtal.h"
#include "indexfile.h"
#include "mainwindow.h"
#include "quicktime.h"
#include "threadindexer.h"

Assets::Assets() : List<Asset>()
{
	thread_indexer_running = 0;
}

Assets::~Assets()
{
	delete_all();
	delete thread_indexer;
}

int Assets::create_objects(MainWindow *mwindow)
{
	this->mwindow = mwindow;
	thread_indexer = new ThreadIndexer(mwindow, this);
return 0;
}

int Assets::delete_all()
{
	while(last)
	{
		mwindow->cache->delete_entry(last);
		remove(last);
	}
return 0;
}

Asset* Assets::update(const char *path)
{
	Asset* current = first;

	while(current)
	{
		if(current->test_path(path)) 
		{
			return current;
		}
		current = NEXT;
	}

	return append(new Asset(path));
}

Asset* Assets::update(Asset *new_asset)
{
	Asset* current = first;

	if(!new_asset) return 0;
	while(current)
	{
// if an asset for this path already exists, skip it
		if(current->test_path(new_asset->path)) 
		{
//			*current = *new_asset;
			return current;
		}
		current = NEXT;
	}

	current = new Asset(new_asset->path);
	*current = *new_asset;
// append after copying data since linklist owner gets copied
	append(current);
	return current;
}

Asset* Assets::get_asset(const char *filename)
{
	Asset* current = first;
	Asset* result = 0;

	while(current && !result)
	{
		if(current->test_path(filename))
		{
			result = current;
		}
		current = current->next;
	}

	return result;	
}

int Assets::build_indexes()
{
	if(!thread_indexer_running)
	{
		thread_indexer_running = 1;
		thread_indexer->start_build();
	}
return 0;
}

int Assets::interrupt_indexes()
{
	if(thread_indexer_running)
	{
		thread_indexer_running = 0;
		thread_indexer->interrupt_build();
	}
return 0;
}

int Assets::number_of(Asset *asset)
{
	int i;
	Asset *current;

	for(i = 0, current = first; current && current != asset; i++, current = NEXT)
		;

	return i;
return 0;
}

Asset* Assets::asset_number(int number)
{
	int i;
	Asset *current;

	for(i = 0, current = first; i < number && current; i++, current = NEXT)
		;
	
	return current;
}

int Assets::update_old_filename(char *old_filename, char *new_filename)
{
	int silence;
	
	for(Asset* current = first; current; current = NEXT)
	{
		if(!strcmp(current->path, old_filename))
		{
			current->update_path(new_filename);
		}
	}
return 0;
}

int Assets::load(FileHTAL *htal)
{
	int result= 0;
	
	while(!result)
	{
		result = htal->read_tag();
		if(!result)
		{
			if(htal->tag.title_is("/ASSETS"))
			{
				result = 1;
			}
			else
			if(htal->tag.title_is("ASSET"))
			{
				char *path = htal->tag.get_property("SRC");
				Asset new_asset(path ? path : SILENCE);
				new_asset.read(mwindow, htal);
				update(&new_asset);
			}
		}
	}
return 0;
}


int Assets::save(FileHTAL *htal, int use_relative_path)
{
	htal->tag.set_title("ASSETS");
	htal->append_tag();
	htal->append_newline();

	for(Asset* current = first; current; current = NEXT)
	{
		current->write(mwindow, htal, 0, use_relative_path);
	}
	
	htal->tag.set_title("/ASSETS");
	htal->append_tag();
	htal->append_newline();	
return 0;
}

int Assets::dump()
{
	printf("Asset dump\n");
	for(Asset *current = first; current; current = NEXT)
	{
		printf("	%x %s\n", current, current->path);
		printf("	index_status %d\n", current->index_status);
		printf("	format %d\n", current->format);
	}
return 0;
}


// ==========================================================

Asset::Asset()
{
	init_values();
}


Asset::Asset(const char *path) : ListItem<Asset>()
{
	init_values();
	strcpy(this->path, path);
	if(!strcmp(path, SILENCE)) silence = 1; else silence = 0;
}

Asset::Asset(const int plugin_type, const char *plugin_title) : ListItem<Asset>()
{
	init_values();
	this->plugin_type = plugin_type;
	transition = 1;
	strcpy(this->plugin_title, plugin_title);
}

int Asset::init_values()
{
	plugin_title[0] = 0;
	path[0] = 0;
	silence = 0;
	transition = audio_data = video_data = 0;
	format = channels = rate = bits = byte_order = signed_ = header = 0;

	layers = 0;
	frame_rate = 0;
	width = 0;
	height = 0;
	strcpy(compression, QUICKTIME_YUV2);
	quality = 100;

	transition = 0;
	plugin_type = 0;

	reset_index();
return 0;
}

int Asset::reset_index()
{
	index_status = 1;
	index_start = old_index_end = index_end = 0;
	index_offsets = 0;
	index_zoom = 0;
	index_bytes = 0;
	index_buffer = 0;
return 0;
}

Asset& Asset::operator=(Asset &asset)
{
	if(index_offsets) delete [] index_offsets;
	if(index_buffer) delete index_buffer;

	index_offsets = 0;
	index_buffer = 0;
	strcpy(this->path, asset.path);
	index_status = asset.index_status;
	index_start = asset.index_start;
	index_end = asset.index_end;
	index_zoom = asset.index_zoom;
	index_bytes = asset.index_bytes;
	old_index_end = asset.old_index_end;

	audio_data = asset.audio_data;
	format = asset.format;
	channels = asset.channels;
	rate = asset.rate;
	bits = asset.bits;
	byte_order = asset.byte_order;
	signed_ = asset.signed_;
	header = asset.header;

	video_data = asset.video_data;
	layers = asset.layers;
	frame_rate = asset.frame_rate;
	width = asset.width;
	height = asset.height;
	quality = asset.quality;
	strcpy(compression, asset.compression);
	if(!strcmp(path, SILENCE)) silence = 1; else silence = 0;
	if(asset.index_offsets && audio_data)
	{
		index_offsets = new long[channels];
		for(int i = 0; i < channels; i++)
		{
			index_offsets[i] = asset.index_offsets[i];
		}
	}

	strcpy(plugin_title, asset.plugin_title);
	transition = asset.transition;
	plugin_type = asset.plugin_type;
	shared_plugin_location = asset.shared_plugin_location;
	shared_module_location = asset.shared_module_location;
	return *this;
}

int Asset::operator==(Asset &asset)
{
	if(((!transition && !strcmp(asset.path, path) && asset.silence == silence) || 
		transition) &&

		format == asset.format && 
		channels == asset.channels && 
		rate == asset.rate && 
		bits == asset.bits && 
		byte_order == asset.byte_order && 
		signed_ == asset.signed_ && 
		header == asset.header && 

		layers == asset.layers && 
		frame_rate == asset.frame_rate &&
		quality == asset.quality &&
		width == asset.width &&
		height == asset.height &&
		!strcmp(compression, asset.compression) &&

		transition == asset.transition &&
		!strcmp(plugin_title, asset.plugin_title) &&
		plugin_type == asset.plugin_type &&
		shared_plugin_location == asset.shared_plugin_location &&
		shared_module_location == asset.shared_module_location
	) return 1;
	else
	return 0;
return 0;
}

int Asset::operator!=(Asset &asset)
{
	return !(*this == asset);
return 0;
}

Asset::~Asset()
{
	if(index_offsets) delete [] index_offsets;
	if(index_buffer) delete index_buffer;
}


int Asset::test_path(const char *path)
{
	char old_name[1024], new_name[1024];
	FileSystem fs;
	
	fs.extract_name(old_name, this->path);
	fs.extract_name(new_name, path);

	if(!strcmp(old_name, new_name)) return 1; else return 0;
return 0;
}

int Asset::test_plugin_title(const char *path)
{
	if(!strcmp(this->plugin_title, path)) return 1; else return 0;
return 0;
}

int Asset::read(MainWindow *mwindow, FileHTAL *htal)
{
	int result = 0;

// Asset is not silence.  Check for relative path.
	if(strcmp(path, SILENCE))
	{
		char new_path[1024];
		strcpy(new_path, path);
		char asset_directory[1024], input_directory[1024];
		FileSystem fs;
		fs.set_current_dir("");

		fs.extract_dir(asset_directory, path);

// No path in asset
		if(!asset_directory[0])
		{
			fs.extract_dir(input_directory, htal->filename);

// Input file has a path
			if(input_directory[0])
			{
				sprintf(path, "%s/%s", input_directory, new_path);
			}
		}
	}

	while(!result)
	{
		result = htal->read_tag();
		if(!result)
		{
			if(htal->tag.title_is("/ASSET"))
			{
				result = 1;
			}
			else
			if(htal->tag.title_is("AUDIO"))
			{
				read_audio(htal);
			}
			else
			if(htal->tag.title_is("FORMAT"))
			{
				File file;
				char *string = htal->tag.get_property("TYPE");
				format = file.strtoformat(mwindow->plugindb, string);
			}
			else
			if(htal->tag.title_is("VIDEO"))
			{
				read_video(htal);
			}
			else
			if(htal->tag.title_is("INDEX"))
			{
				read_index(htal);
			}
		}
	}
return 0;
}

int Asset::read_audio(FileHTAL *htal)
{
	channels = htal->tag.get_property("CHANNELS", 2);
	rate = htal->tag.get_property("RATE", 44100);
	bits = htal->tag.get_property("BITS", 16);
	byte_order = htal->tag.get_property("BYTE_ORDER", 1);
	signed_ = htal->tag.get_property("SIGNED", 1);
	header = htal->tag.get_property("HEADER", 0);
	audio_data = 1;
return 0;
}

int Asset::read_video(FileHTAL *htal)
{
	height = htal->tag.get_property("HEIGHT", height);
	width = htal->tag.get_property("WIDTH", width);
	layers = htal->tag.get_property("LAYERS", layers);
	frame_rate = htal->tag.get_property("FRAMERATE", frame_rate);
	compression[0] = 0;
	htal->tag.get_property("COMPRESSION", compression);
	quality = htal->tag.get_property("QUALITY", quality);
	video_data = 1;
return 0;
}

int Asset::read_index(FileHTAL *htal)
{
	if(index_offsets) delete [] index_offsets;
	index_offsets = new long[channels];
	for(int i = 0; i < channels; i++) index_offsets[i] = 0;
	int current_offset = 0;
	int result = 0;

	index_zoom = htal->tag.get_property("ZOOM", 1);
	index_bytes = htal->tag.get_property("BYTES", (longest)0);

	while(!result)
	{
		result = htal->read_tag();
		if(!result)
		{
			if(htal->tag.title_is("/INDEX"))
			{
				result = 1;
			}
			else
			if(htal->tag.title_is("OFFSET"))
			{
				if(current_offset < channels)
				{
					index_offsets[current_offset++] = htal->tag.get_property("FLOAT", 0);
				}
			}
		}
	}
return 0;
}


int Asset::write(MainWindow *mwindow, FileHTAL *htal, int include_index, int use_relative_path)
{
	char new_path[1024];

	if(use_relative_path)
	{
		char asset_directory[1024], output_directory[1024];
		FileSystem fs;

		fs.extract_dir(asset_directory, path);
		fs.extract_dir(output_directory, mwindow->filename);
		if(!strcmp(asset_directory, output_directory))
		{
			fs.extract_name(new_path, path);
		}
		else
		{
			strcpy(new_path, path);
		}
	}
	else
	{
		strcpy(new_path, path);
	}

	htal->tag.set_title("ASSET");
	htal->tag.set_property("SRC", new_path);
	htal->append_tag();
	htal->append_newline();

// Write the format information
	htal->tag.set_title("FORMAT");

	{
		File file;
		htal->tag.set_property("TYPE", file.formattostr(mwindow->plugindb, format));
	}

	htal->append_tag();
	htal->append_newline();

	if(audio_data) write_audio(htal);
	if(video_data) write_video(htal);
	if(index_status == 0 && include_index) write_index(htal);  // index goes after source

	htal->tag.set_title("/ASSET");
	htal->append_tag();
	htal->append_newline();
	return 0;
return 0;
}

int Asset::write_audio(FileHTAL *htal)
{
	htal->tag.set_title("AUDIO");
	htal->tag.set_property("CHANNELS", channels);
	htal->tag.set_property("RATE", rate);
	htal->tag.set_property("BITS", bits);
	htal->tag.set_property("BYTE_ORDER", byte_order);
	htal->tag.set_property("SIGNED", signed_);
	htal->tag.set_property("HEADER", header);	
	htal->append_tag();
	htal->append_newline();
	return 0;
return 0;
}

int Asset::write_video(FileHTAL *htal)
{
	htal->tag.set_title("VIDEO");
	htal->tag.set_property("HEIGHT", height);
	htal->tag.set_property("WIDTH", width);
	htal->tag.set_property("LAYERS", layers);
	htal->tag.set_property("FRAMERATE", frame_rate);
	if(compression[0])
		htal->tag.set_property("COMPRESSION", compression);
	htal->tag.set_property("QUALITY", quality);
	htal->append_tag();
	htal->append_newline();
	return 0;
return 0;
}

int Asset::write_index(FileHTAL *htal)
{
	htal->tag.set_title("INDEX");
	htal->tag.set_property("ZOOM", index_zoom);
	htal->tag.set_property("BYTES", index_bytes);
	htal->append_tag();
	htal->append_newline();

	if(index_offsets)
	{
		for(int i = 0; i < channels; i++)
		{
			htal->tag.set_title("OFFSET");
			htal->tag.set_property("FLOAT", index_offsets[i]);
			htal->append_tag();
		}
	}

	htal->append_newline();
	htal->tag.set_title("/INDEX");
	htal->append_tag();
	htal->append_newline();
	return 0;
return 0;
}

int Asset::write(MainWindow *mwindow, FILE *file, int include_index, int use_relative_path)
{
	FileHTAL htal;
	write(mwindow, &htal, include_index, use_relative_path);
	htal.write_to_file(file);
	return 0;
return 0;
}

int Asset::update_path(char *new_path)
{
	if(!strcmp(new_path, SILENCE)) silence = 1; else silence = 0;
	strcpy(path, new_path);
	
	if(silence)
	{
		audio_data = 0;
		video_data = 0;
	}
	return 0;
return 0;
}

int Asset::dump()
{
	printf("asset::dump\n");
	printf("	audio_data %d format %d channels %d samplerate %d bits %d byte_order %d signed %d header %d\n",
		audio_data, format, channels, rate, bits, byte_order, signed_, header);
	printf("	video_data %d layers %d framerate %f width %d height %d compression %c%c%c%c\n",
		video_data, layers, frame_rate, width, height, compression[0], compression[1], compression[2], compression[3]);
	return 0;
return 0;
}
