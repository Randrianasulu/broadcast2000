#include <string.h>
#include "bezierauto.h"
#include "bezierautos.h"
#include "filehtal.h"
#include "mainwindow.h"


BezierAutos::BezierAutos(MainWindow *mwindow, 
					Track *track, 
					int color, 
					float center_x, 
					float center_y, 
					float center_z, 
					float frame_w,
					float frame_h,
					float virtual_w,
					float virtual_h)
 : Autos(track, color, 0)
{
	this->mwindow = mwindow;
	this->center_x = center_x;
	this->center_y = center_y;
	this->center_z = center_z;
	this->frame_w = frame_w;
	this->frame_h = frame_h;

	virtual_center_x = 0;
	virtual_center_y = 0;
	this->virtual_w = virtual_w;
	this->virtual_h = virtual_h;
	selection_type = 0;
	old_selected = new BezierAuto(0);
	new_selected = new BezierAuto(0);
}

BezierAutos::~BezierAutos()
{
	delete old_selected;
	delete new_selected;
}

int BezierAutos::paste_derived(FileHTAL *htal, long start)
{
	long frame = htal->tag.get_property("FRAME", 0);
	BezierAuto* current = (BezierAuto*)add_auto(frame + start, 0, 0, 1);
	current->load(htal);
	current->position += start;   // fix the position loaded by load()
return 0;
}

int BezierAutos::draw(BC_Canvas *canvas, 
				int pixel, 
				int zoom_track, 
				float units_per_pixel, 
				float view_start, 
				int vertical)
{
	float center_pixel;
	BezierAuto *current, *before = 0, *after = 0;
	float view_end;
	float scale;
	float frame;
	float x1, y1, x2, y2;
	int x;

	x1 = y1 = x2 = y2 = 0;
	swap_out_selected();

	center_pixel = pixel + zoom_track / 2;
	view_end = view_start + units_per_pixel * (vertical ? canvas->h : canvas->w);
	scale = (float)zoom_track / (vertical ? frame_w : frame_h);
	canvas->set_color(color);

// Can't see the automation at all if opaque.
// Also fix tracks->hide_overlays if you change this.
	canvas->set_inverse();

	if(vertical)
		canvas->draw_line(center_pixel, 0, center_pixel, canvas->h);
	else
		canvas->draw_line(0, center_pixel, canvas->w, center_pixel);

// skip drawing line for now
// // get the first pixel
// 	x = 1;
// 	frame = view_start;
// 
// 	get_center(x1, y1, frame, 0, &before, &after);
// 	if(vertical)
// 	{
// 		x1 = x1 * scale + center_pixel;
// 	}
// 	else
// 	{
// 		y1 = y1 * scale + center_pixel;
// 	}
// 
// //printf("BezierAutos::draw 2\n");
// // draw the remaining pixels
// 	for(frame += units_per_pixel; 
// 		frame < view_end; 
// 		frame += units_per_pixel, x++)
// 	{
// 		get_center(x2, y2, frame, 0, &before, &after);
// 
// 		if(vertical)
// 		{
// 			x2 = (int)(x2 * scale + center_pixel);
// 			canvas->draw_line(x1, x - 1, x2, x);
// 		}
// 		else
// 		{
// 			y2 = (int)(y2 * scale + center_pixel);
// 			canvas->draw_line(x - 1, y1, x, y2);
// 		}
// 		
// 		x1 = x2;
// 		y1 = y2;
// 	}
//printf("BezierAutos::draw 3\n");

// draw the autos
	current = (BezierAuto*)autoof(view_start);
	while(current && current->position <= view_end)
	{
		if(!current->skip)
		{
			x = get_auto_pixel(current->position, view_start, units_per_pixel, get_frame_half(scale, vertical, units_per_pixel));

			current->draw(canvas, x, center_pixel, scale, vertical, 0);
		}
		current = (BezierAuto*)NEXT;
	}
	canvas->set_opaque();

	swap_in_selected();
return 0;
}

int BezierAutos::get_auto_pixel(long position, float view_start, float units_per_pixel, int frame_half)
{
	int result;

	result = (int)((position - view_start) / units_per_pixel) + frame_half;

	return result;
return 0;
}

long BezierAutos::get_auto_frame(int position, float view_start, float units_per_pixel, int frame_half)
{
	long result;

	result = (long)((position) * units_per_pixel + view_start);

	return result;
}

int BezierAutos::get_frame_half(float scale, int vertical, float units_per_pixel)
{
	int result = (int)(.5 * scale * (vertical ? frame_h : frame_w));
	if(1 / units_per_pixel / 2 < result) result = (int)(1 / units_per_pixel / 2);
	return result;
return 0;
}

int BezierAutos::get_center(float &x, float &y, float &z, 
			float frame, 
			int reverse,
			BezierAuto **before, BezierAuto **after)
{
	get_neighbors(frame, frame, (Auto**)before, (Auto**)after);

	if(*before == 0 && *after == 0)
	{
		x = center_x;
		y = center_y;
		z = center_z;
		return 0;
	}

	float x0=0, x1=0, x2=0, x3=0;
	float y0=0, y1=0, y2=0, y3=0;
	float z0=0, z1=0, z2=0, z3=0;
	float frame0=0, frame1=0;

	if(*before)
	{
		x0 = (*before)->center_x;
		y0 = (*before)->center_y;
		z0 = (*before)->center_z;
		frame0 = (float)(*before)->position;

		if(*after == 0 || frame == frame0)
		{
			x = x0;
			y = y0;
			z = z0;
			return 0;
		}

		x1 = (*before)->control_out_x + x0;
		y1 = (*before)->control_out_y + y0;
		z1 = (*before)->control_out_z + z0;
	}

	if(*after)
	{
		x3 = (*after)->center_x;
		y3 = (*after)->center_y;
		z3 = (*after)->center_z;
		frame1 = (float)(*after)->position;
	
		if(*before == 0 || frame == frame1)
		{
			x = x3;
			y = y3;
			z = z3;
			return 0;
		}

		x2 = (*after)->control_in_x + x3;
		y2 = (*after)->control_in_y + y3;
		z2 = (*after)->control_in_z + z3;
	}

 	float t = (frame - frame0) / (frame1 - frame0);
 	float tpow2 = t * t;
	float tpow3 = t * t * t;
	float invt = 1 - t;
	float invtpow2 = invt * invt;
	float invtpow3 = invt * invt * invt;

	if(frame_w) x = (        invtpow3 * x0
		+ 3 * t     * invtpow2 * x1
		+ 3 * tpow2 * invt     * x2 
		+     tpow3            * x3);

	y = (        invtpow3 * y0 
		+ 3 * t     * invtpow2 * y1
		+ 3 * tpow2 * invt     * y2 
		+     tpow3            * y3);

	z = (        invtpow3 * z0 
		+ 3 * t     * invtpow2 * z1
		+ 3 * tpow2 * invt     * z2 
		+     tpow3            * z3);
	return 0;
return 0;
}

int BezierAutos::swap_out_selected()
{
	if(selected)
	{
		BezierAuto *bezier_selected = (BezierAuto*)selected;
		new_selected->position = bezier_selected->position;
		new_selected->center_x = bezier_selected->center_x;
		new_selected->control_in_x = bezier_selected->control_in_x;
		new_selected->control_out_x = bezier_selected->control_out_x;
		new_selected->center_y = bezier_selected->center_y;
		new_selected->control_in_y = bezier_selected->control_in_y;
		new_selected->control_out_y = bezier_selected->control_out_y;
		new_selected->center_z = bezier_selected->center_z;
		new_selected->control_in_z = bezier_selected->control_in_z;
		new_selected->control_out_z = bezier_selected->control_out_z;

		bezier_selected->position = old_selected->position;
		bezier_selected->center_x = old_selected->center_x;
		bezier_selected->control_in_x = old_selected->control_in_x;
		bezier_selected->control_out_x = old_selected->control_out_x;
		bezier_selected->center_y = old_selected->center_y;
		bezier_selected->control_in_y = old_selected->control_in_y;
		bezier_selected->control_out_y = old_selected->control_out_y;
		bezier_selected->center_z = old_selected->center_z;
		bezier_selected->control_in_z = old_selected->control_in_z;
		bezier_selected->control_out_z = old_selected->control_out_z;
	}
return 0;
}

int BezierAutos::swap_in_selected()
{
	if(selected)
	{
		BezierAuto *bezier_selected = (BezierAuto*)selected;
		bezier_selected->position = new_selected->position;
		bezier_selected->center_x = new_selected->center_x;
		bezier_selected->control_in_x = new_selected->control_in_x;
		bezier_selected->control_out_x = new_selected->control_out_x;
		bezier_selected->center_y = new_selected->center_y;
		bezier_selected->control_in_y = new_selected->control_in_y;
		bezier_selected->control_out_y = new_selected->control_out_y;
		bezier_selected->center_z = new_selected->center_z;
		bezier_selected->control_in_z = new_selected->control_in_z;
		bezier_selected->control_out_z = new_selected->control_out_z;
	}
return 0;
}

int BezierAutos::draw_floating_autos(BC_Canvas *canvas, 
						int pixel, 
						int zoom_track, 
						float units_per_pixel, 
						float view_start, 
						int vertical, 
						int flash)
{
	if(selected) 
	{
		BezierAuto *before = 0, *after = 0;
		float frame1, frame2, frame;
		int center_pixel;
		float view_end;
		float scale;
		int x, x1, y1, x2, y2;
		int skip;

		skip = selected->skip;
		selected->skip = 0;

		center_pixel = pixel + zoom_track / 2;
		view_end = view_start + units_per_pixel * (vertical ? canvas->h : canvas->w);
		scale = (float)zoom_track / (vertical ? frame_w : frame_h);

		before = (BezierAuto*)selected->previous;
		after = (BezierAuto*)selected->next;
		
		frame1 = before ? before->position : view_start;
		frame2 = after ? after->position : view_end;
		if(frame1 < view_start) frame1 = view_start;
		if(frame2 > view_end) frame2 = view_end;
		x = get_auto_pixel(frame1, view_start, units_per_pixel, get_frame_half(scale, vertical, units_per_pixel));
		before = 0;
		after = 0;

		canvas->set_inverse();
		canvas->set_color(WHITE);

// skip drawing line for now
// 		get_center(x1, y1, z1, frame1 - units_per_pixel, 0, &before, &after);
// 		if(vertical)
// 		{
// 			x1 = (int)(x1 * scale + center_pixel);
// 		}
// 		else
// 		{
// 			y1 = (int)(y1 * scale + center_pixel);
// 		}
// 		
// 		for(frame = frame1; frame < frame2; frame += units_per_pixel, x++)
// 		{
// 			get_center(x2, y2, z2, frame, 0, &before, &after);
// 
// 			if(vertical)
// 			{
// 				x2 = (int)(x2 * scale + center_pixel);
// 				canvas->draw_line(x1, x - 1, x2, x);
// 			}
// 			else
// 			{
// 				y2 = (int)(y2 * scale + center_pixel);
// 				canvas->draw_line(y1, x - 1, y2, x);
// 			}
// 
// 			x1 = x2;
// 			y1 = y2;
// 		}

		x = get_auto_pixel(selected->position, view_start, units_per_pixel, get_frame_half(scale, vertical, units_per_pixel));
		
		((BezierAuto*)selected)->draw(canvas, x, center_pixel, scale, vertical, 1);
	
		canvas->set_opaque();		
		selected->skip = skip;
	
		if(flash) 
		{
			if(vertical)
			canvas->flash(pixel, 0, zoom_track, canvas->h);
			else
			canvas->flash(0, pixel, canvas->w, zoom_track);
		}
	}
return 0;
}

int BezierAutos::select_auto(BC_Canvas *canvas, 
						int pixel, 
						int zoom_track, 
						float units_per_pixel, 
						float view_start, 
						int cursor_x, 
						int cursor_y, 
						int shift_down,
						int ctrl_down,
						int mouse_button,
						int vertical)
{
// cursor_x is relative to frame number
	BezierAuto *before = 0, *after = 0;
	int center_pixel;
	float view_end;
	float scale;
	float frame;
	float x, y, miny, maxy;

	selection_type = 0;
	center_pixel = pixel + zoom_track / 2;
	view_end = view_start + units_per_pixel * (vertical ? canvas->h : canvas->w);
	scale = (float)zoom_track / (vertical ? frame_w : frame_h);

	frame = get_auto_frame(cursor_x, view_start, units_per_pixel, vertical);

// test autos for selection
	current = (BezierAuto*)autoof(view_start);
	while(current && current->position <= view_end && !selection_type)
	{
		x = get_auto_pixel(current->position, view_start, units_per_pixel, get_frame_half(scale, vertical, units_per_pixel));

		selection_type = ((BezierAuto*)current)->select(canvas, 
										x, 
										center_pixel, 
										scale, 
										cursor_x, 
										cursor_y, 
										shift_down, 
										ctrl_down, 
										mouse_button, 
										vertical);

		if(selection_type)
		{
			selected = current;
			current->skip = 1;
		}
		current = (BezierAuto*)NEXT;
	}

// test line for selection
	if(!selection_type && !shift_down)
	{
// don't use auto line for now
//		get_center(x, y, frame, 0, &before, &after);
//
//		if(vertical) y = x;
//
//		miny = (int)(y * scale + center_pixel - 5);
//		maxy = miny + 10;

		miny = center_pixel - 5;
		maxy = center_pixel + 5;

		if(cursor_y > miny && cursor_y < maxy)
		{
			selected = add_auto(frame, center_x, center_y, center_z);

// copy values from neighbor if possible
			BezierAuto *neighbor = 0, *bezier_selected = (BezierAuto*)selected;
			if(selected->previous)
			{
				neighbor = (BezierAuto*)selected->previous;
			}
			else
			if(selected->next)
			{
				neighbor = (BezierAuto*)selected->next;
			}
			
			if(neighbor)
			{
// center point should be copied.
// Control points should be zero.
				bezier_selected->center_x = neighbor->center_x;
				bezier_selected->center_y = neighbor->center_y;
				bezier_selected->center_z = neighbor->center_z;
// 				bezier_selected->control_in_x = neighbor->control_in_x;
// 				bezier_selected->control_in_y = neighbor->control_in_y;
// 				bezier_selected->control_in_z = neighbor->control_in_z;
// 				bezier_selected->control_out_x = neighbor->control_out_x;
// 				bezier_selected->control_out_y = neighbor->control_out_y;
// 				bezier_selected->control_out_z = neighbor->control_out_z;
			}

// default to sample selection
			selection_type = 1;
			selected->skip = 1;
		}
	}

	if(selection_type)
	{
		draw_floating_autos(canvas, 
						pixel, 
						zoom_track, 
						units_per_pixel, 
						view_start, 
						vertical, 
						1);

		get_virtual_center(virtual_center_x, virtual_center_y, cursor_x, cursor_y, vertical, scale);
	}

	return selection_type;
return 0;
}

int BezierAutos::get_virtual_center(float &x, float &y, int cursor_x, int cursor_y, int vertical, float scale)
{
// get virtual center relative to track canvas
	if(selected)
	{
		BezierAuto *current = (BezierAuto*)selected;
		float selected_x = 0;
		float selected_y = 0;
		switch(selection_type)
		{
			case 2:
				selected_x = current->center_x;
				selected_y = current->center_y;
				break;
			case 3:
				selected_x = 0;
				selected_y = current->center_z * virtual_h;
				break;
			case 4:
				selected_x = current->control_in_x;
				selected_y = current->control_in_y;
				break;
			case 5:
				selected_x = current->control_out_x;
				selected_y = current->control_out_y;
				break;
			case 6:
				selected_x = 0;
				selected_y = current->control_in_z * virtual_h;
				break;
			case 7:
				selected_x = 0;
				selected_y = current->control_out_z * virtual_h;
				break;
			default:
				selected_x = 0;
				selected_y = 0;
				break;
		}

// control points are independant of vertical tracks
		if(vertical)
		{
			cursor_x ^= cursor_y;
			cursor_y ^= cursor_x;
			cursor_x ^= cursor_y;
		}

		if(frame_w) 
			x = center_x + cursor_x - selected_x;
		else 
			x = cursor_x;

		y = center_y + cursor_y - selected_y;
	}
return 0;
}

int BezierAutos::move_auto(BC_Canvas *canvas, 
					int pixel, 
					int zoom_track, 
					float units_per_pixel, 
					float view_start, 
					int cursor_x, 
					int cursor_y, 
					int shift_down, 
					int vertical)
{
	int result = 0;
	if(selected)
	{
		BezierAuto* current = (BezierAuto*)selected;
		long position;
		float scale = (float)zoom_track / (vertical ? frame_w : frame_h);
// Frame auto is on.
		position = get_auto_frame(cursor_x, view_start, units_per_pixel, vertical);
		if(position < 0) position = 0;

		if(selection_type == 1)
		{
// move frame
			if(position != current->position) result = 1;
			current->position = position;
		}
		else
		{
// move control
// relative to frame
			float real_x;
			if(frame_w) 
				real_x = (float)((vertical ? cursor_y : cursor_x) - virtual_center_x);
			else
				real_x = 0;
			
			float real_y = (float)((vertical ? cursor_x : cursor_y) - virtual_center_y);

			switch(selection_type)
			{
				case 2:
					current->center_x = real_x;
					current->center_y = real_y;
					break;
				case 3:
					current->center_z = real_y / virtual_h;
					break;
				case 4:
					current->control_in_x = real_x;
					current->control_in_y = real_y;
					break;
				case 5:
					current->control_out_x = real_x;
					current->control_out_y = real_y;
					break;
				case 6:
					current->control_in_z = real_y / virtual_h;
					break;
				case 7:
					current->control_out_z = real_y / virtual_h;
					break;
			}
			result = 1;
		}
	}
	return result;
return 0;
}

int BezierAutos::release_auto_derived()
{
	selection_type = 0;
return 0;
}


Auto* BezierAutos::add_auto(long frame, float x, float y, float z)
{
	BezierAuto* current = (BezierAuto*)autoof(frame);
	BezierAuto* new_auto;
	
	insert_before(current, new_auto = new BezierAuto(this));
	new_auto->position = frame;
	new_auto->control_in_x = new_auto->control_out_x = 0;
	new_auto->control_in_y = new_auto->control_out_y = 0;
	new_auto->control_in_z = new_auto->control_out_z = 0;
	new_auto->center_x = x;
	new_auto->center_y = y;
	new_auto->center_z = z;

//printf("BezierAutos::add_auto %ld\n", new_auto->position);
	return (Auto*)new_auto;
}

Auto* BezierAutos::append_auto()
{
	return append(new BezierAuto(this));
}

int BezierAutos::scale_video(float scale, int *offsets)
{
	for(BezierAuto *current = (BezierAuto*)first; 
	current; 
	current = (BezierAuto*)NEXT)
	{
		current->center_x -= offsets[0];
		current->center_y -= offsets[1];
		current->center_z *= scale;
	}
	return 0;
return 0;
}
