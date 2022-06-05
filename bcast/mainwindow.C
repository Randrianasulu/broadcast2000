#include <string.h>
#include "assetmanager.h"
#include "assets.h"
#include "buttonbar.h"
#include "cache.h"
#include "channel.h"
#include "console.h"
#include "errorbox.h"
#include "file.h"
#include "fileformat.h"
#include "filehtal.h"
#include "indexfile.h"
#include "levelwindow.h"
#include "levelwindowgui.h"
#include "mainmenu.h"
#include "mainsamplescroll.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "module.h"
#include "modules.h"
#include "new.h"
#include "patchbay.h"
#include "playbackengine.h"
#include "preferences.h"
#include "record.h"
#include "recordlabel.h"
#include "statusbar.h"
#include "threadloader.h"
#include "timebar.h"
#include "track.h"
#include "tracks.h"
#include "trackscroll.h"
#include "videodevice.inc"
#include "videowindow.h"

MainWindow::MainWindow()
{
}

MainWindow::~MainWindow()
{
	delete timebar;
	delete tracks;
	delete patches;
	delete console;
	delete level_window;
	delete video_window;
	delete assets;
	delete cache;             // delete the cache after the assets
							// assets deletes cache entries
	delete threadloader;
	if(gui) delete gui;
	delete undo;
	delete playback_engine;
	delete preferences;
}

int MainWindow::create_objects(Defaults *defaults, 
			ArrayList<PluginServer*> *plugindb, 
			const char *local_plugin_dir, 
			const char *global_plugin_dir, 
			const char *display, 
			int want_gui, 
			int want_new)
{
	char string[1024];
	FileSystem fs;

	changes_made = 0;
	view_start = 0;
	track_start = 0;
	selectionstart = selectionend = 0;
	filename[0] = 0;
	resize_lock = 0;
	playback_cursor_visible = 0;
	is_playing_back = 0;
	loop_playback = 0;
	loop_start = loop_end = 0;
	this->defaults = defaults;
	this->plugindb = plugindb;
	this->local_plugin_dir = local_plugin_dir;
	this->global_plugin_dir = global_plugin_dir;
	strcpy(this->display, display);

	preferences = new Preferences;
	preferences->load_defaults(defaults);
	load_defaults();

	gui = 0;
	if(want_gui) gui = new MainWindowGUI(display, defaults->get("MWINDOWWIDTH", 640), defaults->get("MWINDOWHEIGHT", 263));

	assets = new Assets;
	assets->create_objects(this);
	cache = new Cache(this);

	undo = new MainUndo(this);
	playback_engine = new PlaybackEngine(this);


// ================================== output channels before menu
	output_channels = defaults->get("OUTCHANNELS", 2);
	for(int i = 0, default_position; i < MAXCHANNELS; i++)
	{
		sprintf(string, "CHANNELPOSITION%d", i);
		default_position = i * 30;
		
		if(i == 0) default_position = 270;
		else
		if(i == 1) default_position = 90;
		else
		if(default_position == 90) default_position = 300;
		
		channel_positions[i] = defaults->get(string, default_position);
	}

	if(gui) gui->create_objects(this);

// subwindows are owned not inherited so call create_objects()
	int top = get_top();
	int bottom = get_bottom();

	patches = new PatchBay(this);
	patches->create_objects(top, bottom);

	tracks = new Tracks(this);
	if(gui)
		tracks->create_objects(defaults, gui->get_w(), gui->get_h(), top, bottom);
	else
		tracks->create_objects(defaults, 0, 0, 0, 0);


// ============================ console
	console = new Console(this);
	console->create_objects(defaults->get("CONSOLEW", 400), defaults->get("CONSOLEH", 181), defaults->get("HIDECONSOLE", 0), defaults->get("CONSOLEVERTICAL", 0));

// ======================= objects which depend on other objects
	if(gui)
	{
		timebar->draw();
		gui->statusbar->draw();
		gui->mainmenu->load_defaults(defaults);
	}

// =============================== thread out the level window

	level_window = new LevelWindow(this);
	level_window->load_defaults(defaults);
	level_window->create_objects();
	level_window->start();

// ============================== thread out the video window
	
	video_window = new VideoWindow(this);
	video_window->load_defaults(defaults);
	video_window->create_objects();
	video_window->start();

// ================================== set up plugins
	init_plugins();

	threadloader = new ThreadLoader(this);

// ============================== create the default project
	if(want_new)
	{
		int new_video_tracks = defaults->get("VTRACKS", 0);

		for(int i = 0; i < new_video_tracks; i++)
		{
			add_video_track();
		}

		int new_audio_tracks = defaults->get("ATRACKS", 2);

		for(int i = 0; i < new_audio_tracks; i++)
		{
			add_audio_track();
		}
	}
	changes_made = 0;

// ================================== thread out console
	console->start();
return 0;
}

int MainWindow::init_plugins() {  return 0;
}

int MainWindow::load_defaults()
{
// modify new.C if you modify these
	tracks_vertical = defaults->get("TRACKSVERTICAL", 0);
	zoom_sample = defaults->get("ZOOMSAMPLE", 1);
	zoom_y = defaults->get("ZOOMY", 64);
	zoom_track = defaults->get("ZOOMTRACK", 64);
	
	frame_rate = defaults->get("FRAMERATE", (float)10);
	sample_rate = defaults->get("SAMPLERATE", 44100);
	labels_follow_edits = defaults->get("LABELSFOLLOWEDITS", 0);
	autos_follow_edits = defaults->get("AUTOSFOLLOWEDITS", 1);
	cursor_on_frames = defaults->get("CURSORONFRAMES", 0);
	track_w = defaults->get("TRACKW", 720);
	track_h = defaults->get("TRACKH", 480);
	output_w = defaults->get("OUTPUTW", 720);
	output_h = defaults->get("OUTPUTH", 480);
	aspect_w = defaults->get("ASPECTW", (float)4);
	aspect_h = defaults->get("ASPECTH", (float)3);

	load_channels();
return 0;
}

int MainWindow::load_channels()
{
	FileSystem fs;
	char directory[1024];
	FileHTAL file;
	Channel *channel;
	int done;

	sprintf(directory, BCASTDIR);
	fs.complete_path(directory);
	strcat(directory, "channels");
	done = file.read_from_file(directory, 1);

// Load channels
	while(!done)
	{
		channel = new Channel;
		if(!(done = channel->load(&file)))
			channeldb.append(channel);
		else
		{
			delete channel;
		}
	}
return 0;
}

int MainWindow::save_channels()
{
	FileSystem fs;
	char directory[1024];
	FileHTAL file;

	sprintf(directory, BCASTDIR);
	fs.complete_path(directory);
	strcat(directory, "channels");

	if(channeldb.total)
	{
		for(int i = 0; i < channeldb.total; i++)
		{
// Save channel here
			channeldb.values[i]->save(&file);
		}
		file.terminate_string();
		file.write_to_file(directory);
	}
	return 0;
return 0;
}

int MainWindow::save_defaults()
{
	char string[1024];

	preferences->save_defaults(defaults);
	for(int i = 0; i < MAXCHANNELS; i++)
	{
		sprintf(string, "CHANNELPOSITION%d", i);
		defaults->update(string, channel_positions[i]);
	}

	tracks->save_defaults(defaults);

	defaults->update("TRACKSVERTICAL", tracks_vertical);

	defaults->update("LABELSFOLLOWEDITS", labels_follow_edits);
	defaults->update("AUTOSFOLLOWEDITS", autos_follow_edits);
	defaults->update("CURSORONFRAMES", cursor_on_frames);
	defaults->update("ZOOMSAMPLE", zoom_sample);
	defaults->update("ZOOMY", zoom_y);
	defaults->update("ZOOMTRACK", zoom_track);

	if(console->gui) defaults->update("HIDECONSOLE", console->gui->get_hidden());
	console->update_defaults(defaults);

	level_window->update_defaults(defaults);
	video_window->update_defaults(defaults);

	if(gui) gui->save_defaults(defaults);

	save_channels();
	return 0;
return 0;
}

int MainWindow::run_script(FileHTAL *script)
{
	int result = 0, result2 = 0;
	while(!result && !result2)
	{
		result = script->read_tag();
		if(!result)
		{
			if(script->tag.title_is("new_project"))
			{
// Run new in immediate mode.
				gui->mainmenu->new_project->set_script(script);
				gui->mainmenu->new_project->run();
			}
			else
			if(script->tag.title_is("record"))
			{
// Run record as a thread.  It is a terminal command.
				gui->buttonbar->record_button->set_script(script);
				gui->buttonbar->record_button->start();
// Will read the complete scipt file without letting record read it if not
// terminated.
				result2 = 1;
			}
			else
			{
				printf("MainWindow::run_script: Unrecognized command: %s\n",script->tag.get_title() );
			}
		}
	}
	return result2;
return 0;
}

// ================================= synchronization

int MainWindow::lock_resize() { resize_lock = 1; return 0;
}

int MainWindow::unlock_resize() { resize_lock = 0; return 0;
}


// =============================== file operations


int MainWindow::load_filenames(ArrayList<char*> *filenames)
{
	threadloader->set_paths(filenames);
// run as a thread to allow prompting for missing files
	threadloader->start();
return 0;
}

int MainWindow::load(const char *filename, int import_)
{
// get the format
	File in;
	Asset asset(filename);
	int result = in.try_to_open_file(plugindb, &asset, 1, 0);
	long length_samples = 0, length_frames = 0;
	int fix_sizes = 1;    // Change the frame sizes for certain files
	int bcast_file = 0;

	switch(result)
	{
		case 0:
			length_samples = in.get_audio_length();
			length_frames = in.get_video_length(import_ ? frame_rate : asset.frame_rate);
			in.close_file();
			break;

		case 1:                         
// ================== file not found
			if(gui)
			{
				ErrorBox errorbox(display);
				char string[1024];
				sprintf(string, "Couldn't open %s", filename);
				errorbox.create_objects(string);
				errorbox.run_window();
				unlock_resize();
				gui->enable_window();
			}
			else
			{
				printf("Unable to open %s\n", filename);
			}
			return 1;
			break;
			
		case 2:
		{
// ===================== try to get format from index file or asset
			int result2;
			IndexFile indexfile(this);
			Asset *old_asset;
			if(!(result2 = indexfile.open_index(this, &asset)))
			{
// successfully read info
				indexfile.close_index();
			}
			else
			if(old_asset = assets->get_asset(asset.path))
			{
// Asset already exists
				asset = *old_asset;
			}
			else
			{
				FileFormat fwindow(display);
// ================================== assume format for an audio file
				asset.audio_data = 1;
				asset.format = PCM;
				asset.channels = defaults->get("CHANNELS", 2);
				asset.rate = defaults->get("RATE", 44100);
				asset.bits = defaults->get("BITS", 16);
				asset.byte_order = defaults->get("BYTE_ORDER", 1);
				asset.signed_ = defaults->get("SIGNED_", 1);
				asset.header = defaults->get("HEADER", 0);
				
				char string[1024], filename_name[1024];
				FileSystem fs;
				fs.extract_name(filename_name, asset.path);

				sprintf(string, "%s's format couldn't be determined.", filename_name);

// ======================= get file format from user

				fwindow.create_objects(&asset, string);
				result = fwindow.run_window();
				
// ========================== save defaults
				defaults->update("CHANNELS", asset.channels);
				defaults->update("RATE", asset.rate);
				defaults->update("BITS", asset.bits);
				defaults->update("BYTE_ORDER", asset.byte_order);
				defaults->update("SIGNED_", asset.signed_);
				defaults->update("HEADER", asset.header);

				if(result) 
				{
					if(gui)
					{
						unlock_resize();
						gui->enable_window();
					}
					return 1;     // ============== operation cancelled
				}
			}

// ============================== open it again

			result = in.try_to_open_file(plugindb, &asset, 1, 0);
			length_samples = in.get_audio_length();
			length_frames = in.get_video_length(import_ ? frame_rate : asset.frame_rate);
			in.close_file();
		}
			break;
		
		case 3:
			bcast_file = 1;
			break;
	}

// in case of single frame video files paste a second of data
	if(length_frames < 0)
	{
		length_frames = (long)frame_rate;
		fix_sizes = 0;
	}

// prevent thread indexer from accessing database during load
	if(gui)
	{
		gui->lock_window();
		console->gui->lock_window();
// save the before undo
		undo->update_undo_all("Load", 0);
	}

// ready to load
	if(!import_)
	{                 // delete existing project if not imported
		delete_project();
		loop_playback = 0;
		if(bcast_file) 
		{
			set_filename(filename);   // only bcast files can be saved
		}
		else 
		{
			set_filename("");     // otherwise clear the previous filename

// ===================== fix channels for audio
			if(asset.audio_data)
				if(asset.channels > 0 && asset.channels < 10) change_channels(output_channels, asset.channels);

// ===================== fix the frame size for certain files
			if(asset.video_data && fix_sizes)
			{
				track_w = asset.width;
				track_h = asset.height;
				output_w = asset.width;
				output_h = asset.height;
				//create_aspect_ratio(aspect_w, aspect_h, asset.width, asset.height);
				if(gui) video_window->resize_window();
			}
		}
	}

// ===================================== import the data

	if(bcast_file)
	{
// bcast file
	 	FileHTAL htal;
	 	htal.read_from_file(filename);

	 	result = load(&htal, import_);

//assets->dump();
	}
	else
	{
// store the asset
		Asset *new_asset = assets->update(&asset);

// ========================== set the settings when not importing
// Since the tracks draw when they import, set the frame and sample rates here
// to get synchronization.
		if(!import_)
		{
			if(asset.audio_data)
			{
				sample_rate = asset.rate;
			}
			
			if(asset.video_data)
			{
// some video formats should change the project frame rate
				if(asset.frame_rate && length_frames > 0)
					frame_rate = asset.frame_rate;
			}
		}

// ========================= create new tracks
		for(int i = 0; i < asset.layers; i++)
		{
			tracks->import_video_track(length_frames, i, new_asset);
		}

		for(int i = 0; i < asset.channels; i++)
		{
			tracks->import_audio_track(length_samples, i, new_asset);
		}
	}

// ================================ fix the interface
	if(!import_)
	{               // no changes if not imported
		changes_made = 0;
	}
	else
	{
		changes_made = 1;
	}

	if(gui)
	{
// save the after undo
		undo->update_undo_all();
// ===================== fix time bar
		timebar->draw();
// ===================== fix the scroll bars
		gui->mainsamplescroll->set_position();
		gui->trackscroll->update();

		if(!import_) gui->mainmenu->add_load(filename);

		console->gui->unlock_window();
		gui->unlock_window();
// ================================ create index files in background
		assets->build_indexes();
	}

	return 0;
return 0;
}

int MainWindow::load(FileHTAL *htal, int import_,
		int edits_only,
		int patches_only,
		int console_only,
		int timebar_only,
		int automation_only)
{
	int result;
	int load_all = 0;
	Track *current_track;
	Patch* current_patch;
	Module* current_module;

	if(edits_only + patches_only + console_only + timebar_only + automation_only == 0) load_all = 1;
	if((edits_only || automation_only || load_all) && gui) tracks->hide_overlays(0);

// get the current track to load
	current_track = tracks->first;
	current_patch = patches->first;
	current_module = console->modules->first;
	int track_offset = tracks->total();   // offset for importing shared plugins

// scan file for HTAL block	
	do{
	  result = htal->read_tag();
	}while(!result && strcmp(htal->tag.get_title(), "HTAL"));


// HTAL block found
	if(!result)
	{
		do{
// scan until end of block or end of file
			result = htal->read_tag();
			if(!result)
			{
				if(htal->tag.title_is("/HTAL"))
				{
					result = 1;
				}
				else
				if(htal->tag.title_is("ASSETS"))
				{
					if(load_all || edits_only)
					{
						assets->load(htal);
					}
				}
				else
				if(htal->tag.title_is("LABELS"))
				{
					if(timebar_only || edits_only) timebar->delete_project();
					if(timebar_only || load_all || edits_only) timebar->load(htal);
				}
				else
				if(htal->tag.title_is("VIDEO"))
				{
					load_video_config(htal, import_);
				}
				else
				if(htal->tag.title_is("AUDIO"))
				{
					load_audio_config(htal, import_, edits_only, patches_only, console_only, automation_only);
				}
				else
				if(htal->tag.title_is("TRACK"))
				{
//assets->dump();
					if(load_all)
					{
						tracks->load(htal, track_offset);
					}
					else
					if(edits_only)
					{
						if(current_track)
						{
							current_track->load_edits(htal);
							current_track = current_track->next;
						}
					}
					else
					if(automation_only)
					{
						if(current_track)
						{
							current_track->load_automation(htal);
							current_track = current_track->next;
						}
					}
					else
					if(patches_only)
					{
						if(current_patch)
						{
							current_patch->load(htal);
							current_patch = current_patch->next;
						}
					}
					else
					if(console_only)
					{
						if(current_module)
						{
							current_module->load(htal, track_offset);
							current_module = current_module->next;
						}
					}
				}     // TRACK
			}
		}while(!result);
	}


	if((edits_only || automation_only || load_all) && gui) 
	{
		tracks->show_overlays(1);
		gui->mainsamplescroll->set_position();
		timebar->draw();
	}
	
	return 0;
return 0;
}

int MainWindow::load_video_config(FileHTAL *htal, int import_)
{
	if(!import_) 
	{
		frame_rate = htal->tag.get_property("FRAMERATE", frame_rate);
		preferences->frames_per_foot = htal->tag.get_property("FRAMES_PER_FOOT", preferences->frames_per_foot);
		track_w = htal->tag.get_property("TRACKW", track_w);
		track_h = htal->tag.get_property("TRACKH", track_h);
		output_w = htal->tag.get_property("OUTPUTW", output_w);
		output_h = htal->tag.get_property("OUTPUTH", output_h);
		aspect_w = htal->tag.get_property("ASPECTW", aspect_w);
		aspect_h = htal->tag.get_property("ASPECTH", aspect_h);
		video_window->resize_window();
	}
return 0;
}

int MainWindow::load_audio_config(FileHTAL *htal, int import_,
		int edits_only, 
		int patches_only,
		int console_only,
		int automation_only)
{
	if(!import_)
	{
		if(edits_only + patches_only + console_only + automation_only == 0)
		{
// load channels setting
			long new_channels;

			new_channels = htal->tag.get_property("OUTCHANNELS", (long)output_channels);
			if(new_channels != output_channels) change_channels(output_channels, new_channels);	
			
			char string1[32];
			
			for(int i = 0; i < output_channels; i++)
			{
				sprintf(string1, "CHPOSITION%d", i);
				channel_positions[i] = htal->tag.get_property(string1, channel_positions[i]);
			}
		}
	
		sample_rate = htal->tag.get_property("SAMPLERATE", (long)sample_rate);
	}
return 0;
}

int MainWindow::load_edits(FileHTAL *htal)
{
	load(htal, 0, 1);
return 0;
}

int MainWindow::load_patches(FileHTAL *htal)
{
	load(htal, 0, 0, 1);
return 0;
}

int MainWindow::load_console(FileHTAL *htal)	
{
	load(htal, 0, 0, 0, 1);
return 0;
}

int MainWindow::load_timebar(FileHTAL *htal)	
{
	load(htal, 0, 0, 0, 0, 1);
return 0;
}

int MainWindow::load_automation(FileHTAL *htal)
{
	load(htal, 0, 0, 0, 0, 0, 1);
return 0;
}

int MainWindow::interrupt_indexes()
{
	assets->interrupt_indexes();
	gui->mainmenu->asset_manager->interrupt_indexes();
return 0;
}





int MainWindow::save(FileHTAL *htal, int use_relative_path)
{
// begin file
	htal->tag.set_title("HTAL");
	htal->append_tag();
	htal->append_newline();

	assets->save(htal, use_relative_path);
	timebar->save(htal);

	save_video_config(htal);
	save_audio_config(htal);

	Track *current_track = tracks->first;
	while(current_track)
	{
		current_track->save(htal);
		current_track = current_track->next;
	}

// terminate file
	htal->tag.set_title("/HTAL");
	htal->append_tag();
	htal->append_newline();
	htal->terminate_string();
return 0;
}

int MainWindow::save_video_config(FileHTAL *htal)
{
	htal->tag.set_title("VIDEO");
	htal->tag.set_property("FRAMERATE", frame_rate);
	htal->tag.set_property("FRAMES_PER_FOOT", preferences->frames_per_foot);
	htal->tag.set_property("TRACKW", track_w);
	htal->tag.set_property("TRACKH", track_h);
	htal->tag.set_property("OUTPUTW", output_w);
	htal->tag.set_property("OUTPUTH", output_h);
	htal->tag.set_property("ASPECTW", aspect_w);
	htal->tag.set_property("ASPECTH", aspect_h);
	htal->append_tag();
	htal->append_newline();
	htal->append_newline();
return 0;
}

int MainWindow::save_audio_config(FileHTAL *htal)
{
	char string1[32], string2[32];
	htal->tag.set_title("AUDIO");
	htal->tag.set_property("SAMPLERATE", (long)sample_rate);
	htal->tag.set_property("OUTCHANNELS", (long)output_channels);
	
	for(int i = 0; i < output_channels; i++)
	{
		sprintf(string1, "CHPOSITION%d", i);
		sprintf(string2, "%d", channel_positions[i]);
		htal->tag.set_property(string1, string2);
	}
	
	htal->append_tag();
	htal->append_newline();
	htal->append_newline();
return 0;
}


int MainWindow::set_filename(const char *filename)
{
	strcpy(this->filename, filename);
	if(gui)
	{
		if(filename[0] == 0)
		{
			gui->set_title("Broadcast 2000");
		}
		else
		{
			FileSystem dir;
			char string[1024], string2[1024];
			dir.extract_name(string, filename);
			sprintf(string2, ICONNAME ": %s\n", string);
			gui->set_title(string2);
		}
	}
return 0;
}

// ========================================== drawing

int MainWindow::draw()
{
	if(gui)
	{
		gui->statusbar->draw();
		tracks->draw();
		gui->mainsamplescroll->set_position();
		timebar->draw();

		tracks->show_overlays(1);
	}
return 0;
}

int MainWindow::redraw_time_dependancies() 
{ 
	if(gui)
	{
		timebar->draw(); gui->statusbar->draw();
	}
return 0;
}

int MainWindow::flip_vertical(int new_orientation)
{
	if(gui)
	{
		tracks_vertical = new_orientation;

		gui->flip_vertical();
		timebar->flip_vertical();
		gui->mainsamplescroll->flip_vertical();
		
		int top = get_top();
		int bottom = get_bottom();
		patches->flip_vertical(top, bottom);
		gui->trackscroll->flip_vertical(top, bottom);
		tracks->flip_vertical(top, bottom);
		draw();
	}
return 0;
}

// ======================================= cursors



int MainWindow::draw_floating_handle(int flash) 
{ 
	tracks->draw_floating_handle(flash); 
return 0;
}







// ============================================ selecting


long MainWindow::align_to_frames(long sample)
{
	long frame = (long)(((float)(sample + 1) / sample_rate) * frame_rate);
    long result = (long)(((float)frame / frame_rate) * sample_rate);
    return result;
}

int MainWindow::set_selectionend(long new_position)
{
	if(gui)
	{
		tracks->draw_cursor(0);   // hide old cursor
		selectionend = new_position;
		if(new_position < selectionstart) selectionstart = new_position;
		tracks->draw_cursor(1);   // show new cursor
		gui->statusbar->draw();
	}
return 0;
}

int MainWindow::set_selectionstart(long new_position)
{
	if(gui)
	{
		tracks->draw_cursor(0);   // hide old cursor
		selectionstart = new_position;
		if(new_position > selectionend) selectionend = new_position;
		tracks->draw_cursor(1);   // show new cursor
		gui->statusbar->draw();
	}
return 0;
}

int MainWindow::set_selection(long selectionstart, long selectionend)
{
	if(gui)
	{
		tracks->draw_cursor(0);   // hide old cursor
		this->selectionstart = selectionstart;
		this->selectionend = selectionend;
		tracks->draw_cursor(1);   // show new cursor
		gui->statusbar->draw();
	}
return 0;
}

int MainWindow::get_affected_range(long *start, long *end, int reverse)
{
	if(selectionstart == selectionend)
	{
		if(!reverse)
		{
			*start = selectionstart;
			*end = tracks->total_playable_samples();
		}
		else
		{
			*start = 0;
			*end = selectionstart;
		}
		if(*end < *start) *end = *start;
	}
	else
	{
		*start = selectionstart; *end = selectionend;
	}
return 0;
}


int MainWindow::set_loop_boundaries()
{
	long start, end;
	
	if(selectionstart != selectionend) 
	{
		start = selectionstart;
		end = selectionend;
	}
	else
	if(tracks->total_samples())
	{
		start = 0;
		end = tracks->total_samples();
	}
	else
	{
		start = end = 0;
	}

	if(!loop_playback && start != end)
	{
		loop_start = start;
		loop_end = end;
		loop_playback = 1;
		if(gui) tracks->draw_loop_points(1);
	}
	else
	if(loop_playback)
	{
		if(gui) tracks->draw_loop_points(1);
		loop_playback = 0;
	}
return 0;
}


// ======================================= playback

int MainWindow::update_playback_cursor(long new_position, int view_follows_playback)
{
	if(!gui) return 1;
	
	long cursor_x1, cursor_x2;
	long view_end;
	
	gui->lock_window();
	
	
	view_end = view_start + tracks->view_samples();
	cursor_x1 = (last_playback_position - view_start) / zoom_sample;
	cursor_x2 = (new_position - view_start) / zoom_sample;
	if(cursor_x1 != cursor_x2)
	{
// recalculate in case of window movement
		view_end = view_start + tracks->view_samples();
		cursor_x1 = (last_playback_position - view_start) / zoom_sample;
		cursor_x2 = (new_position - view_start) / zoom_sample;

		if(new_position >= view_start && new_position <= view_end)
			tracks->draw_playback_cursor(cursor_x2);

		if(playback_cursor_visible && last_playback_position >= view_start && last_playback_position <= view_end) 
			tracks->draw_playback_cursor(cursor_x1, 1);

		playback_cursor_visible = 1;

		last_playback_position = new_position;
		
		if(view_follows_playback && !gui->mainsamplescroll->in_use())
		{
			if(playback_engine->reverse)
			{
				if(cursor_x2 < 0 &&
					cursor_x2 > -tracks->view_pixels() / 4 &&
					((!loop_playback && playback_engine->playback_start < view_start) ||
					(loop_playback && loop_start > view_start)))
				{
					move_left(zoom_sample * tracks->view_pixels());
				}
			}
			else
			{
				if(cursor_x2 > tracks->view_pixels() &&
					cursor_x2 < tracks->view_pixels() + tracks->view_pixels() / 4 &&
					((!loop_playback && playback_engine->playback_end > view_end) ||
					(loop_playback && loop_end > view_end)))
				{
					move_right(zoom_sample * tracks->view_pixels());
				}
			}
		}
	}
	
	gui->statusbar->update_playback(new_position);
	gui->unlock_window();
return 0;
}

int MainWindow::show_playback_cursor(long position, int flash)
{
	if(playback_engine->is_playing_back)
	{
		playback_cursor_visible = 1;
		if(position == -1) position = last_playback_position;

		long cursor_x1 = (position - view_start) / zoom_sample;
		if(position >= view_start && position <= view_start + tracks->view_samples())
		{
			tracks->draw_playback_cursor(cursor_x1, flash);
		}
		last_playback_position = position;
	}
return 0;
}

int MainWindow::hide_playback_cursor(int flash)
{
	if(playback_cursor_visible)
	{
		playback_cursor_visible = 0;
		long cursor_x1 = (last_playback_position - view_start) / zoom_sample;
		if(last_playback_position >= view_start && last_playback_position <= view_start + tracks->view_samples())
		{
			tracks->draw_playback_cursor(cursor_x1, flash);
		}
	}
return 0;
}

int MainWindow::set_playback_range(long start_position, int reverse, float speed)
{
	long start, end;

	playback_engine->reset_parameters();
	get_affected_range(&start, &end, reverse);

//printf("MainWindow::set_playback_range %ld %ld %f\n", start, end, speed);
// use default
	if(start_position == -1)  
	{
		if(reverse) 
			playback_engine->set_range(start, end, end, reverse, speed);      // start at the end
		else 
			playback_engine->set_range(start, end, start, reverse, speed);     // start at the start
	}
	else
// use current position
	{
		if(reverse)
		{
			playback_engine->set_range(start, start_position, start_position, reverse, speed);
		}
		else
		{
			playback_engine->set_range(start_position, end, start_position, reverse, speed);
		}
	}
return 0;
}

int MainWindow::arm_playback(int follow_loop, 
					int use_buttons, 
					int infinite, 
					AudioDevice *audio)
{
	if(!is_playing_back)
	{
		is_playing_back = 1;
//printf("MainWindow::arm_playback 1\n");
		playback_engine->arm_playback(follow_loop, 
					use_buttons, 
					infinite, 
					audio);
//printf("MainWindow::arm_playback 2\n");
	}
return 0;
}



int MainWindow::start_playback() 
{ 
	playback_engine->start_playback(); 
return 0;
}

int MainWindow::stop_playback(int update_button) 
{
	is_playing_back = 0;
// need cursor to finish so unlock
	if(gui) gui->unlock_window();
	playback_engine->stop_playback(update_button); 
	if(gui) gui->lock_window();
return 0;
}

int MainWindow::wait_for_playback()
{
	if(is_playing_back) playback_engine->wait_for_completion();
return 0;
}




long MainWindow::get_playback_position() 
{
// Want the position rendered
	return playback_engine->get_position(0);
}

int MainWindow::start_reconfigure(int unlock_window)
{
	if(is_playing_back)
	{
// call stop_playback here because calling from console screws up window locking
		if(unlock_window && gui) gui->unlock_window();
		playback_engine->start_reconfigure(); 
	}
return 0;
}

int MainWindow::stop_reconfigure(int unlock_window)
{
	if(is_playing_back)
	{
		playback_engine->stop_reconfigure();		
		if(unlock_window && gui) gui->lock_window();
	}
return 0;
}


int MainWindow::reset_meters()
{
	if(gui)
	{
		level_window->gui->lock_window();
		level_window->gui->reset_over();
		level_window->gui->unlock_window();

		console->gui->lock_window();
		console->modules->reset_meters();
		console->gui->unlock_window();
	}
return 0;
}


// ======================================= window sizes


int MainWindow::get_top() 
{ 
	if(gui)
	{
		if(tracks_vertical)
			return gui->get_w() - 18 - 17;     // timebar + scrollbar
		else
			return timebar->gui->get_y() + timebar->gui->get_h();
	}
	else
	return 0;
return 0;
}

int MainWindow::get_bottom() 
{  
	if(gui)
	{
		if(tracks_vertical)
			return BUTTONBARWIDTH;         // buttonbar
		else
			return gui->get_h() - 24 - 17;      // statusbar + scrollbar
	}
	else
	return 0;
return 0;
}
