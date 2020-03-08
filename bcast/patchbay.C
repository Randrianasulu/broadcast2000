#include <string.h>
#include "filehtal.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "patchbay.h"
#include "track.h"
#include "tracks.h"

// ==================================== patchbay

PatchBay::PatchBay(MainWindow *mwindow)
 : List<Patch>()
{
	gui = 0;
	button_down = 0;
	reconfigure_trigger = 0;
	this->mwindow = mwindow;
}

PatchBay::~PatchBay() 
{
	delete_all();
	if(gui) delete gui;
}

int PatchBay::create_objects(int top, int bottom)
{
	if(mwindow->gui) create_gui(top, bottom);
return 0;
}

int PatchBay::create_gui(int top, int bottom)
{
	int x, y, w, h;
	if(mwindow->tracks_vertical)
	{
		x = bottom;
		y = mwindow->gui->menu_h();
		w = top - bottom;
		h = PATCHBAYHEIGHT;
	}
	else
	{
		x = 0;
		y = top;
		w = PATCHBAYWIDTH;
		h = bottom - top;
	}
	
	mwindow->gui->add_subwindow(gui = new PatchBayGUI(mwindow, x, y, w, h));
	gui->add_border();
return 0;
}

int PatchBay::load_patches(FileHTAL *htal, Patch *current_patch)
{
	int result = 0;
	
	do{
		result = htal->read_tag();
		if(!result)
		{
			if(htal->tag.title_is("/TRACK"))
			{
				result = 1;
			}
			else
			if(htal->tag.title_is("PATCH"))
			{
				current_patch->load(htal);
			}
		}
	}while(!result);
return 0;
}

int PatchBay::resize_event(int top, int bottom)
{
	if(gui)
	{
		if(top - bottom)
		{
			if(mwindow->tracks_vertical)
			{ gui->resize_window(bottom, gui->get_y(), top - bottom, gui->get_h()); }
			else
			{ if(gui) gui->resize_window(0, top, gui->get_w(), bottom - top); }
		}
		else
		{
			//delete_gui(top, bottom);
			//gui = 0;
		}
	}
	else
	{
		if(top - bottom)
		{
		}
	}
return 0;
}

int PatchBay::flip_vertical(int top, int bottom)
{
	if(gui)
	{
		if(mwindow->tracks_vertical)
		{ gui->resize_window(bottom, mwindow->gui->menu_h(), top - bottom, PATCHBAYHEIGHT); }
		else
		{ gui->resize_window(0, top, PATCHBAYWIDTH, bottom - top); }


		Patch *current;
		for(current = first; current; current = NEXT)
		{ current->flip_vertical(); }
	}
return 0;
}

int PatchBay::add_track(int start_pixel, char *default_title, int data_type)
{
	int pixel;
	Patch *new_patch;
	
	if(last)   // set new y
	{          // additional patch
		pixel = last->pixel;
		pixel += mwindow->zoom_track;
	}
	else
	{        // first patch
		pixel = 0 - start_pixel;
	}
	
	char string[1024];
	if(data_type == TRACK_AUDIO)
	sprintf(string, "%s %d", default_title, total_audio() + 1);
	else
	sprintf(string, "%s %d", default_title, total_video() + 1);
	new_patch = new Patch(mwindow, this, data_type);
	new_patch->create_objects(string, pixel);
	append(new_patch);
return 0;
}

int PatchBay::delete_track(int start_pixel)
{
	delete_track(last, start_pixel);
return 0;
}

int PatchBay::delete_track(Patch *patch, int start_pixel)
{
	remove(patch);
// fix pixel coords if not the last patch
	//redo_pixels(int start_pixel);
return 0;
}

int PatchBay::delete_all()
{
	while(last) remove(last);
return 0;
}

int PatchBay::expand_t(int start_pixel)
{
	if(gui)
	{
		redo_pixels(start_pixel);
	}
return 0;
}

int PatchBay::zoom_in_t(int start_pixel)
{
	if(gui)
	{
		redo_pixels(start_pixel);
	}
return 0;
}

int PatchBay::trackmovement(int distance)
{
	if(gui)
	{
		for(current = first; current; current = NEXT)
		{ current->pixelmovement(distance); }
	}
return 0;
}

int PatchBay::redo_pixels(int start_pixel)
{
	int pixel;
	Patch *current;
	
	if(first)
	{
		for(pixel = 0, current = first; current; current = NEXT, pixel += mwindow->zoom_track)
		{
			current->set_pixel(pixel - start_pixel);
		}
	}
return 0;
}

int PatchBay::number_of(Patch *patch)
{
	int i = 0;
	for(Patch *current = first; current && current != patch; current = NEXT)
	{
		i++;
	}
	return i;
return 0;
}

Patch* PatchBay::number(int number)
{
	Patch *current;
	int i = 0;
	for(current = first; current && i < number; current = NEXT)
	{
		i++;
	}
	return current;
}

int PatchBay::copy_length()
{
	Patch *current;
	int i = 0;
	for(current = first; current; current = NEXT)
	{
		if(current->record) i++;
	}
	return i;
return 0;
}

BC_TextBox* PatchBay::atrack_title_number(int number)    // return textbox of atrack #
{
	static int i;
	
	for(current = first, i = 0; i < number && current; i++, current = NEXT)
		;
	
	return current->title_text;
}

int PatchBay::total_playable_atracks()
{
	Patch *current;
	Track *current_track;
	int result = 0;

	for(current = first, current_track = mwindow->tracks->first; 
		current; current = NEXT, current_track = current_track->next)
	{
		if(current->play && current_track->data_type == TRACK_AUDIO) result++;
	}
	return result;
return 0;
}

int PatchBay::total_playable_vtracks()
{
	Patch *current;
	Track *current_track;
	int result = 0;

	for(current = first, current_track = mwindow->tracks->first; 
		current; current = NEXT, current_track = current_track->next)
	{
		if(current->play && current_track->data_type == TRACK_VIDEO) result++;
	}
	return result;
return 0;
}

int PatchBay::total_recordable_tracks(int data_type)
{
	Patch *current;
	Track *current_track;
	int result = 0;

	for(current = first, current_track = mwindow->tracks->first; 
		current; current = NEXT, current_track = current_track->next)
	{
		if(current->record && current_track->data_type == data_type) result++;
	}
	return result;
}

int PatchBay::total_recordable_atracks()
{
	Patch *current;
	Track *current_track;
	int result = 0;

	for(current = first, current_track = mwindow->tracks->first; 
		current; current = NEXT, current_track = current_track->next)
	{
		if(current->record && current_track->data_type == TRACK_AUDIO) result++;
	}
	return result;
return 0;
}

int PatchBay::total_recordable_vtracks()
{
	Patch *current;
	Track *current_track;
	int result = 0;

	for(current = first, current_track = mwindow->tracks->first; 
		current; current = NEXT, current_track = current_track->next)
	{
		if(current->record && current_track->data_type == TRACK_VIDEO) result++;
	}
	return result;
return 0;
}

int PatchBay::deselect_all_play()
{
	for(Patch* current = first; current; current = NEXT)
	{
		current->play = 0;
		if(mwindow->gui) current->playpatch->update(0);
	}
return 0;
}

int PatchBay::select_all_play()
{
	for(Patch* current = first; current; current = NEXT)
	{
		current->play = 1;
		if(mwindow->gui) current->playpatch->update(1);
	}
return 0;
}

int PatchBay::deselect_all_record()
{
	for(Patch* current = first; current; current = NEXT)
	{
		current->record = 0;
		if(mwindow->gui) current->recordpatch->update(0);
	}
return 0;
}

int PatchBay::select_all_record()
{
	for(Patch* current = first; current; current = NEXT)
	{
		current->record = 1;
		if(mwindow->gui) current->recordpatch->update(1);
	}
return 0;
}

int PatchBay::deselect_all_auto()
{
	for(Patch* current = first; current; current = NEXT)
	{
		current->automate = 0;
		if(mwindow->gui) current->autopatch->update(0);
	}
return 0;
}

int PatchBay::deselect_all_draw()
{
	for(Patch* current = first; current; current = NEXT)
	{
		current->draw = 0;
		if(mwindow->gui) current->drawpatch->update(0);
	}
return 0;
}

int PatchBay::select_all_auto()
{
	for(Patch* current = first; current; current = NEXT)
	{
		current->automate = 1;
		if(mwindow->gui) current->autopatch->update(1);
	}
return 0;
}

int PatchBay::select_all_draw()
{
	for(Patch* current = first; current; current = NEXT)
	{
		current->draw = 1;
		if(mwindow->gui) current->drawpatch->update(1);
	}
return 0;
}

int PatchBay::plays_selected()
{
	int result = 0;
	
	for(Patch* current = first; current; current = NEXT)
	{
		if(current->play) result++;
	}
	return result;
return 0;
}

int PatchBay::records_selected()
{
	int result = 0;
	
	for(Patch* current = first; current; current = NEXT)
	{
		if(current->record) result++;
	}
	return result;
return 0;
}

int PatchBay::autos_selected()
{
	int result = 0;
	
	for(Patch* current = first; current; current = NEXT)
	{
		if(current->automate) result++;
	}
	return result;
return 0;
}

int PatchBay::draws_selected()
{
	int result = 0;
	
	for(Patch* current = first; current; current = NEXT)
	{
		if(current->draw) result++;
	}
	return result;
return 0;
}

int PatchBay::total_audio()
{
	int result = 0;
	
	for(Patch* current = first; current; current = NEXT)
	{
		if(current->data_type == TRACK_AUDIO) result++;
	}
	return result;
return 0;
}

int PatchBay::total_video()
{
	int result = 0;
	
	for(Patch* current = first; current; current = NEXT)
	{
		if(current->data_type == TRACK_VIDEO) result++;
	}
	return result;
return 0;
}

PatchBayGUI::PatchBayGUI(MainWindow *mwindow, int x, int y, int w, int h)
 : BC_SubWindow(x, y, w, h, MEGREY)
{
	this->mwindow = mwindow;
	patches = mwindow->patches;
}

PatchBayGUI::~PatchBayGUI()
{
}


int PatchBayGUI::cursor_motion()
{
	int result;
	result = 0;

	if(mwindow->patches->button_down)
	{
		if(mwindow->tracks_vertical)
		{
			if(cursor_x < 0) mwindow->tracks->move_up(-get_cursor_x());
			if(cursor_x > get_w()) mwindow->tracks->move_down(get_cursor_x() - get_w());
		}
		else
		{
			if(cursor_y < 0) mwindow->tracks->move_up(-get_cursor_y());
			if(cursor_y > get_h()) mwindow->tracks->move_down(get_cursor_y() - get_h());
		}
		result = 1;
	}
	return result;
return 0;
}

int PatchBayGUI::button_release()
{
	if(patches->button_down && patches->reconfigure_trigger)
	{
		patches->button_down = 0;
		patches->reconfigure_trigger = 0;
// restart the playback
		mwindow->start_reconfigure(1);
		mwindow->stop_reconfigure(1);
		return 1;
	}
	patches->button_down = 0;
	return 0;
return 0;
}
