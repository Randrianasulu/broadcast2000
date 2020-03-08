#include <string.h>
#include "assets.h"
#include "channel.h"
#include "defaults.h"
#include "errorbox.h"
#include "file.h"
#include "filehtal.h"
#include "formatcheck.h"
#include "formattools.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "neworappend.h"
#include "playbackengine.h"
#include "preferences.h"
#include "quicktime.h"
#include "record.h"
#include "recordengine.h"
#include "recordgui.h"
#include "recordlabel.h"
#include "testobject.h"
#include "timebar.h"
#include "videoconfig.h"

Record::Record(Defaults *defaults, MainWindow *mwindow, int x, int y)
 : BC_RecButton(x, y, 24, 24), Thread()
{
	this->defaults = defaults;
	this->mwindow = mwindow;
	script = 0;
	in_progress = 0;
	
	int _601_to_rgb_value;
	for(int i = 0; i <= 255; i++)
	{
		_601_to_rgb_value = (int)(1.1644 * i - 255 * 0.0627 + 0.5);
		if(_601_to_rgb_value < 0) _601_to_rgb_value = 0;
		if(_601_to_rgb_value > 255) _601_to_rgb_value = 255;
		_601_to_rgb_table[i] = _601_to_rgb_value;
	}
}

Record::~Record()
{
}

int Record::handle_event() { start(); return 0;
}

int Record::set_script(FileHTAL *script)
{
	this->script = script;
return 0;
}

int Record::run_script(Asset *asset, int &do_audio, int &do_video)
{
	int script_result = 0, result = 0;
	File test_file;

	while(!result && !script_result)
	{
		result = script->read_tag();

		if(!result)
		{
			if(script->tag.title_is("set_path"))
			{
				strcpy(asset->path, script->tag.get_property_text(0));
			}
			else
			if(script->tag.title_is("set_audio"))
			{
				do_audio = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_video"))
			{
				do_video = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_paste_output"))
			{
				to_tracks = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_format"))
			{
				if(!(asset->format = test_file.strtoformat(mwindow->plugindb, script->tag.get_property_text(0))))
				{
					printf("Invalid file format %s.  See the menu for possible file formats.\n", script->tag.get_property_text(0));
				}
			}
			else
			if(script->tag.title_is("set_audio_compression"))
			{
				if(!(asset->bits = test_file.strtobits(script->tag.get_property_text(0))))
				{
					printf("Invalid audio compressor %s.  See the menu for possible compressors.\n", script->tag.get_property_text(0));
				}
			}
			else
			if(script->tag.title_is("set_audio_dither"))
			{
				dither = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_audio_signed"))
			{
				asset->signed_ = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_audio_channels"))
			{
				asset->channels = script->tag.get_property_int(0);
				if(!asset->channels || asset->channels > MAXCHANNELS)
				{
					printf("Invalid number of channels %d.\n", asset->channels);
				}
			}
			else
			if(script->tag.title_is("set_channel"))
			{
				char string[1024];
				strcpy(string, script->tag.get_property_text(0));
				for(int i = 0; i < mwindow->channeldb.total; i++)
				{
					if(!strcasecmp(mwindow->channeldb.values[i]->title, string))
					{
						current_channel = i;
						break;
					}
				}
			}
			else
			if(script->tag.title_is("set_video_compression"))
			{
				strcpy(asset->compression, test_file.strtocompression(script->tag.get_property_text(0)));
			}
			else
			if(script->tag.title_is("set_video_quality"))
			{
				asset->quality = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("ok"))
			{
				script_result = 1;
			}
			else
			{
				printf("Record::run_script: Unrecognized command: %s\n", script->tag.get_title());
			}
		}
	}

	return script_result;
return 0;
}

void Record::run()
{
	if(in_progress) return;
	else
	in_progress = 1;

	long start, end;
	int format_error;
	char string[1024];
	start = mwindow->selectionstart;
	end = mwindow->selectionend;
	long total_length = end - start;
	this->realtime = mwindow->preferences->real_time_record;
	frame_w = mwindow->track_w;
	frame_h = mwindow->track_h;
	cpus = mwindow->preferences->smp + 1;
	int result, script_result = 0;
	Asset asset("");
	Asset asset2("");

	load_defaults(&asset);
	append_to_file = 0;

	format_error = 0;
	
	if(script)
	{
// Get parameters from script
		script_result = run_script(&asset, do_audio, do_video);
		result = 0;
	}

// Also skip updating the default asset compression since these are
// only available for their specific driver.
	if(get_video_driver() == CAPTURE_LML)
		strncpy(asset.compression, QUICKTIME_MJPA, 4);
	else
	if(get_video_driver() == CAPTURE_FIREWIRE)
		strncpy(asset.compression, QUICKTIME_DV, 4);

	do
	{
		if(!script_result)
		{
// Script did not contain "ok" so pop up a window.
			RecordWindow window(this, &asset);
			window.create_objects();
			result = window.run_window();
		}

		asset.audio_data = do_audio;
		asset.video_data = do_video;

		if(do_video)
		{
			asset.video_data = do_video;
			asset.layers = 1;
			asset.frame_rate = mwindow->frame_rate;
			asset.width = mwindow->track_w;
			asset.height = mwindow->track_h;
		}

		if(!result)
		{
			FormatCheck check_format(&asset);
			format_error = check_format.check_format();
		}
	}while(format_error && !result);

	save_defaults(&asset);

	if(!result)
	{
		File file, file2;

// set up file parameters
		FileSystem fs;
		fs.complete_path(asset.path);

// test existance of file
		{
			NewOrAppend window;
			result = window.test_file("", &asset, script);
			switch(result)
			{
				case 0:
					mwindow->purge_asset(asset.path);
					remove(asset.path);        // overwrite
					append_to_file = 0;
					break;

				case 1:
					in_progress = 0;
					return;                // cancel
					break;

				case 2:
					append_to_file = 1;      // append
					break;
			}
		}

// open output file in read/write mode
		file.set_processors(cpus > 1 ? 2 : 1);
		if(file.try_to_open_file(mwindow->plugindb, &asset, 1, 1))
		{
// open failed
			sprintf(string, "Couldn't open %s", asset.path);
			ErrorBox error;
			error.create_objects(string);
			error.run_window();
			in_progress = 0;
			return;
		}

// dithering is done at the device for recording and depends on the bits of the output file

// get selection to record
		RecordLabels labels;
		if(!mwindow->labels_follow_edits) labels.get_project_labels(mwindow->timebar->labels, start, end);
		startsource_sample = 0;
		startsource_frame = 0;
		if(do_audio) startsource_sample = file.get_audio_length();
		if(do_video) startsource_frame = file.get_video_length(mwindow->frame_rate);

// record it
		{
			char filename[1024];
			int real_time = mwindow->preferences->real_time_record;
			fs.extract_name(filename, asset.path);
			sprintf(string, "Recording %s", filename);

// create the engine
			RecordEngine engine(mwindow, this, &file, &asset, &labels);
			engine.initialize();

// create the GUI
			RecordGUI gui(this, &engine, string, 
				300 + (do_audio ? 25 + asset.channels * 25 : 0));
			gui.create_objects();

// start the engine
			engine.set_gui(&gui);
			engine.start_monitor();

			if(script)
			{
// Cause engine to start another action
				gui.lock_window();
				script_result = engine.run_script(script);
				gui.unlock_window();
			}

// Go straight to the gui
			result = gui.run_window();
		}

		save_engine_defaults();

		endsource_sample = 0;
		endsource_frame = 0;
		if(!result)
		{
			if(do_audio) endsource_sample = file.get_audio_length();
			if(do_video)
				endsource_frame = file.get_video_length(mwindow->frame_rate);
		}
		file.close_file();

		if(result || 
			(do_audio && endsource_sample <= 0) ||
			(do_video && endsource_frame <= 0))
		{     // delete file
			script = 0;
			in_progress = 0;
			remove(asset.path);
		}
		else
		if(to_tracks)
		{     // paste file into project
			mwindow->gui->lock_window();
			mwindow->undo->update_undo_edits("Record", 0);
			if(startsource_frame < 0) startsource_frame = 0;
// printf("startsource_sample %d endsource_sample %d startsource_frame %d endsource_frame %d\n", 
// 	startsource_sample, endsource_sample, startsource_frame, endsource_frame);
			result = mwindow->paste_output(start, 
					end, 
					startsource_sample, 
					endsource_sample, 
					startsource_frame, 
					endsource_frame, 
					&asset, 
					&labels);
			mwindow->undo->update_undo_edits();
			mwindow->changes_made = 1;
			mwindow->gui->unlock_window();
		}
	}
	script = 0;
	in_progress = 0;
}

int Record::get_time_format()
{
	return mwindow->preferences->time_format;
return 0;
}

float Record::get_frame_rate()
{
	return mwindow->frame_rate;
}

int Record::keypress_event()
{
	if(get_keypress() == 'r') { handle_event(); return 1; }
	return 0;
return 0;
}

int Record::load_defaults(Asset *asset)
{
	char string[1024];
	sprintf(asset->path, "");
	defaults->get("RECORDPATH", asset->path);

	to_tracks = defaults->get("RECORDTOTRACKS", 1);
	do_audio = defaults->get("RENDERAUDIO", 1);
	do_video = defaults->get("RENDERVIDEO", 1);

	sprintf(string, "WAV");
	defaults->get("RENDERFORMAT", string);
	File file;
	asset->format = file.strtoformat(mwindow->plugindb, string);

// Record compression can't be the same as render compression
// because DV can't be synthesized.
	sprintf(asset->compression, QUICKTIME_YUV2);
	defaults->get("RECORDCOMPRESSION", asset->compression);
	asset->quality = defaults->get("RENDERQUALITY", 80);
	asset->bits = defaults->get("RENDERBITS", 16);
	dither = defaults->get("RENDERDITHER", 0);
	asset->signed_ = defaults->get("RENDERSIGNED", 1);
	asset->byte_order = defaults->get("RENDERBYTEORDER", 1);
	duplex_status = defaults->get("RECORDDUPLEX", 0);
	asset->channels = defaults->get("RECORDSESSIONCHANNELS", 2);
	record_mode = defaults->get("RECORDMODE", 0);
	loop_duration = defaults->get("RECORDLOOPDURATION", 0);
	monitor_audio = defaults->get("MONITORAUDIO", 1);
	monitor_video = defaults->get("MONITORVIDEO", 1);
	video_window_open = defaults->get("VIDEOMONITOROPEN", 1);
//	video_window_scale = defaults->get("VIDEOMONITORSCALE", (float)1);
	video_window_w = defaults->get("RECORDVIDEOW", 320);
	video_x = defaults->get("VIDEOMONITORX", 0);
	video_y = defaults->get("VIDEOMONITORY", 0);
	video_zoom = defaults->get("VIDEOMONITORZ", (float)1);
	current_channel = defaults->get("RECORDCHANNEL", 0);
	video_brightness = defaults->get("VIDEO_BRIGHTNESS", 0);
	video_hue = defaults->get("VIDEO_HUE", 0);
	video_color = defaults->get("VIDEO_COLOR", 0);
	video_contrast = defaults->get("VIDEO_CONTRAST", 0);
	video_whiteness = defaults->get("VIDEO_WHITENESS", 0);
	reverse_interlace = defaults->get("REVERSE_INTERLACE", 0);
	asset->rate = mwindow->sample_rate;

	for(int i = 0; i < MAXCHANNELS; i++) 
	{
		sprintf(string, "RECORDDCOFFSET%d", i);
		dc_offset[i] = defaults->get(string, 0);
	}
return 0;
}

int Record::save_defaults(Asset *asset)
{
// Defaults which are saved after the format dialogue.
	defaults->update("RECORDPATH", asset->path);
	defaults->update("RECORDTOTRACKS", to_tracks);
	defaults->update("RENDERAUDIO", do_audio);
	defaults->update("RENDERVIDEO", do_video);

	File file;
	defaults->update("RENDERFORMAT", file.formattostr(mwindow->plugindb, asset->format));
	if(get_video_driver() != CAPTURE_LML &&
		get_video_driver() != CAPTURE_FIREWIRE)
			defaults->update("RECORDCOMPRESSION", asset->compression);
	defaults->update("RENDERQUALITY", asset->quality);
	defaults->update("RENDERBITS", asset->bits);
	defaults->update("RENDERDITHER", dither);
	defaults->update("RENDERSIGNED", asset->signed_);
	defaults->update("RENDERBYTEORDER", asset->byte_order);
	defaults->update("RECORDDUPLEX", duplex_status);
	defaults->update("RECORDSESSIONCHANNELS", asset->channels);
return 0;
}

int Record::save_engine_defaults()
{
	char string[256];
// Defaults which are saved after the record engine is closed.
	defaults->update("RECORDMODE", record_mode);
	defaults->update("RECORDLOOPDURATION", loop_duration);
	defaults->update("MONITORAUDIO", monitor_audio);
	defaults->update("MONITORVIDEO", monitor_video);
	defaults->update("VIDEOMONITOROPEN", video_window_open);
//	defaults->update("VIDEOMONITORSCALE", video_window_scale);
	defaults->update("RECORDVIDEOW", video_window_w);
	defaults->update("VIDEOMONITORX", video_x);
	defaults->update("VIDEOMONITORY", video_y);
	defaults->update("VIDEOMONITORZ", video_zoom);
	defaults->update("RECORDCHANNEL", current_channel);
	defaults->update("VIDEO_BRIGHTNESS", video_brightness);
	defaults->update("VIDEO_HUE", video_hue);
	defaults->update("VIDEO_COLOR", video_color);
	defaults->update("VIDEO_CONTRAST", video_contrast);
	defaults->update("VIDEO_WHITENESS", video_whiteness);
	defaults->update("REVERSE_INTERLACE", reverse_interlace);

	for(int i = 0; i < MAXCHANNELS; i++) 
	{
		sprintf(string, "RECORDDCOFFSET%d", i);
		defaults->update(string, dc_offset[i]);
	}
return 0;
}

int Record::set_loop_duration(long value)
{
	loop_duration = value; 
return 0;
}


Channel* Record::get_current_channel() 
{ 
	if(current_channel < mwindow->channeldb.total &&
		current_channel > -1)
		return mwindow->channeldb.values[current_channel];
	else
		return 0;
}
int Record::get_vu_format() { return mwindow->preferences->meter_format; return 0;
}
float Record::get_min_db() { return mwindow->preferences->min_meter_db; }

int Record::get_rec_mode() { return record_mode; return 0;
}
int Record::set_rec_mode(int value) { record_mode = value; return 0;
}
int Record::get_video_driver() { return mwindow->preferences->vconfig->video_in_driver; return 0;
}

int Record::get_samplerate() { return mwindow->sample_rate; return 0;
}
float Record::get_framerate() { return mwindow->frame_rate; }
int Record::get_video_buffersize() { return mwindow->preferences->video_write_length; return 0;
}
int Record::use_floatingpoint() { return mwindow->preferences->video_floatingpoint; return 0;
};
int Record::get_everyframe() { return mwindow->preferences->video_every_frame; return 0;
}

int Record::get_out_length() { return mwindow->preferences->playback_buffer; return 0;
}
int Record::get_software_positioning() { return mwindow->preferences->playback_software_timer; return 0;
}
long Record::get_out_buffersize() { return mwindow->preferences->playback_buffer; }
long Record::get_in_buffersize() { return mwindow->preferences->record_write_length; }
int Record::get_realtime() { return realtime; return 0;
}
int Record::get_meter_speed() { return mwindow->preferences->record_speed; return 0;
}

int Record::enable_duplex() { return mwindow->preferences->enable_duplex; return 0;
}
long Record::get_playback_buffer() { return mwindow->preferences->playback_buffer; }
int Record::get_duplex_range(long *start, long *end) { return mwindow->get_affected_range(start, end); return 0;
}
float Record::get_aspect_ratio() { return mwindow->aspect_w / mwindow->aspect_h; };

RecordWindow::RecordWindow(Record *record, Asset *asset)
 : BC_Window("", MEGREY, ICONNAME ": Record", 410, 335, 0, 0)
{
	this->record = record;
	this->asset = asset;
}

RecordWindow::~RecordWindow()
{
	delete format_tools;
 	delete ok_button;
	delete cancel_button;
	delete path_button;
	delete pathtext_button;
	//delete channels_button;
	delete to_tracks_button;
	delete format_button;
	delete bits_button;
	delete dither_button;
	delete signed_button;
	delete hilo_button;
	delete lohi_button;
}



int RecordWindow::create_objects()
{
	add_tool(new BC_Title(5, 5, "Select a file to record to:"));

	int x = 5, y = 25;
	format_tools = new FormatTools(this, 
					record->mwindow->plugindb, 
					asset, 
					&(record->do_audio),
					&(record->do_video),
					&(record->dither));
	format_tools->create_objects(x, 
					y, 
					1, 
					1, 
					1, 
					1, 
					1, 
					!(record->get_video_driver() == CAPTURE_LML ||
						record->get_video_driver() == CAPTURE_FIREWIRE),
					1);

	//add_tool(new BC_Title(5, 245, "Number of audio channels to record:"));
	//add_tool(channels_button = new RecordChannels(record, asset));

	add_tool(new BC_Title(5, 270, "Paste output into project:"));
	add_tool(to_tracks_button = new RecordToTracks(record, record->to_tracks));

	add_tool(ok_button = new RecordOK());
	add_tool(cancel_button = new RecordCancel());
return 0;
}






RecordOK::RecordOK() : BC_BigButton(5, 300, "Do it") {}
RecordOK::~RecordOK() {};
int RecordOK::handle_event() { set_done(0); return 0;
};
int RecordOK::keypress_event() 
{ 
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

RecordCancel::RecordCancel() : BC_BigButton(300, 300, "Cancel") {}
RecordCancel::~RecordCancel() {};
int RecordCancel::handle_event() { set_done(1); return 0;
};
int RecordCancel::keypress_event() 
{ 
	if(get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}




RecordPathText::RecordPathText(Record *record, Asset *asset)
 : BC_TextBox(5, 30, 300, asset->path) { this->record = record; this->asset = asset; }
RecordPathText::~RecordPathText() {}
int RecordPathText::handle_event() 
{
	strcpy(asset->path, get_text());
return 0;
}

RecordPath::RecordPath(Record *record, BC_TextBox *textbox, char *text)
 : BrowseButton(310, 30, textbox, text, "Record to file", "Select a file to record to", 0) 
{ this->record = record; }
RecordPath::~RecordPath() {}



RecordToTracks::RecordToTracks(Record *record, int default_)
 : BC_CheckBox(200, 270, 17, 17, default_) { this->record = record; }
RecordToTracks::~RecordToTracks() {}
int RecordToTracks::handle_event()
{
	record->to_tracks = down;
return 0;
}

