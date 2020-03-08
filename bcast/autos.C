#include <string.h>
#include "autos.h"
#include "filehtal.h"


Autos::Autos(Track *track, 
			int color, 
			float default_, 
			int stack_number, 
			int stack_total)
 : List<Auto>()
{
	this->track = track;
	this->color = color;
	this->default_ = default_;
	selected = 0;
	this->stack_number = stack_number;
	this->stack_total = stack_total;
}


Autos::~Autos()
{
}

int Autos::create_objects()
{
	//add_auto(0, default_);
return 0;
}


int Autos::clear_all()
{
	Auto *current_, *current;
	
	for(current = first; current; current = current_)
	{
		current_ = NEXT;
		remove(current);
	}
	add_auto(0, default_);
return 0;
}

int Autos::insert(long start, long end)
{
	long length;
	Auto *current = autoof(start);
	length = end - start;
	
	for(; current; current = NEXT)
	{
		current->position += length;
	}
return 0;
}


int Autos::paste_silence(long start, long end)
{
	insert(start, end);
return 0;
}

int Autos::copy(long start, long end, FileHTAL *htal, int autos_follow_edits)
{
	if(autos_follow_edits)
	{
		for(Auto* current = autoof(start); current && current->position <= end; current = NEXT)
		{
			current->copy(start, end, htal);
		}
		return 0;
	}
	return 1;
return 0;
}

int Autos::paste(long start, long end, long total_length, FileHTAL *htal, char *end_string, int autos_follow_edits, int shift_autos)
{
	if(!autos_follow_edits) return 1;

	clear(start, end, 1, shift_autos);

	if(shift_autos) insert(start, start + total_length);

	int result = 0;
	long position;
	float value;
	Auto *current;
	
	do{
		result = htal->read_tag();

		if(!result)
		{
			if(!strcmp(htal->tag.get_title(), end_string))
			{
				result = 1;
			}
			else
			if(!strcmp(htal->tag.get_title(), "AUTO"))
			{
				paste_derived(htal, start);
			}
		}
	}while(!result);
return 0;
}

int Autos::paste_derived(FileHTAL *htal, long start)
{
	float value;
	long position;

	position = htal->tag.get_property("SAMPLE", (long)0);
	value = htal->tag.get_property("VALUE", (float)0);

	current = add_auto(position + start, value);
return 0;
}

int Autos::clear(long start, long end, int autos_follow_edits, int shift_autos)
{
	long length;
	Auto *current_, *current;
	length = end - start;
	
	current = autoof(start);
// Should ignore border autos.
	//if(current && current->position == start) current = NEXT;
	while(current && current->position < end)
	{
		current_ = NEXT;
		remove(current);
		current = current_;
	}
	while(current && shift_autos)
	{
		current->position -= length;
		current = NEXT;
	}
return 0;
}

int Autos::clear_auto(long position)
{
	Auto *current;
	current = autoof(position);
	if(current->position == position) remove(current);
return 0;
}

int Autos::save(FileHTAL *htal)
{
// swap modified auto for original auto's position if an auto is being modified
	swap_out_selected();
	
	for(Auto* current = first; current; current = NEXT)
	{
		if(!current->skip)
		{
			current->save(htal);
		}
	}

// swap original auto's position for modified auto
	swap_in_selected();
return 0;
}

int Autos::load(FileHTAL *htal, char *end_string)
{
	while(last)
		remove(last);    // remove any existing autos

	int result = 0;
	Auto *current;
	
	do{
		result = htal->read_tag();
		
		if(!result)
		{
			if(!strcmp(htal->tag.get_title(), end_string))
			{
				result = 1;
			}
			else
			if(!strcmp(htal->tag.get_title(), "AUTO"))
			{
				current = append_auto();
				current->load(htal);
			}
		}
	}while(!result);
return 0;
}






int Autos::slope_adjustment(long ax, float slope)
{
	return (int)(ax * slope);
return 0;
}

int Autos::draw(BC_Canvas *canvas, int pixel, int zoom_track, float units_per_pixel, float view_start, int vertical)
{
	int center_pixel;
	float view_end;
	long ax, ay, ax2, ay2;
	float yscale, slope;
	Auto* current;

	swap_out_selected();

	get_track_pixels(zoom_track, pixel, center_pixel, yscale);

	view_end = view_start + units_per_pixel * (vertical ? canvas->h : canvas->w);
	canvas->set_color(color);
	current = autoof(view_start);

// no autos so draw default
// get first points and point current to first auto
	if(!last && !first)
	{
		ax = 0;
		ay = (int)(default_ * yscale);
	}
	else
	if(current)
	{
		if(current->position > view_start)
		{
// inner auto
			if(PREVIOUS)
			{
				current = PREVIOUS;
				ax = (long)((current->position - view_start) / units_per_pixel);
				ay = (int)(current->value * yscale);

				current = NEXT;

				ax2 = (long)((current->position - view_start) / units_per_pixel);
				ay2 = (int)(current->value * yscale);

				slope = (float)(ay2 - ay) / (ax2 - ax);
				ay += slope_adjustment(-ax, slope);
				ax = 0;
			}
			else
// no previous auto
			{
				ax = 0;
				ay = (int)(current->value * yscale);

				ax2 = (long)((current->position - view_start) / units_per_pixel);
				ay2 = ay;
			} 
		}
// first auto is on first pixel
		else
		{
			ax = 0;
			ay = (int)(current->value * yscale);

//printf("Autos::draw %d %d\n", ax, ay);
			current->draw(canvas, ax, ay, center_pixel, zoom_track, vertical, 0);

			current = NEXT;
			
			if(current)
			{
				ax2 = (long)((current->position - view_start) / units_per_pixel);
				ay2 = (int)(current->value * yscale);
			}
		}
	}
// beyond last auto
	else
	{
		ax = 0;
		ay = (int)(last->value * yscale);
	}
	
	while(current && current->position <= view_end)
	{
		draw_joining_line(canvas, vertical, center_pixel, ax, ay, ax2, ay2);
		
		current->draw(canvas, ax2, ay2, center_pixel, zoom_track, vertical, 0);
		ax = ax2;
		ay = ay2;

		current = NEXT;
		if(current)
		{
			ax2 = (long)((current->position - view_start) / units_per_pixel);
			ay2 = (int)(current->value * yscale);
		}
	}

	if(current)
	{
		slope = (float)(ay2 - ay) / (ax2 - ax);
		ay2 -= slope_adjustment((ax2 - (vertical ? canvas->h : canvas->w)), slope);
		ax2 = (vertical ? canvas->h : canvas->w);
	}
	else
	{
		ax2 = (vertical ? canvas->h : canvas->w);
		ay2 = ay;
	}
	
	draw_joining_line(canvas, vertical, center_pixel, ax, ay, ax2, ay2);

	
	
	swap_in_selected();	
return 0;
}

int Autos::swap_out_selected()
{
	if(selected)
	{
		selected_position_ = selected->position;
		selected_value_ = selected->value;
		selected->position = selected_position;
		selected->value = selected_value;
	}
return 0;
}

int Autos::swap_in_selected()
{
	if(selected)
	{
		selected->position = selected_position_;
		selected->value = selected_value_;
	}
return 0;
}

int Autos::draw_floating_autos(BC_Canvas *canvas, int pixel, int zoom_track, float units_per_pixel, float view_start, int vertical, int flash)
{
	int result;
	Auto* current;
	
	for(result = 0, current = first; current && !result; current = NEXT)
	{
		if(selected == current) 
		{
			result = 1;
		}
	}
	if(result) draw_floating(canvas, pixel, zoom_track, units_per_pixel, view_start, vertical, flash);
return 0;
}

int Autos::draw_floating(BC_Canvas *canvas, int pixel, int zoom_track, float units_per_pixel, float view_start, int vertical, int flash)
{
	int center_pixel;
	float view_end;
	long ax1, ay1, ax2, ay2, ax3, ay3;
	float yscale, slope;
	int skip;
	
	skip = selected->skip;
	selected->skip = 0;
	view_end = view_start + units_per_pixel * (vertical ? canvas->h : canvas->w);
	get_track_pixels(zoom_track, pixel, center_pixel, yscale);

	ax2 = (long)((selected->position - view_start) / units_per_pixel);
	ay2 = (int)(selected->value * yscale);

	if(selected->previous)
	{
		ax1 = (long)((selected->previous->position - view_start) / units_per_pixel);
		ay1 = (int)(selected->previous->value * yscale);
		
		if(ax1 < 0)
		{
				slope = (float)(ay2 - ay1) / (ax2 - ax1);
				ay1 -= slope_adjustment(ax1, slope);
				ax1 = 0;
		}
	}
	else
	{
		ax1 = 0;
		ay1 = ay2;
	}
	
	if(selected->next)
	{
		ax3 = (long)((selected->next->position - view_start) / units_per_pixel);
		ay3 = (int)(selected->next->value * yscale);
		
		if(ax3 > (vertical ? canvas->h : canvas->w))
		{
				slope = (float)(ay3 - ay2) / (ax3 - ax2);
				ay3 -= slope_adjustment((ax3 - (vertical ? canvas->h : canvas->w)), slope);
				ax3 = (vertical ? canvas->h : canvas->w);
		}
	}
	else
	{
		ax3 = (vertical ? canvas->h : canvas->w);
		ay3 = ay2;
	}

	canvas->set_inverse();
	canvas->set_color(WHITE);

	draw_joining_line(canvas, vertical, center_pixel, ax1, ay1, ax2, ay2);

	selected->draw(canvas, ax2, ay2, center_pixel, zoom_track, vertical, 1);
	
	draw_joining_line(canvas, vertical, center_pixel, ax2, ay2, ax3, ay3);
	
	canvas->set_opaque();
	selected->skip = skip;
	
	if(flash) 
	{
		if(vertical)
		canvas->flash(pixel, 0, zoom_track, canvas->h);
		else
		canvas->flash(0, pixel, canvas->w, zoom_track);
	}
return 0;
}

int Autos::select_auto(BC_Canvas *canvas, int pixel, int zoom_track, float units_per_pixel, float view_start, int cursor_x, int cursor_y, int vertical)
{
	int result;
	long position;
	long ax, ay, ax2, ay2, miny, maxy, test_y;
	float yscale, slope, init_value;
	int center_pixel;
	Auto* current;

	result = 0;
	position = (long)(view_start + units_per_pixel * cursor_x);

	get_track_pixels(zoom_track, pixel, center_pixel, yscale);
	cursor_y -= center_pixel;
// =================================== test existing autos

	for(current = first; current && !result; current = NEXT)
	{
		ax = (long)((current->position - view_start) / units_per_pixel);
		ay = (long)(current->value * yscale);

		if(current->selected(ax, vertical ? -ay : ay, cursor_x, cursor_y, pixel, zoom_track))
		{
			result = 1;
			selected = current;
			selected_position = current->position;
			selected_value = current->value;
			selected->skip = 0;
			draw_floating(canvas, pixel, zoom_track, units_per_pixel, view_start, vertical, 1);
		}
	}
	current = autoof(position);

// ================================= test lines
// get points of nearest line

	if(!last && !first && !result)
	{
		ax = 0;
		ay = (int)(default_ * yscale);
		ax2 = ax;
		ay2 = ay;
		slope = 0;
		init_value = default_;
	}
	else
	if(current && !result)
	{
		if(current->position > view_start)
		{
// inner auto
			if(PREVIOUS)
			{
				current = PREVIOUS;
				ax = (long)((current->position - view_start) / units_per_pixel);
				ay = (int)(current->value * yscale);
				init_value = current->value;
				current = NEXT;
				
				ax2 = (long)((current->position - view_start) / units_per_pixel);
				ay2 = (int)(current->value * yscale);
				
				slope = (float)(ay2 - ay) / (ax2 - ax);
			}
			else
// no previous auto
			{
				ax = 0;
				ay = (int)(current->value * yscale);
				init_value = current->value;
				
				ax2 = (long)((current->position - view_start) / units_per_pixel);
				ay2 = ay;
				slope = 0;
			} 
		}
// current auto is on first pixel
		else
		{
			ax = 0;
			ay = (int)(current->value * yscale);
			init_value = current->value;
			ax2 = vertical ? canvas->h : canvas->w;
			ay2 = ay;
			slope = 0;
		}
	}
// beyond last auto
	else
	if(!result)
	{
		ax = 0;
		ay = (int)(last->value * yscale);
		init_value = last->value;
		ax2 = vertical ? canvas->h : canvas->w;
		ay2 = ay;
		slope = 0;
	}

	if(!result)
	{
		test_y = get_testy(slope, cursor_x, ax, ay);

		if(vertical) 
		{ miny = maxy = test_y * -1; }
		else
		{ miny = maxy = test_y; }
		
		miny -= 5; maxy += 5;

		if(cursor_y > miny && cursor_y < maxy)
		{
			selected = add_auto(position, (float)test_y / yscale);
			selected_position = (long)position;
			selected_value = selected->value;
			selected->skip = 1;
			draw_floating(canvas, pixel, zoom_track, units_per_pixel, view_start, vertical, 1);
			result = 1;
		}
	}


	if(result)
	{
		if(vertical)
		virtual_center = center_pixel + cursor_y - (int)(selected_value * virtual_h / (max - min));
		else
		virtual_center = (int)(selected_value * virtual_h / (max - min) + cursor_y + center_pixel);
	}
	return result;
return 0;
}

int Autos::move_auto(BC_Canvas *canvas, int pixel, int zoom_track, float units_per_pixel, float view_start, int cursor_x, int cursor_y, int shift_down, int vertical)
{
	if(selected)
	{
		long position;
		float value, yscale;
		int value_int;
		Auto *current = selected;

// hide the old auto
		//draw_floating(canvas, pixel, zoom_track, units_per_pixel, view_start, vertical, 0);

		position = (long)(units_per_pixel  * cursor_x + view_start);
		if(position < 0) position = 0;

		if(!shift_down)
		{
			yscale = (float)virtual_h / (max - min);
			if(!vertical) yscale *= -1;
			
			value = cursor_y - virtual_center; value /= yscale;

// fix precision
			value = fix_value(value);
		}
		else
// ================================= shift down
		{
			if(PREVIOUS)
			{
				if(PREVIOUS->value != 0 && PREVIOUS->value != min && PREVIOUS->value != max)
				{
					value = PREVIOUS->value;
				}
				else
				{
					if(NEXT && NEXT->value != min && NEXT->value != max)
					{
						value = NEXT->value;
					}
					else
					{
						value = 0;
					}
				}
			}
			else
			if(NEXT)
			{
				if(NEXT->value != min && NEXT->value != max)
				{
					value = NEXT->value;
				}
				else
				{
					value = 0;
				}
			}
			else
			{
				value = 0;
			}
		}

		current->position = position;
		current->value = value;

// show autos
		//draw_floating(canvas, pixel, zoom_track, units_per_pixel, view_start, vertical, 1);
	}
	
	return 0;
return 0;
}


int Autos::release_auto()
{
	int result = 0, deleted = 0;
	
//printf("Autos::release_auto %x\n", selected);
	if(selected)
	{
		result = 1;
		deleted = 0;
		selected->skip = 0;

// delete if necessary
		if(selected->previous)
		{
			if(selected->previous->position >= selected->position) 
			{
				remove(selected);
				deleted = 1;
			}
		}

//printf("Autos::release_auto 2\n");
		if(!deleted && selected->next)
		{
			if(selected->next->position <= selected->position) 
			{
				remove(selected);
				deleted = 1;
			}
		}

//printf("Autos::release_auto 3\n");
		// need first auto to get started
		//if(first == last != 0)
		//{
		//	default_ = first->value;
		//	remove(first);
		//}
		
		release_auto_derived();
		selected = 0;
	}
	return result;
return 0;
}

int Autos::scale_time(float rate_scale, int scale_edits, int scale_autos, long start, long end)
{
	Auto *current;
	
	for(current = first; current && scale_autos; current = NEXT)
	{
//		if(current->position >= start && current->position <= end)
//		{
			current->position = (long)((current->position - start) * rate_scale + start + 0.5);
//		}
	}
return 0;
}

Auto* Autos::autoof(long position)
{
	Auto *current;

	for(current = first; current && current->position < position; current = NEXT)
	{ ; }
	return current;     // return 0 on failure
}

Auto* Autos::nearest_before(long position)
{
	Auto *current;

	for(current = last; current && current->position >= position; current = PREVIOUS)
	{ ; }


	return current;     // return 0 on failure
}

Auto* Autos::nearest_after(long position)
{
	Auto *current;

	for(current = first; current && current->position <= position; current = NEXT)
	{ ; }


	return current;     // return 0 on failure
}

int Autos::get_neighbors(long start, long end, Auto **before, Auto **after)
{
	if(*before == 0) *before = first;
	if(*after == 0) *after = last; 

	while(*before && (*before)->next && (*before)->next->position <= start)
		*before = (*before)->next;
	
	while(*after && (*after)->previous && (*after)->previous->position >= end)
		*after = (*after)->previous;

	while(*before && (*before)->position > start) *before = (*before)->previous;
	
	while(*after && (*after)->position < end) *after = (*after)->next;
return 0;
}

int Autos::automation_is_constant(long start, long end, Auto **before, Auto **after)
{
	Auto *current_auto;
	int result;

	result = 1;          // default to constant
	if(!last && !first) return result; // no automation at all

// quickly get autos just outside range	
	get_neighbors(start, end, before, after);

// autos before range
	if(*before) 
	current_auto = *before;   // try first auto
	else 
	current_auto = first;

// test autos in range	
	for(;result && current_auto && current_auto->next && current_auto->position < end; current_auto = current_auto->next)
	{
		if(current_auto->next->value != current_auto->value) result = 0;   // not constant
	}

	return result;
return 0;
}

float Autos::get_automation_constant(long start, long end, Auto **before, Auto **after)
{
	Auto *current_auto;
	
// quickly get autos just outside range	
	get_neighbors(start, end, before, after);

// no auto before range so use first
	if(*before)
	current_auto = *before;
	else
	current_auto = first;

// no autos at all so use default value
	if(!current_auto) return default_;

	return current_auto->value;
}


int Autos::init_automation(long &buffer_position,
				long &input_start, 
				long &input_end, 
				int &automate, 
				float &constant, 
				long input_position,
				long buffer_len,
				Auto **before, 
				Auto **after,
				int reverse)
{
	buffer_position = 0;

// set start and end boundaries for automation info
	input_start = reverse ? input_position - buffer_len : input_position;
	input_end = reverse ? input_position : input_position + buffer_len;

// test automation for constant value
// and set up *before and *after
	if(automate)
	{
		if(automation_is_constant(input_start, input_end, before, after))
		{
			constant += get_automation_constant(input_start, input_end, before, after);
			automate = 0;
		}
	}
	return automate;
return 0;
}


int Autos::init_slope(Auto **current_auto, 
				float &slope_start, 
				float &slope_value,
				float &slope_position, 
				long &input_start, 
				long &input_end, 
				Auto **before, 
				Auto **after,
				int reverse)
{
// apply automation
	*current_auto = reverse ? *after : *before;
// no auto before start so use first auto in range
// already know there is an auto since automation isn't constant
	if(!*current_auto)
	{
		*current_auto = reverse ? last : first;
		slope_value = (*current_auto)->value;
		slope_start = input_start;
		slope_position = 0;
	}
	else
	{
// otherwise get the first slope point and advance auto
		slope_value = (*current_auto)->value;
		slope_start = (*current_auto)->position;
		slope_position = reverse ? slope_start - input_end : input_start - slope_start;
		(*current_auto) = reverse ? (*current_auto)->previous : (*current_auto)->next;
	}
return 0;
}


int Autos::get_slope(Auto **current_auto, 
				float &slope_start, 
				float &slope_end, 
				float &slope_value,
				float &slope, 
				long buffer_len, 
				long buffer_position,
				int reverse)
{
// get the slope
	if(*current_auto)
	{
		slope_end = reverse ? slope_start - (*current_auto)->position : (*current_auto)->position - slope_start;
		if(slope_end) 
			slope = ((*current_auto)->value - slope_value) / slope_end;
		else
			slope = 0;
	}
	else
	{
		slope = 0;
		slope_end = buffer_len - buffer_position;
	}
return 0;
}

int Autos::advance_slope(Auto **current_auto, 
				float &slope_start, 
				float &slope_value,
				float &slope_position, 
				int reverse)
{
	if(*current_auto) 
	{
		slope_start = (*current_auto)->position;
		slope_value = (*current_auto)->value;
		(*current_auto) = reverse ? (*current_auto)->previous : (*current_auto)->next;
		slope_position = 0;
	}
return 0;
}
