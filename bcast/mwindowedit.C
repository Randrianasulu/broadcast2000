#include <string.h>
#include "assets.h"
#include "cache.h"
#include "console.h"
#include "filehtal.h"
#include "levelwindow.h"
#include "mainmenu.h"
#include "mainsamplescroll.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "patchbay.h"
#include "playbackengine.h"
#include "statusbar.h"
#include "timebar.h"
#include "track.h"
#include "tracks.h"
#include "trackscroll.h"


int MainWindow::init_selection(long cursor_position, 
				int cursor_x, 
				int cursor_y, 
				int &current_end, 
				long &selection_midpoint1,
				long &selection_midpoint2,
				int &selection_type)
{
	int result = 0;
	long new_start, new_end;
	stop_playback(1);
	tracks->draw_cursor(0);   // hide old cursor

// Search for selected edit
	if(gui->get_double_click())
	{
		if(tracks->select_edit(cursor_position, cursor_x, cursor_y, new_start, new_end))
		{
			result = 1;
			if(gui->shift_down())
			{
				selection_midpoint1 = selectionstart;
				selection_midpoint2 = selectionend;
				if(new_end > selectionend)
				{
					selection_midpoint2 = selectionend;
					selectionend = new_end;
				}
				if(new_start < selectionstart)
				{
					selection_midpoint1 = selectionstart;
					selectionstart = new_start;
				}
				
			}
			else
			{
				selectionstart = selection_midpoint1 = new_start;
				selectionend = selection_midpoint2 = new_end;
			}
			selection_type = SELECTION_EDIT;
		}
	}

	if(!result)
	{
// determine the end to adjust
		if(gui->shift_down()) 
		{
			if(cursor_position > selectionend / 2 + selectionstart / 2)
			{                    // right end selected
				current_end = 2;
				selectionend = cursor_position;
				selection_midpoint1 = selection_midpoint2 = selectionstart;
			}
			else
			{                    // left end selected
				current_end = 1;
				selectionstart = cursor_position;
				selection_midpoint1 = selection_midpoint2 = selectionend;
			}
		}
		else
		{
			selectionend = selectionstart = selection_midpoint1 = selection_midpoint2 = cursor_position;
			current_end = 2;     // default to right end
		}
		selection_type = SELECTION_SAMPLES;
	}

	tracks->draw_cursor(1);   // show new cursor
	gui->statusbar->draw();
	timebar->draw();
	return 0;
return 0;
}

int MainWindow::update_selection(long cursor_position,
				int cursor_x, 
				int cursor_y, 
				int &current_end, 
				long selection_midpoint1,
				long selection_midpoint2,
				int selection_type)
{
	long new_start, new_end;
	if(cursor_position < 0) cursor_position = 0;

// was cursor reposition
	tracks->draw_cursor(0);   // hide old cursor

	if(selection_type == SELECTION_EDIT)
	{
		if(tracks->select_edit(cursor_position, cursor_x, cursor_y, new_start, new_end))
		{
			if(new_start < selection_midpoint1)
			{
				selectionstart = new_start;
				if(new_end <= selection_midpoint2)
				{
					selectionend = new_end;
				}
			}

			if(new_end > selection_midpoint2)
			{
				selectionend = new_end;
				if(new_start >= selection_midpoint1)
				{
					selectionstart = new_start;
				}
			}
		}
	}
	else
	{
		if(current_end == 1)
		{
			if(cursor_position <= selection_midpoint1)
			{
				selectionstart = cursor_position;
			}
			else
			{
				selectionend = cursor_position;
				selectionstart = selection_midpoint1;
				current_end = 2;
			}
		}
		else
		{
			if(cursor_position > selection_midpoint2)
			{
				selectionend = cursor_position;
			}
			else
			{
				selectionstart = cursor_position;
				selectionend = selection_midpoint2;
				current_end = 1;
			}
		}
	}

	timebar->draw();
	gui->statusbar->draw();
	tracks->draw_cursor(1);    // draw new cursor and flash it
return 0;
}


int MainWindow::init_handle_selection(long cursor_position, int handle_pixel, int which_handle)
{
	stop_playback(1);
	if(gui->shift_down())
	{
// adjust the selected region not a handle
		if(cursor_position > selectionend / 2 + selectionstart / 2)
		{                    // right end selected
			set_selectionend(cursor_position);
		}
		else
		{                    // left end selected
			set_selectionstart(cursor_position);
		}
		return 3;  // Region selections don't enable editing
//		return which_handle;
	}
	else
	{
		set_selection(cursor_position, cursor_position);
		return which_handle;
	}
return 0;
}

int MainWindow::add_audio_track()
{
	if(gui) tracks->hide_overlays(0);
	changes_made = 1;
	tracks->add_audio_track(0);
	if(gui)
	{
		gui->trackscroll->update();
		tracks->show_overlays(1);
	}
return 0;
}

int MainWindow::add_video_track()
{
	if(gui) tracks->hide_overlays(0);
	changes_made = 1;
	tracks->add_video_track(0);
	if(gui)
	{
		gui->trackscroll->update();
		tracks->show_overlays(1);
	}
return 0;
}

int MainWindow::delete_project(int flash)
{
	if(gui) tracks->hide_overlays(0);
	tracks->delete_all(flash);
	patches->delete_all();
	timebar->delete_project();
	console->delete_project();
	assets->delete_all();
	if(gui)
	{
		gui->trackscroll->update();
		gui->mainsamplescroll->set_position();
		tracks->show_overlays(0); // don't flash since this is an intermediate step
	}
return 0;
}

int MainWindow::cut(long start, long end)
{
	tracks->copy(start, end);
	clear(start, end);
return 0;
}

int MainWindow::paste_output(long startproject, 
				long endproject, 
				long startsource_sample, 
				long endsource_sample, 
				long startsource_frame,
				long endsource_frame,
				Asset *asset, 
				RecordLabels *new_labels)
{
// Sanity check
	if(startsource_frame < 0) startsource_frame = 0;

	purge_asset(asset->path);
	Asset *new_asset = assets->update(asset);
	tracks->paste_output(startproject, 
						endproject, 
						startsource_sample, 
						endsource_sample, 
						startsource_frame, 
						endsource_frame, 
						new_asset);

	if(new_labels)
	{
		timebar->paste_output(startproject, 
						endproject, 
						startsource_sample, 
						endsource_sample, 
						new_labels);
	}

	if(gui)
	{
		draw();
	}

	if(asset->audio_data)
	{
		set_selectionend(startproject + endsource_sample - startsource_sample);
		if(endsource_sample > startsource_sample) assets->build_indexes();    // create if necessary and draw indexes
	}
	else 
	if(asset->video_data) set_selectionend(startproject + 
		tosamples(endsource_frame, sample_rate, frame_rate) - 
		tosamples(startsource_frame, sample_rate, frame_rate));
	return 0;
return 0;
}

int MainWindow::paste_transition(long startproject, 
				long endproject, 
				Transition *transition)
{

	tracks->paste_transition(startproject, 
						endproject, 
						transition);

	if(gui)
	{
		draw();
	}

	timebar->paste_silence(selectionstart, selectionend);
	changes_made = 1;

	return 0;
return 0;
}


int MainWindow::clear(long start, long end)
{
	if(start == end)
	{
		tracks->clear_handle(start, end);
	}
	else
	{
		tracks->clear(start, end);
		timebar->clear(start, end);
	}

	selectionend = selectionstart;
	if(gui) draw();
	changes_made = 1;	
return 0;
}

int MainWindow::feather_edits(long feather_samples, int audio, int video)
{
	if(selectionstart != selectionend)
	{
		stop_playback();

		tracks->feather_edits(selectionstart, selectionend, feather_samples, audio, video);
	}
return 0;
}

long MainWindow::get_feather(int audio, int video) 
{ 
// Want the default feather for this command to be the feather of the selection.
	return tracks->get_feather(selectionstart, selectionend, audio, video); 
}

float MainWindow::get_aspect_ratio()
{
	return aspect_w / aspect_h;
}

int MainWindow::create_aspect_ratio(float &w, float &h, int width, int height)
{
	int denominator;
	float fraction = (float)width / height;
	
	for(denominator = 1; 
		denominator < 100 && 
			fabs(fraction * denominator - (int)(fraction * denominator)) > .001; 
		denominator++)
		;
	
	w = denominator * width / height;
	h = denominator;
return 0;
}

int MainWindow::mute_audio()
{
	if(selectionstart != selectionend)
	{
// clear the data
		tracks->clear(selectionstart, selectionend);
		//timebar->clear(selectionstart, selectionend);

// paste silence
		tracks->paste_silence(selectionstart, selectionend);	
		//timebar->paste_silence(selectionstart, selectionend);

		if(gui) draw();
		changes_made = 1;	
	}
return 0;
}

int MainWindow::trim_selection()
{
	if(selectionstart != selectionend)
	{
// clear the data
		tracks->clear(0, selectionstart);
		timebar->clear(0, selectionstart);

// paste silence
		tracks->clear(selectionend - selectionstart, tracks->total_samples());	
		timebar->clear(selectionend - selectionstart, tracks->total_samples());

		if(gui) draw();
		set_selection(0, selectionend - selectionstart);
		changes_made = 1;	
	}
return 0;
}

int MainWindow::paste_silence()
{
	if(selectionstart != selectionend)
	{
		tracks->paste_silence(selectionstart, selectionend);	
		timebar->paste_silence(selectionstart, selectionend);
		if(gui) draw();
		changes_made = 1;
	}
return 0;
}

int MainWindow::cut_automation()
{
	copy_automation();
	clear_automation();
return 0;
}

int MainWindow::copy_automation()
{
	if(selectionstart == selectionend) return 1;
	FileHTAL htal;

	htal.tag.set_title("HTAL");
	htal.append_tag();
	htal.append_newline();

	htal.tag.set_title("AUTOCLIPBOARD");
	htal.tag.set_property("SAMPLES", selectionend - selectionstart);
	htal.append_tag();
	htal.append_newline();
	htal.append_newline();

	tracks->copy_automation(selectionstart, selectionend, &htal);	

	htal.tag.set_title("/AUTOCLIPBOARD");
	htal.append_tag();
	htal.append_newline();
	htal.append_newline();

	htal.tag.set_title("/HTAL");
	htal.append_tag();
	htal.append_newline();
	htal.terminate_string();

	gui->to_clipboard(htal.string);
	changes_made = 1;
return 0;
}

int MainWindow::paste_automation()
{
	FileHTAL htal;
	
// load from the clipboard
	{
		char *string;
		string = new char[gui->clipboard_len() + 1];
		gui->from_clipboard(string);

		htal.read_from_string(string);
		delete [] string;
	}
	
	int result = 0;
	
// scan file for HTAL block	
	do{
		result = htal.read_tag();
	}while(!result && !htal.tag.title_is("HTAL"));

// scan file for clipboard	
	if(!result)
	do{
		result = htal.read_tag();
	}while(!result && !htal.tag.title_is("AUTOCLIPBOARD"));

	long total_length;
	if(!result)
	{
		total_length = htal.tag.get_property("SAMPLES", (long)0);
	}

	Track* current_track = tracks->first;
	Patch* current_patch = patches->first;

// get first recordable track
	while(current_patch && !current_patch->record)
	{
		current_patch = current_patch->next;
		current_track = current_track->next;
	}

	while(!result && current_track)
	{
		result = htal.read_tag();
		
		if(!result)
		{
			if(!strcmp(htal.tag.get_title(), "/AUTOCLIPBOARD"))
			{
				result = 1;
			}
			else
			if(!strcmp(htal.tag.get_title(), "TRACK"))
			{
				current_track->paste_automation(selectionstart, selectionend, total_length, &htal, 0);

// get next recordable track
				do{
					current_patch = current_patch->next;
					current_track = current_track->next;
				}while(current_patch && !current_patch->record);
			}
		}
	}

	draw();
	set_selection(selectionstart, selectionstart + total_length);
return 0;
}

int MainWindow::clear_automation()
{
	if(selectionstart == selectionend)
	{
		return 1;
	}
	else
	{
		tracks->clear_automation(selectionstart, selectionend);	
	}

	draw();
	changes_made = 1;
return 0;
}

int MainWindow::toggle_label()
{
	changes_made = 1;
	undo->update_undo_timebar("Label", 0);
	if(playback_engine->is_playing_back) timebar->toggle_label(last_playback_position);
	else timebar->toggle_label();
	undo->update_undo_timebar();
return 0;
}

int MainWindow::modify_handles(long oldposition, long newposition, int currentend, int handle_mode)
{
	undo->update_undo_edits("Drag Handle", 0);
	tracks->modify_handles(oldposition, newposition, currentend, handle_mode);
	if(handle_mode == MOVE_ALL_EDITS)
		timebar->modify_handles(oldposition, newposition, currentend);
	undo->update_undo_edits();
	if(selectionstart < 0) set_selection(0, 0);
	draw();
	changes_made = 1;
return 0;
}

int MainWindow::change_channels(int old_channels, int new_channels)
{
	if(gui) gui->mainmenu->change_channels(old_channels, new_channels);
	tracks->change_channels(old_channels, new_channels);
	if(gui) console->gui->lock_window();
	console->change_channels(old_channels, new_channels);
	output_channels = new_channels;
	if(gui) console->gui->unlock_window();
	
	if(gui) level_window->change_channels(new_channels);
return 0;
}




int MainWindow::fix_timing(long &samples_out, long &frames_out, long samples_in)
{
	frames_out = (long)((float)samples_in / sample_rate * frame_rate);
	samples_out = (long)((float)frames_out / frame_rate * sample_rate);
return 0;
}

int MainWindow::purge_asset(char *path)
{
	Asset *asset = assets->get_asset(path);

	if(asset)
	{
		cache->delete_entry(asset);
		int result = tracks->purge_asset(asset);
		delete asset;



		if(result) draw();
	}
return 0;
}

int MainWindow::optimize_assets()
{
	Asset *asset = assets->first;
	Asset *next;
	
	while(asset)
	{
		next = asset->next;
		
		if(!tracks->asset_used(asset))
		{
			cache->delete_entry(asset);
			tracks->purge_asset(asset);
			delete asset;
		}
		asset = next;
	}
return 0;
}
