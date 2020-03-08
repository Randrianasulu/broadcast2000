#include <string.h>
#include "cursor.h"
#include "file.h"
#include "filehtal.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "module.h"
#include "patchbay.h"
#include "track.h"
#include "trackcanvas.h"
#include "tracks.h"

Tracks::Tracks(MainWindow *mwindow)
 : List<Track>()
{
	this->mwindow = mwindow;
	canvas = 0;
	cursor = 0;
	overlays_visible = 0;
	handles = 0;
	titles = 0;
}

Tracks::~Tracks()
{
	if(cursor) delete cursor;
	if(canvas) delete canvas;
}

int Tracks::create_objects(Defaults *defaults, int w, int h, int top, int bottom)
{
	load_defaults(defaults);

	if(mwindow->gui)
	{
		if(mwindow->tracks_vertical)
			{ mwindow->gui->add_tool(canvas = new TrackCanvas(mwindow, bottom, PATCHBAYHEIGHT + mwindow->gui->menu_h(), top - bottom, h - PATCHBAYHEIGHT - mwindow->gui->menu_h() - 24 - 17)); }
		else
			{ mwindow->gui->add_tool(canvas = new TrackCanvas(mwindow, PATCHBAYWIDTH, top, w - PATCHBAYWIDTH - 17, bottom - top)); }

		cursor = new Cursor_(canvas);
		show_overlays(0);
	}
	return 0;
return 0;
}

int Tracks::load_defaults(Defaults *defaults)
{
	handles = defaults->get("SHOWHANDLES", 1);
	titles = defaults->get("SHOWTITLES", 1);
	show_output = defaults->get("SHOWOUTPUT", 1);
	auto_conf.load_defaults(defaults);
	return 0;
return 0;
}

int Tracks::save_defaults(Defaults *defaults)
{
	defaults->update("SHOWTITLES", titles);
	defaults->update("SHOWHANDLES", handles);
	defaults->update("SHOWOUTPUT", show_output);
	auto_conf.save_defaults(defaults);
	return 0;
return 0;
}

int Tracks::set_index_file(int flash, Asset *asset)
{
	Track* current;
	int result;

	result = 1;

	if(mwindow->gui)
	{
		hide_overlays(0);

		for(current = first; current; current = NEXT)
		{
			if(!(current->set_index_files(0, asset))) result = 0;
		}

		show_overlays(flash);
	}
	
	return result;
return 0;
}


int Tracks::resize_event(int w, int h, int top, int bottom)
{
	if(mwindow->tracks_vertical)
		{ canvas->set_size(bottom, PATCHBAYHEIGHT + mwindow->gui->menu_h(), top - bottom, h - PATCHBAYHEIGHT - mwindow->gui->menu_h() - 24 - 17); }
	else
		{ canvas->set_size(PATCHBAYWIDTH, top, w - PATCHBAYWIDTH - 17, bottom - top); }
	return 0;
return 0;
}

int Tracks::flip_vertical(int top, int bottom)
{
	resize_event(mwindow->gui->get_w(), mwindow->gui->get_h(), top, bottom);
	return 0;
return 0;
}


int Tracks::dump()
{
	for(Track* current = first; current; current = NEXT)
	{
		printf("Track %x\n", current);
		current->dump();
		current->get_module_of()->dump();
	}
	return 0;
return 0;
}

int Tracks::vrender_init(int duplicate, long position)
{
	Track *current;
	for(current = first; current; current = NEXT)
	{
		if(current->data_type == TRACK_VIDEO) current->render_init(0, duplicate, position);
	}
	return 0;
return 0;
}

int Tracks::arender_init(int realtime_sched, int duplicate, long position)
{
	Track *current;
	for(current = first; current; current = NEXT)
	{
		if(current->data_type == TRACK_AUDIO) current->render_init(realtime_sched, duplicate, position);
	}
	return 0;
return 0;
}

int Tracks::vrender_stop(int duplicate)
{
	Track *current;
	for(current = first; current; current = NEXT)
	{
		if(current->data_type == TRACK_VIDEO) current->render_stop(duplicate);
	}
	return 0;
return 0;
}

int Tracks::arender_stop(int duplicate)
{
	Track *current;
	for(current = first; current; current = NEXT)
	{
		if(current->data_type == TRACK_AUDIO) current->render_stop(duplicate);
	}
	return 0;
return 0;
}

// ===================================== file operations

int Tracks::save(FileHTAL *htal)
{
// save each track
	for(Track* current = first; current; current = NEXT)
	{ current->save(htal); }
	return 0;
return 0;
}

int Tracks::load(FileHTAL *htal, int track_offset)
{
// add the appropriate type of track
	char string[1024];
	sprintf(string, "");
	
	htal->tag.get_property("TYPE", string);
	
	if(!strcmp(string, "VIDEO"))
	{
		add_video_track();
	}
	else
	{
		add_audio_track();    // default to audio
	}
// load it
	last->load(htal, track_offset, 0, 0);
	return 0;
return 0;
}

int Tracks::popup_transition(int cursor_x, int cursor_y)
{
	int result = 0;
	for(Track* current = first; current && !result; current = NEXT)
	{
		result = current->popup_transition(cursor_x, cursor_y);
	}
	return result;
return 0;
}


int Tracks::change_channels(int oldchannels, int newchannels)
{
	for(Track *current = first; current; current = NEXT)
	{ current->change_channels(oldchannels, newchannels); }
	return 0;
return 0;
}

long Tracks::view_samples() 
{ 
	if(mwindow->gui)
		return mwindow->tracks_vertical ? (long)canvas->h * mwindow->zoom_sample : (long)canvas->w * mwindow->zoom_sample; 
	else
		return 0;
}

int Tracks::view_pixels()
{
	if(mwindow->gui)
		return mwindow->tracks_vertical ? canvas->h : canvas->w;
	else
		return 0;
return 0;
}

int Tracks::vertical_pixels()
{
	if(mwindow->gui)
		return mwindow->tracks_vertical ? canvas->w : canvas->h;
	else
		return 0;
return 0;
}



long Tracks::total_samples() 
{
	long total = 0;
	for(Track *current = first; current; current = NEXT)
	{
		if(current->length() > total) total = current->length();
	}
	return total; 
}

long Tracks::total_playable_samples() 
{
	long total = 0;
	Track *current_track;
	Patch *current_patch;
	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->play && current_track->length() > total) total = current_track->length();
	}
	return total; 
}

int Tracks::totalpixels()
{
	int result = 0;
	for(Track* current = first; current; current = NEXT)
	{
		result += mwindow->zoom_track;
	}
	return result;
return 0;
}

int Tracks::number_of(Track *track)
{
	int i = 0;
	for(Track *current = first; current && current != track; current = NEXT)
	{
		i++;
	}
	return i;
return 0;
}

Track* Tracks::number(int number)
{
	Track *current;
	int i = 0;
	for(current = first; current && i < number; current = NEXT)
	{
		i++;
	}
	return current;
}

int Tracks::copy_length(long start, long end)
{
	return mwindow->patches->copy_length();
return 0;
}
