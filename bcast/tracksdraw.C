#include <string.h>
#include "cache.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "patchbay.h"
#include "track.h"
#include "trackcanvas.h"
#include "tracks.h"
#include "trackscroll.h"

//============================== atrack drawing ======================

int Tracks::draw(int flash)
{
	if(canvas && canvas->h * canvas->w > 0)
	{
		draw(0, canvas->w, 0, canvas->h, flash);
	}
return 0;
}

int Tracks::draw(int x, int w, int y, int h, int flash)
{
	if(canvas && canvas->h * canvas->w > 0)
	{
		canvas->clear_box(x, y, w, h);
		for(current = first; current; current = NEXT)
		{
			current->draw(x, w, y, h, flash);
		}
		if(flash == 1) canvas->flash(x, y, w, h);

		mwindow->cache->age_audio();
		mwindow->cache->age_video();
		
		overlays_visible = 0;
 	}
return 0;
}

int Tracks::draw_floating_handle(int flash)
{
	if(canvas)	canvas->draw_floating_handle(flash);
return 0;
}

int Tracks::show_overlays(int flash)
{
	if(mwindow->gui && !overlays_visible)
	{
		mwindow->show_playback_cursor(-1, 0);
		if(titles) draw_titles(0);
		draw_autos(0);
		if(handles) draw_handles(0);
		if(mwindow->loop_playback) draw_loop_points(0);
		draw_cursor(flash);
		overlays_visible = 1;
	}
return 0;
}

int Tracks::hide_overlays(int flash)
{
	if(mwindow->gui && overlays_visible)
	{
		mwindow->show_playback_cursor(-1, 0);
		draw_autos(0);     // draw if there are inverse autos
		if(handles) draw_handles(0);
		if(mwindow->loop_playback) draw_loop_points(0);
		draw_cursor(flash);
		overlays_visible = 0;
	}
return 0;
}


int Tracks::draw_loop_points(int flash) 
{ 
	if(mwindow->gui)
		draw_loop_points(mwindow->loop_start, mwindow->loop_end, flash); 
return 0;
}

int Tracks::set_draw_output()
{
	hide_overlays(0);
	show_output ^= 1;
	draw(0);
	show_overlays(1);
return 0;
}

int Tracks::toggle_handles()
{
	if(mwindow->gui)
	{
		hide_overlays(0);
		handles ^= 1;
		show_overlays(1);
	}
return 0;
}

int Tracks::draw_handles(int flash)
{
	if(canvas)
	{
		for(Track *current = first; current; current = NEXT)
		{
			current->draw_handles();
		}
		if(flash == 1) canvas->flash();
	}
return 0;
}


int Tracks::toggle_titles()
{
	if(mwindow->gui)
	{
		hide_overlays(0);
		titles ^= 1;
		if(titles) draw_titles(1);
		else
		{
			draw();
		}
		show_overlays(1);
	}
return 0;
}

int Tracks::draw_titles(int flash)
{
	if(mwindow->gui)
	{
		for(Track *current = first; current; current = NEXT)
		{
			current->draw_titles();
		}
		if(flash == 1) canvas->flash();
	}
return 0;
}

int Tracks::set_show_autos(int camera, int project)
{
// need to redraw waveforms if these autos are opaque
// otherwise hide_overlays will draw inverted autos
	if(auto_conf.camera || auto_conf.projector)
		draw(1);

	hide_overlays(0);

	auto_conf.camera = camera;
	auto_conf.projector = project;
	show_overlays(1);
return 0;
}

int Tracks::toggle_auto_fade()
{
	if(mwindow->gui)
	{
		hide_overlays(0);
		auto_conf.fade ^= 1;
		draw();
		show_overlays(1);
	}
return 0;
}

int Tracks::toggle_auto_camera()
{
	if(mwindow->gui)
	{
		hide_overlays(0);
		auto_conf.camera ^= 1;
		draw();
		show_overlays(1);
	}
return 0;
}

int Tracks::toggle_auto_project()
{
	if(mwindow->gui)
	{
		hide_overlays(0);
		auto_conf.projector ^= 1;
		draw();
		show_overlays(1);
	}
return 0;
}

int Tracks::toggle_auto_play()
{
	if(mwindow->gui)
	{
		hide_overlays(0);
		auto_conf.play ^= 1;
		draw();
		show_overlays(1);
	}
return 0;
}

int Tracks::toggle_auto_mute()
{
	if(mwindow->gui)
	{
		hide_overlays(0);
		auto_conf.mute ^= 1;
		draw();
		show_overlays(1);
	}
return 0;
}

int Tracks::toggle_auto_pan(int pan)
{
	if(mwindow->gui)
	{
		hide_overlays(0);
		auto_conf.pan[pan] ^= 1;
		draw();
		show_overlays(1);
	}
return 0;
}

int Tracks::toggle_auto_plugin(int plugin)
{
	if(mwindow->gui)
	{
		hide_overlays(0);
		auto_conf.plugin[plugin] ^= 1;
		draw();
		show_overlays(1);
	}
return 0;
}

int Tracks::draw_autos(int flash)
{
	if(canvas)
	{
		for(Track *current = first; current; current = NEXT)
		{
			current->draw_autos(&auto_conf);
		}
		if(flash == 1) canvas->flash();
	}
return 0;
}

int Tracks::draw_floating_autos(int flash)
{
	if(canvas)
	{
		for(Track *current = first; current; current = NEXT)
		{
			current->draw_floating_autos(&auto_conf, 0);
		}
		if(flash) canvas->flash();
	}
return 0;
}

int Tracks::draw_cursor(int flash)
{
	if(canvas)
	{
//printf("Tracks::draw_cursor\n");
		long start = mwindow->view_start;
		long end = start + (long)(mwindow->tracks_vertical ? canvas->h : canvas->w) * mwindow->zoom_sample;
		int pixel1, pixel2;

		if((mwindow->selectionstart >= start && mwindow->selectionstart <= end) ||
			 (mwindow->selectionend >= start && mwindow->selectionend <= end) ||
			 (start >= mwindow->selectionstart && end <= mwindow->selectionend))
		{
// cursor in range
			if(mwindow->selectionstart < start)
			{
				pixel1 = 0;
			}
			else
			{
				pixel1 = (mwindow->selectionstart - start) / mwindow->zoom_sample;
			}

			if(mwindow->selectionend > end)
			{
				pixel2 = (mwindow->tracks_vertical ? canvas->h : canvas->w);
			}
			else
			{
				pixel2 = (mwindow->selectionend - start) / mwindow->zoom_sample;
			}
			pixel2++;

			canvas->set_inverse();
			canvas->set_color(WHITE);

			if(mwindow->tracks_vertical)
			canvas->draw_box(0, pixel1, canvas->w, pixel2 - pixel1);
			else
			canvas->draw_box(pixel1, 0, pixel2 - pixel1, canvas->h);


			canvas->set_opaque();
		}
		if(flash) canvas->flash();
	}
return 0;
}

int Tracks::draw_playback_cursor(int x, int flash)
{
	if(!canvas || canvas->h * canvas->w == 0) return 1;
	canvas->draw_playback_cursor(x, flash);
return 0;
}

int Tracks::draw_loop_points(long start, long end, int flash)
{
	if(!canvas || canvas->h * canvas->w == 0) return 1;
	canvas->draw_loop_point(start, flash);
	canvas->draw_loop_point(end, flash);
return 0;
}


int Tracks::zoom_y()
{
	if(mwindow->zoom_y < 65536)
	{
		mwindow->zoom_y *= 2;
		draw();
		mwindow->gui->trackscroll->update();
	}
return 0;
}

int Tracks::expand_y()
{
	if(mwindow->zoom_y > 1)
	{
		mwindow->zoom_y /= 2;
		draw();
		mwindow->gui->trackscroll->update();
	}
return 0;
}

int Tracks::move_up(long distance)
{
	if(!distance) distance = mwindow->zoom_track;
	if(view_start < distance)
	{
		distance = view_start;
	}
	trackmovement(-distance);
return 0;
}

int Tracks::move_down(long distance)
{
	if(view_start >= totalpixels() - vertical_pixels()) return 1;
	if(!distance) distance = mwindow->zoom_track;
	if(view_start + distance > totalpixels() - vertical_pixels())
	{
		distance = (totalpixels() - vertical_pixels()) - view_start;
	}
	trackmovement(distance);
return 0;
}


int Tracks::samplemovement(long distance)
{
//printf("Tracks::samplemovement 1\n");
	if(!canvas) return 1;
	int pixels, new_pixel, old_pixel;

	if(distance > 0)
	{
		if(distance / mwindow->zoom_sample < view_pixels())
		{
			//pixels = distance / mwindow->zoom_sample;
			old_pixel = mwindow->view_start / mwindow->zoom_sample;
			new_pixel = (mwindow->view_start + distance) / mwindow->zoom_sample;
			pixels = new_pixel - old_pixel;
		}
		else pixels = view_pixels();
		
		if(mwindow->tracks_vertical)
		{
			canvas->slide_up(pixels);
			draw(0, canvas->w, canvas->h - pixels, pixels, 2);
		}
		else
		{
			canvas->slide_left(pixels);
			draw(canvas->w - pixels, pixels, 0, canvas->h, 2);
		}
	}
	else
	if(distance < 0)
	{
		if(-distance / mwindow->zoom_sample < view_pixels())
		{
			old_pixel = mwindow->view_start / mwindow->zoom_sample;
			new_pixel = (mwindow->view_start + distance) / mwindow->zoom_sample;
			pixels = old_pixel - new_pixel;
			//pixels = -distance / mwindow->zoom_sample;
		}
		else pixels = view_pixels();

		if(mwindow->tracks_vertical)
		{
			canvas->slide_down(pixels);
			draw(0, canvas->w, 0, pixels, 2);
		}
		else
		{
			canvas->slide_right(pixels);
			draw(0, pixels, 0, canvas->h, 2);
		}
	}
//printf("Tracks::samplemovement 2\n");
return 0;
}

int Tracks::trackmovement(long distance)
{
	hide_overlays(0);

	view_start += distance;
	int pixels;
	
	if(!canvas) return 1;
	if(distance > 0 && canvas->h > 0)
	{
		if(distance < vertical_pixels()) pixels = distance;
		else pixels = vertical_pixels();
		
		for(current = first; current; current = NEXT)    // tracks
		{
			current->pixel -= distance;
		}
		
		if(mwindow->tracks_vertical)
		{
			canvas->slide_left(pixels);      // canvas
			draw(canvas->w - pixels, pixels, 0, canvas->h, 0);
		}
		else
		{
			canvas->slide_up(pixels);      // canvas
			draw(0, canvas->w, canvas->h - pixels, pixels, 0);
		}
	}
	else
	if(distance < 0 && canvas->h > 0)
	{
		if(-distance < vertical_pixels()) pixels = -distance;
		else pixels = vertical_pixels();

		for(current = first; current; current = NEXT)    // tracks
		{
			current->pixel -= distance;
		}

		if(mwindow->tracks_vertical)
		{
			canvas->slide_right(pixels);      // canvas
			draw(0, pixels, 0, canvas->h, 0);
		}
		else
		{
			canvas->slide_down(pixels);      // canvas
			draw(0, canvas->w, 0, pixels, 0);
		}
	}

	mwindow->patches->trackmovement(distance);
	
	show_overlays(1);
	mwindow->gui->trackscroll->update();
return 0;
}

int Tracks::zoom_t()
{
	view_start *= 2;
	mwindow->patches->zoom_in_t(view_start);
	
	int pixel;
	Track *current;
	
	for(pixel = 0, current = first; current; current = NEXT, pixel += mwindow->zoom_track)
	{
		current->pixel = pixel - view_start;
	}
	draw();

	mwindow->gui->trackscroll->update();
return 0;
}

int Tracks::expand_t()
{
	view_start /= 2;
	mwindow->patches->expand_t(view_start);
	
	int pixel;
	Track *current;
	
	for(pixel = 0, current = first; current; current = NEXT, pixel += mwindow->zoom_track)
	{
		current->pixel = pixel - view_start;
	}
	draw();
	
	mwindow->gui->trackscroll->update();
return 0;
}

int Tracks::redo_pixels()
{
	int current_pixel = -view_start;
	Track *current_track = first;
	
	for( ; current_track; current_track = current_track->next, current_pixel += mwindow->zoom_track)
	{
		current_track->pixel = current_pixel;
	}
	mwindow->gui->trackscroll->update();
return 0;
}
