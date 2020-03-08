#ifndef ASSETS_H
#define ASSETS_H

#include <stdio.h>

#include "assets.inc"
#include "filehtal.inc"
#include "linklist.h"
#include "mainwindow.inc"
#include "sharedpluginlocation.h"
#include "sizes.h"
#include "threadindexer.inc"

class Assets : public List<Asset>
{
public:
	Assets();
	~Assets();

	int create_objects(MainWindow *mwindow);
	int load(FileHTAL *htal);
	int save(FileHTAL *htal, int use_relative_path);
	int delete_all();
	int dump();

// return the asset containing this path or create a new asset containing this path
	Asset* update(const char *path);

// return the asset containing this asset's path or create a new asset as a copy
// the asset argument is not deleted if a new one is returned
	Asset* update(Asset *asset);

// return the asset containing this path or 0 if not found
	Asset* get_asset(const char *filename);

// return number of the asset
	int number_of(Asset *asset);
	Asset* asset_number(int number);

	int update_old_filename(char *old_filename, char *new_filename);
	int build_indexes();
	int interrupt_indexes();

	MainWindow *mwindow;
	ThreadIndexer *thread_indexer;
	int thread_indexer_running;
};



class Asset : public ListItem<Asset>
{
public:
	Asset();
	Asset(const char *path);
	Asset(const int plugin_type, const char *plugin_path);
	~Asset();

	int init_values();
	int dump();

	Asset& operator=(Asset &asset);
	int operator==(Asset &asset);
	int operator!=(Asset &asset);
	int test_path(const char *path);
	int test_plugin_title(const char *path);
	int read(MainWindow *mwindow, FileHTAL *htal);
	int read_audio(FileHTAL *htal);
	int read_video(FileHTAL *htal);
	int read_index(FileHTAL *htal);
	int reset_index();  // When the index file is wrong, reset the asset values
	int write(MainWindow *mwindow, FileHTAL *htal, int include_index, int use_relative_path);
	int write(MainWindow *mwindow, FILE *file, int include_index, int use_relative_path);
	int write_audio(FileHTAL *htal);
	int write_video(FileHTAL *htal);
	int write_index(FileHTAL *htal);
	int update_path(char *new_path);

// Path to file
	char path[1024];

// is silence
	int silence;

// is a transition	
	int transition;
	char plugin_title[1024];  // title of plugin
	int plugin_type;	// type of plugin
						// 1 - plugin
						// 2 - shared plugin
						// 3 - shared module
	SharedPluginLocation shared_plugin_location;   // Location of plugin if shared plugin
	SharedModuleLocation shared_module_location;   // Location of module if shared module

// Determines the file engine to use
	int format;      // format of file

// contains audio data
	int audio_data;

	int channels, rate, bits, byte_order, signed_, header;    // Audio specifications

// contains video data
	int video_data;       // Video specifications
	int layers;
	float frame_rate;
	int width, height;
	char compression[16];
	int quality;     // for jpeg compression

// Edits store data for the transition

// index info
	int index_status;     // 0 ready  1 not tested  2 being built  3 small source
	long index_zoom;      // zoom factor of index data
	long index_start;     // byte start of index data in the index file
	longest index_bytes;     // Total bytes in source file for comparison before rebuilding the index
	long index_end, old_index_end;    // values for index build
	long* index_offsets;  // offsets of channels in index file in floats
	float* index_buffer;  
};




#endif
