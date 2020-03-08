#include <string.h>
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "preferences.h"
#include "statusbar.h"
#include "trackcanvas.h"
#include "tracks.h"

TrackCanvas::TrackCanvas(MainWindow *mwindow, int x, int y, int w, int h)
 : BC_Canvas(x, y, w, h, BLACK)
{
	this->mwindow = mwindow;
	current_end = 0;
	selection_midpoint1 = selection_midpoint2 = 0;
	selection_type = 0;
	region_selected = 0;
	handle_selected = 0;
	auto_selected = 0;
	translate_selected = 0;
	which_handle = 0;
	handle_pixel = 0;
}

TrackCanvas::~TrackCanvas()
{
}

int TrackCanvas::button_press()
{
	int result, cursor_x, cursor_y;
	int enable_edit_handles;
	result = 0;
	if(mwindow->preferences->edit_handle_mode[get_buttonpress() - 1] != MOVE_EDITS_DISABLED)
		enable_edit_handles = 1;
	else
		enable_edit_handles = 0;

	cursor_x = get_cursor_x();
	cursor_y = get_cursor_y();
	if(get_cursor_x() > 0 && get_cursor_x() < get_w() && get_cursor_y() > 0 && get_cursor_y() < get_h())
	{
// make cursor_x position relative to sample
		if(mwindow->tracks_vertical)
		{
			cursor_x ^= cursor_y;
			cursor_y ^= cursor_x;
			cursor_x ^= cursor_y;
		}

// test handles
		if(enable_edit_handles && 
			(which_handle = mwindow->tracks->select_handle(cursor_x, 
								cursor_y, 
								handle_oldposition, 
								handle_position, 
								handle_pixel)))
		{
// Disable editing if the track wasn't recordable
			if(which_handle != 3)
			{
				handle_selected = 1;
				handle_pixel = cursor_x;
				handle_mode = mwindow->preferences->edit_handle_mode[get_buttonpress() - 1];
			}
			else
			which_handle = 0;
		}
		else
// Test video translation.
		if(get_buttonpress() == 2)
		{
			translate_selected = mwindow->tracks->select_translation(cursor_x, cursor_y);
		}
		else
// Test transition menus
		if(get_buttonpress() == 3 && !ctrl_down() && !shift_down())
		{
			mwindow->tracks->popup_transition(cursor_x, cursor_y);
		}
		else
// test autos
		if(auto_selected = mwindow->tracks->select_auto(cursor_x, cursor_y))
		{
			;
		}
		else
		{
// select region
			long position = mwindow->view_start + (long)cursor_x * mwindow->zoom_sample;
			if(mwindow->cursor_on_frames)
			{
				position = mwindow->align_to_frames(position);
			}
		
			mwindow->init_selection(position, 
								cursor_x, 
								cursor_y, 
								current_end, 
								selection_midpoint1,
								selection_midpoint2,
								selection_type);
			region_selected = 1;
		}
		return 1;
	}
	return result;
return 0;
}

int TrackCanvas::button_release()
{
	region_selected = 0;
	if(auto_selected) mwindow->tracks->release_auto();
	auto_selected = 0;
// only end handle_selecion if floating
	if(handle_selected == 2) end_handle_selection();
	handle_selected = 0;
	handle_pixel = mwindow->tracks_vertical ? w / 2 : h / 2;
	if(translate_selected) end_translation();
	translate_selected = 0;
return 0;
}

int TrackCanvas::cursor_motion()
{
	int result, cursor_x, cursor_y;

//printf("TrackCanvas::cursor_motion 1\n");
	result = 0;
	cursor_x = get_cursor_x();
	cursor_y = get_cursor_y();

// fix cursor position for vertical tracks
	if(region_selected || auto_selected || handle_selected || translate_selected)
	{
		result = 1;
		if(mwindow->tracks_vertical)
		{
			cursor_x ^= cursor_y;
			cursor_y ^= cursor_x;
			cursor_x ^= cursor_y;
		}

		long sample_position = mwindow->view_start + (long)cursor_x * mwindow->zoom_sample;
		if(mwindow->cursor_on_frames)
		{
			sample_position = mwindow->align_to_frames(sample_position);
		}

// translation of video
		if(translate_selected)
		{
			mwindow->tracks->update_translation(cursor_x, cursor_y, mwindow->gui->shift_down());
		}
// region selection
		else
		if(region_selected)         
		{
			auto_reposition(cursor_x, cursor_y, sample_position);

// check track position here
			mwindow->update_selection(sample_position, 
								cursor_x, 
								cursor_y, 
								current_end, 
								selection_midpoint1,
								selection_midpoint2,
								selection_type);
		}
		else
// automation selection
		if(auto_selected)
		{
			mwindow->tracks->draw_floating_autos(0);
			auto_reposition(cursor_x, cursor_y, sample_position);
			mwindow->tracks->move_auto(cursor_x, cursor_y, mwindow->gui->shift_down());
			mwindow->tracks->draw_floating_autos(1);
		}
		else
// test for edit handle selection
		if(handle_selected == 1)
		{
//printf("%d %d\n", cursor_x, handle_pixel);
			if(cursor_x < handle_pixel - 5 || cursor_x > handle_pixel + 5)
			{                             // initialize floating handle
				handle_selected = 2;  // next stage
				handle_position = sample_position;
				draw_floating_handle(1);
			}
		}
		else
		if(handle_selected == 2)
		{
			draw_floating_handle(0);       // hide

			auto_reposition(cursor_x, cursor_y, sample_position);
			
			update_handle_selection(sample_position);
		}
	}
	else
	{
// update the status bar with cursor position
		if(cursor_x > 0 && cursor_x < w && cursor_y > 0 && cursor_y < h &&
			!mwindow->is_playing_back)
		{
			long position = mwindow->view_start + (long)(mwindow->tracks_vertical ? cursor_y : cursor_x) * mwindow->zoom_sample;
			mwindow->gui->statusbar->update_playback(position);
		}
	}
//printf("TrackCanvas::cursor_motion 2\n");
	return result;
return 0;
}

int TrackCanvas::auto_reposition(int &cursor_x, int &cursor_y, long cursor_position)
{
	if(!auto_selected)
	{
// don't want vertical movement for automation
		if(cursor_y < 0 && mwindow->tracks->view_start > 0)
		{
			int old_view_start = mwindow->tracks->view_start;

			if(mwindow->tracks_vertical)
			{ mwindow->tracks->move_down(-cursor_y); }
			else
			{ mwindow->tracks->move_up(-cursor_y); }

			cursor_y += old_view_start - mwindow->tracks->view_start;
		}
		else if(cursor_y > (mwindow->tracks_vertical ? w : h))
		{
			int old_view_start = mwindow->tracks->view_start;

			if(mwindow->tracks_vertical)
			{ mwindow->tracks->move_up(cursor_y - w); }
			else
			{ mwindow->tracks->move_down(cursor_y - h); }

			cursor_y += old_view_start - mwindow->tracks->view_start;
		}
	}

	if(auto_selected < 2)
	{
		if(cursor_position < mwindow->view_start && mwindow->view_start > 0)
		{
			if(cursor_position < 0) cursor_position = 0;
			long distance = mwindow->view_start - cursor_position;
			cursor_x += distance / mwindow->zoom_sample;
			mwindow->move_left(distance);
		}
		else 
		if(cursor_position > mwindow->view_start + mwindow->tracks->view_samples())
		{
			long distance = cursor_position -  mwindow->view_start - mwindow->tracks->view_samples();
			cursor_x -= distance / mwindow->zoom_sample;
			mwindow->move_right(distance);
		}
	}
	mwindow->gui->statusbar->update_playback(cursor_position);
return 0;
}


int TrackCanvas::draw_floating_handle(int flash)
{
	static int pixel, hx, hy;

	set_inverse();
	set_color(WHITE);

	pixel = (handle_position - mwindow->view_start) / mwindow->zoom_sample;

	if(mwindow->tracks_vertical)
	{
		hx = w / 2;
		hy = pixel;
		draw_line(0, pixel, w, pixel);
	}
	else
	{
		hx = pixel;
		hy = h / 2;
		draw_line(pixel, 0, pixel, h);
	}

	if(which_handle == 1) 
	draw_start_edit(hx, hy, mwindow->tracks_vertical);
	else 
	draw_end_edit(hx, hy, mwindow->tracks_vertical);

	set_opaque();
	if(flash) this->flash();
return 0;
}

int TrackCanvas::draw_loop_point(long position, int flash)
{
	static int pixel;
	set_inverse();
	set_color(GREEN);
	
	pixel = (position - mwindow->view_start) / mwindow->zoom_sample;

	if(mwindow->tracks_vertical)
	draw_line(0, pixel, w, pixel);
	else
	draw_line(pixel, 0, pixel, h);
	
	set_opaque();
	if(flash) this->flash();
return 0;
}

int TrackCanvas::draw_playback_cursor(int pixel, int flash)
{
	set_inverse();
	set_color(PINK);
	
	if(mwindow->tracks_vertical)
	{
		draw_line(0, pixel, w, pixel);
		set_opaque();
		if(flash) this->flash(0, pixel, w, 1);
	}
	else
	{
		draw_line(pixel, 0, pixel, h);
		set_opaque();
		if(flash) this->flash(pixel, 0, 1, h);
	}
return 0;
}


int TrackCanvas::update_handle_selection(long cursor_position)
{
	handle_position = cursor_position;
	draw_floating_handle(1);         // show
return 0;
}

int TrackCanvas::end_handle_selection()
{
	handle_selected = 0;
	draw_floating_handle(1);       // hide
// determine new cursor position

	if(which_handle == 2) 
		mwindow->set_selection(handle_position, handle_position);
	else
		mwindow->set_selection(handle_oldposition, handle_oldposition);

	mwindow->modify_handles(handle_oldposition, handle_position, which_handle, handle_mode);
return 0;
}

int TrackCanvas::end_translation()
{
	mwindow->tracks->end_translation();
return 0;
}

// long TrackCanvas::align_to_frames(long sample)
// {
// 	long frame = (long)(((float)sample / mwindow->sample_rate) * mwindow->frame_rate);
// 	long result = (long)(((float)frame / mwindow->frame_rate) * mwindow->sample_rate);
//     return result;
// }
