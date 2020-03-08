#include "arraylist.h"
#include "filehtal.h"
#include "filesystem.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "pluginserver.h"
#include "preferences.h"
#include <stdlib.h>
#include <string.h>

int init_plugins(Defaults *defaults, ArrayList<PluginServer*> *plugindb, char *local_plugin_dir, char *global_plugin_dir);
int delete_plugins(ArrayList<PluginServer*> *plugindb);

int delete_indexes(MainWindow *mwindow)
{
	FileSystem fs;
	int total_excess;
	long oldest;
	int oldest_item;
	int result;
	char string[1024];

// Delete extra indexes
	fs.set_filter(".idx");
	fs.complete_path(mwindow->preferences->index_directory);
	fs.update(mwindow->preferences->index_directory);

// Eliminate directories
	result = 1;
	while(result)
	{
		result = 0;
		for(int i = 0; i < fs.dir_list.total && !result; i++)
		{
			fs.join_names(string, mwindow->preferences->index_directory, fs.dir_list.values[i]->name);
			if(!fs.is_dir(string))
			{
				delete fs.dir_list.values[i];
				fs.dir_list.remove_number(i);
				result = 1;
			}
		}
	}
	total_excess = fs.dir_list.total - mwindow->preferences->index_count;

	while(total_excess > 0)
	{
// Get oldest
		for(int i = 0; i < fs.dir_list.total; i++)
		{
			fs.join_names(string, mwindow->preferences->index_directory, fs.dir_list.values[i]->name);

			if(i == 0 || fs.get_date(string) <= oldest)
			{
				oldest = fs.get_date(string);
				oldest_item = i;
			}
		}

		fs.join_names(string, mwindow->preferences->index_directory, fs.dir_list.values[oldest_item]->name);
		if(remove(string))
			perror("delete_indexes");
		delete fs.dir_list.values[oldest_item];
		fs.dir_list.remove_number(oldest_item);
		total_excess--;
	}
	return 0;
}






int main(int argc, char *argv[])
{
	fprintf(stderr, "Broadcast 2000c (C)2001 Adam Williams\n");
	if(sizeof(VWORD) == 2) printf("16 bits per channel enabled.\n");

// handle command line arguments first
	srand(time(0));
	ArrayList<char*> filenames;
	FileSystem fs;
	FileHTAL script('(', ')');
	char display[1024];
	display[0] = 0;
	
	int playback = 0;
	int record = 0;
	int want_gui = 1;
	int run_script = 0;
	int result = 0;

	for(int i = 1; i < argc; i++)
	{
		if(!strcmp(argv[i], "-h"))
		{                     // help
			printf("Usage:\n");
			printf("%s [-b script]\n", argv[0]);
			exit(0);
		}
		else
		if(!strcmp(argv[i], "-b"))
		{
			i++;
			if(i < argc)
			{
				script.read_from_file(argv[i]);
				run_script = 1;
			}
			else
			{
				printf("-b expects a script filename to run\n");
				exit(1);
			}
		}
		else
		if(!strcmp(argv[i], "-play"))
		{
			playback = 1;
			want_gui = 0;
		}
		else
		if(!strcmp(argv[i], "-record"))
		{
			record = 1;
			want_gui = 0;
		}
		else
		if(!strcmp(argv[i], "-display") && i + 1 < argc)
		{
			strcpy(display, argv[i + 1]);
			i++;
		}
		else
		{
			char *new_filename;
			new_filename = new char[1024];
			strcpy(new_filename, argv[i]);
            fs.complete_path(new_filename);

			filenames.append(new_filename);
		}
	}

// set the .bcast directory
	char directory[1024];
	sprintf(directory, "%s", BCASTDIR);
	fs.complete_path(directory);
	if(fs.is_dir(directory)) { fs.create_dir(directory); }
	
// load the defaults
	if(sizeof(VWORD) == 2)
		strcat(directory, "bcastrc16");
	else
		strcat(directory, "bcastrc");
	Defaults defaults(directory);
	defaults.load();

// load the plugins
	ArrayList<PluginServer*> plugindb;
	char global_plugin_dir[1024];
	char local_plugin_dir[1024];

//printf("main 1\n");
	init_plugins(&defaults, &plugindb, local_plugin_dir, global_plugin_dir);


//printf("main 1\n");
// create the main window
	MainWindow mainwindow;
	mainwindow.create_objects(&defaults, 
							&plugindb, 
							local_plugin_dir, 
							global_plugin_dir, 
							display, 
							want_gui, 
							!filenames.total);

//printf("main 1\n");
	if(filenames.total && want_gui)
	{
// load the initial files on seperate tracks
		mainwindow.load_filenames(&filenames);
	}

// 	if(playback)
// 	{
// // load and play files one at a time
// 		for(int i = 0; i < filenames.total; i++)
// 		{
// 			mainwindow.load(filenames.values[i], 0);
// 			mainwindow.set_playback_range();
// 			mainwindow.arm_playback(0, 0, 0, 0);
// 			mainwindow.start_playback();
// 			mainwindow.wait_for_playback();
// 			mainwindow.is_playing_back = 0;
// 		}
// 	}
// 	
// 	if(record)
// 	{
// // Start the record engine with default parameters.
// 	}

	if(want_gui)
	{
// run the program
		if(run_script) 
			result = mainwindow.run_script(&script);

		mainwindow.gui->run_window();
		mainwindow.save_defaults();		
		defaults.save();
	}

// on exit
	delete_indexes(&mainwindow);

// delete char strings
	for(int i = 0; i < filenames.total; i++) delete filenames.values[i];
	delete_plugins(&plugindb);
return 0;
}

// want plugins to initialize before mainwindow
int init_plugins(Defaults *defaults, ArrayList<PluginServer*> *plugindb, char *local_plugin_dir, char *global_plugin_dir)
{
	FileSystem fs;
	int result, pass; 
	char path[1024];

// get the plugin directories
	sprintf(global_plugin_dir, "/usr/local/bcast/plugins");
	defaults->get("GLOBAL_PLUGIN_DIR", global_plugin_dir);
	sprintf(local_plugin_dir, "");
	defaults->get("LOCAL_PLUGIN_DIR", local_plugin_dir);
	fs.complete_path(local_plugin_dir);
	fs.complete_path(global_plugin_dir);
	fs.set_filter(".plugin");

	PluginServer *newplugin;
// load plugins
	for(pass = 0; pass < 2; pass++)
	{
		result = 1;          // default abort

		switch(pass)
		{
// on first pass use global dir	
			case 0: 
				result = fs.update(global_plugin_dir); 
				if(!strlen(global_plugin_dir)) result = 1;
				break;
// on second pass use local dir
			case 1: 
				result = fs.update(local_plugin_dir);  
				if(!strlen(local_plugin_dir)) result = 1;
				break;
		}
//printf("init_plugins 1 %s %s\n", fs.get_current_dir(), fs.get_filter());

		if(!result) 
		{
			for(int i = 0; i < fs.dir_list.total; i++)
			{
// make sure filename isn't a directory
//printf("init_plugins 2 %s %d\n", fs.dir_list.values[i]->path, fs.is_dir(fs.dir_list.values[i]->path));
				if(fs.is_dir(fs.dir_list.values[i]->path))
				{
					strcpy(path, fs.dir_list.values[i]->path);
					fs.complete_path(path);
// query the plugin
					newplugin = new PluginServer(path);
//printf("init_plugins 3 %s\n", path);
					if(newplugin->query_plugin())
					{
						delete newplugin;
					}
					else
					{
						plugindb->append(newplugin);
//printf("init_plugins 4 %s audio %d video %d fileio %d\n", newplugin->title, newplugin->audio, newplugin->video, newplugin->fileio);
					}
				}
			}
		}
		else
		{
// notify user of failed directory search
			switch(pass)
			{
	// on first pass use global dir	
				case 0: if(strlen(global_plugin_dir)) printf("Couldn't open global plugin directory.\n");  break;
	// on second pass use local dir
				case 1: if(strlen(local_plugin_dir)) printf("Couldn't open local plugin directory.\n");    break;
			}
		}
	}
//printf("init_plugins 1\n");

	defaults->update("GLOBAL_PLUGIN_DIR", global_plugin_dir);
	defaults->update("LOCAL_PLUGIN_DIR", local_plugin_dir);
	return 0;
}

int delete_plugins(ArrayList<PluginServer*> *plugindb)
{
	int i;
	for(i = 0; i < plugindb->total; i++)
	{
		delete plugindb->values[i];
	}
return 0;
}
