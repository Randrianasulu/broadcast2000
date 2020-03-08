#include <string.h>
#include "bcfont.h"
#include "bckeys.h"
#include "bcsliders.h"
#include "bcresources.h"
#include "bcwindow.h"

int BC_Slider_Base::hs = 15;

BC_Slider_Base::BC_Slider_Base(int x, 
	int y, 
	int w, 
	int h, 
	int virtual_pixels, 
	int ltface, 
	int dkface, 
	int fader,
	int caption)
	: BC_Tool(x, y, w, h)
{
	buttondown = 0;
	highlighted = 0;
	this->virtual_pixels = virtual_pixels;
	this->dkface = dkface; this->ltface = ltface;
	this->fader = fader;
	this->caption = caption;
}

BC_Slider_Base::BC_Slider_Base(int x, 
	int y, 
	int w, 
	int h, 
	int virtual_pixels, 
	int fader,
	int caption)
	: BC_Tool(x, y, w, h)
{
	buttondown = 0;
	highlighted = 0;
	this->virtual_pixels = virtual_pixels;
	this->dkface = -1; this->ltface = -1;
	this->fader = fader;
	this->caption = caption;
}

int BC_Slider_Base::change_backcolor(int newcolor)
{
	backcolor = newcolor;
return 0;
}

int BC_Slider_Base::resize_tool(int x, int y, int w, int h, int virtual_pixels)
{
	if(virtual_pixels != -1) this->virtual_pixels = virtual_pixels;
	resize_window(x, y, w, h);
	update_();	
return 0;
}


int BC_Slider_Base::update_()
{
	if(w > h)
	{
// horizontal
		int hh;    /* handle dimensions */
		int x1, y1;  /* position of handle */
		int y2, hb;  // position of bar

		x1 = position - hs / 2;
		y1 = 2;
		hh = h - 4;

		if(fader)
		{ y2 = h / 2 - 4; hb = 8; } 
		else
		{ y2 = 0; hb = h; }

// clear box
		set_color(backcolor);
		draw_box(0, 0, w, h);

// slider box
		draw_3d_big(0, y2, w, hb, 
			top_level->get_resources()->button_shadow, 
			BLACK, 
			top_level->get_resources()->button_down, 
			top_level->get_resources()->button_down,
			top_level->get_resources()->button_light);

		if(highlighted)
			draw_3d_big(x1, y1, hs, hh, 
				top_level->get_resources()->button_light, 
				ltface, 
				ltface, 
				top_level->get_resources()->button_shadow,
				BLACK);
		else
			draw_3d_big(x1, y1, hs, hh, 
				top_level->get_resources()->button_light, 
				dkface, 
				dkface, 
				top_level->get_resources()->button_shadow,
				BLACK);

		set_color(BLACK);
		draw_line(x1 + hs / 2, y1, x1 + hs / 2, y1 + hh - 1);

		if(caption)
		{
			set_font(SMALLFONT);
			if(fader)
			{
				set_color(RED);
  				draw_center_text(w / 2, y2, text, SMALLFONT);
			}
			else
			{
				set_color(MEYELLOW);
  				draw_center_text(w / 2, y2 + hb - 2, text, SMALLFONT);
			}
			set_font(MEDIUMFONT);
		}
	}
	else
	{
// vertical
		int hw;    // handle dimensions
		int x1, y1;  // position of handle 
		int x2, wb;  // position of bar

		x1 = 2;
		y1 = h - position - hs / 2;
		hw = w - 4;

		if(fader)
		{ x2 = w / 2 - 4; wb = 8; } 
		else
		{ x2 = 0; wb = w; }

// clear box
		set_color(backcolor);
		draw_box(0, 0, w, h);


// slider box
		draw_3d_big(x2, 0, wb, h, 
			top_level->get_resources()->button_shadow, 
			BLACK, 
			top_level->get_resources()->button_down, 
			top_level->get_resources()->button_down,
			top_level->get_resources()->button_light);

		if(highlighted)
			draw_3d_big(x1, y1, hw, hs, 
				top_level->get_resources()->button_light, 
				ltface, 
				ltface, 
				top_level->get_resources()->button_shadow,
				BLACK);
		else
			draw_3d_big(x1, y1, hw, hs, 
				top_level->get_resources()->button_light, 
				dkface, 
				dkface, 
				top_level->get_resources()->button_shadow,
				BLACK);

		//set_color(BLACK);
		//draw_line(x1, y1 + hs / 2, x1 + hw - 1, y1 + hs / 2);

		set_color(RED);
		set_font(SMALLFONT);
		if(fader)
		{
			draw_center_text(x1 + w / 2, y1 + hs - 2, text, SMALLFONT);
		}
		else
		{
			draw_center_text(x + w / 2, h / 2, text, SMALLFONT);
		}
		set_font(MEDIUMFONT);
	}
	flash();
return 0;
}



//  ========================= event dispatch handlers

int BC_Slider_Base::keypress_event_()
{
	int result;
	result = 0;
	
	if(get_active_tool() == this)
	{
		if(top_level->get_keypress() == LEFT) { decrease_level(); result = 3; };
		if(top_level->get_keypress() == RIGHT) { increase_level(); result = 3; };
		if(top_level->get_keypress() == DOWN) { decrease_level(); result = 3; };
		if(top_level->get_keypress() == UP) { increase_level(); result = 3; };
	}

	if(result == 3) 
	{
		handle_event(); 
		trap_keypress(); 
	}    // user event handler before trapping
	return result;
return 0;
}

int BC_Slider_Base::cursor_left_()
{
	if(highlighted)
	{
		if(cursor_x < 0 || cursor_x > w ||
			 cursor_y < 0 || cursor_y > h)
		{   // draw highlighted
			highlighted = 0;
			update_();
		}
	}
return 0;
}

int BC_Slider_Base::button_release_()
{
	if(buttondown) buttondown = 0;
return 0;
}

int BC_Slider_Base::cursor_motion_()
{
	int result;
	result = 0;

	if(buttondown)
	{
		result = cursor_motion_derived();
	}
	else
	if(cursor_x > 0 && cursor_x < w &&
		 cursor_y > 0 && cursor_y < h)
	{
//		result = 1;
		if(!highlighted)
		{   // draw highlighted
			top_level->unhighlight();
			highlighted = 1;
			update_();
		}
	}
	else
	if(cursor_x < 0 || cursor_x > w ||
		 cursor_y < 0 || cursor_y > h)
	{   // draw unhighlighted
		if(highlighted)
		{
			highlighted = 0;
			update_();
		}
	}
	return result;
return 0;
}

int BC_Slider_Base::unhighlight_()
{
	if(highlighted)
	{
		highlighted = 0;
		update_();
	}
	return 0;
return 0;
}

// ==================================== constructors

BC_ISlider::BC_ISlider(int x, int y, int w, int h, int virtual_pixels, int value, int minvalue, int maxvalue, int ltface, int dkface, int fader, int caption)
	: BC_Slider_Base(x, y, w, h, virtual_pixels, ltface, dkface, fader, caption)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
}

BC_FSlider::BC_FSlider(int x, int y, int w, int h, int virtual_pixels, float value, float minvalue, float maxvalue, int ltface, int dkface, int fader, int caption)
	: BC_Slider_Base(x, y, w, h, virtual_pixels, ltface, dkface, fader, caption)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
}

BC_QSlider::BC_QSlider(int x, int y, int w, int h, int virtual_pixels, int value, int minvalue, int maxvalue, int ltface, int dkface, int fader, int caption)
	: BC_Slider_Base(x, y, w, h, virtual_pixels, ltface, dkface, fader, caption)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
}

BC_ISlider::BC_ISlider(int x, int y, int w, int h, int virtual_pixels, int value, int minvalue, int maxvalue, int fader, int caption)
	: BC_Slider_Base(x, y, w, h, virtual_pixels, fader, caption)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
}

BC_FSlider::BC_FSlider(int x, int y, int w, int h, int virtual_pixels, float value, float minvalue, float maxvalue, int fader, int caption)
	: BC_Slider_Base(x, y, w, h, virtual_pixels, fader, caption)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
}

BC_QSlider::BC_QSlider(int x, int y, int w, int h, int virtual_pixels, int value, int minvalue, int maxvalue, int fader, int caption)
	: BC_Slider_Base(x, y, w, h, virtual_pixels, fader, caption)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
}

int BC_ISlider::create_tool_objects()
{
	if(ltface == -1) ltface = top_level->get_resources()->button_highlighted;
	if(dkface == -1) dkface = top_level->get_resources()->button_up;

	backcolor = subwindow->get_color();
	create_window(x, y, w, h, backcolor);
	update(value);
return 0;
}

int BC_FSlider::create_tool_objects()
{
	if(ltface == -1) ltface = top_level->get_resources()->button_highlighted;
	if(dkface == -1) dkface = top_level->get_resources()->button_up;

	backcolor = subwindow->get_color();
	create_window(x, y, w, h, backcolor);
	update(value);
return 0;
}

int BC_QSlider::create_tool_objects()
{
	if(ltface == -1) ltface = top_level->get_resources()->button_highlighted;
	if(dkface == -1) dkface = top_level->get_resources()->button_up;

	backcolor = subwindow->get_color();
	create_window(x, y, w, h, backcolor);
	update(value.freq);
return 0;
}



// ===================================== resizing

int BC_ISlider::set_position(int virtual_pixels, int value, int minvalue, int maxvalue)
{
	this->minvalue = minvalue;
	this->maxvalue = maxvalue;
	this->value = value;
	this->virtual_pixels = virtual_pixels;
	update(value);
return 0;
}


// ==================================== contents updating

int BC_ISlider::update(int value_)
{
	int x_, w_; // inner dimensions
	
	if(w > h)
	{
		 w_ = w - 4 - hs;
		 x_ = 2 + hs / 2;
	}
	else
	{
		 w_ = h - 4 - hs;
		 x_ = 2 + hs / 2;
	}

	if(value_ > maxvalue) value_ = maxvalue;
	if(value_ < minvalue) value_ = minvalue;

	value = value_;
	sprintf(text, "%d", value);
		position = (int)(x_ + w_ * ((float)(value - minvalue) / (float)(maxvalue - minvalue)));
	update_();
return 0;
}

int BC_FSlider::update(float value_)
{
	int x_, w_; // inner dimensions
	
	if(w > h)
	{
		w_ = w - 4 - hs;
		x_ = 2 + hs / 2;
	}
	else
	{
		w_ = h - 4 - hs;
		x_ = 2 + hs / 2;
	}
	
	if(value_ > maxvalue) value_ = maxvalue;
	if(value_ < minvalue) value_ = minvalue;

	value = value_;
	position = (int)(x_ + w_ * (value - minvalue) / (maxvalue - minvalue));
	
	if(value >= 0) sprintf(text, "+%.1f", value);
	else 
	if(value == INFINITYGAIN) sprintf(text, "oo");
	else 
	sprintf(text, "%.1f", value);
	update_();
return 0;
}
	
int BC_QSlider::update(int value_)
{
	int x_, w_; // inner dimensions
	
	if(w > h)
	{
		w_ = w - 4 - hs;
		x_ = 2 + hs / 2;
	}
	else
	{
		w_ = h - 4 - hs;
		x_ = 2 + hs / 2;
	}
	
	if(value_ > maxvalue.freq) value_ = maxvalue.freq;
	if(value_ < minvalue.freq) value_ = minvalue.freq;
	
	value = value_;
	position = (int)(x_ + w_ * ((float)(value.fromfreq() - minvalue.fromfreq()) / (maxvalue.fromfreq() - minvalue.fromfreq())));

	if(value.freq < 10000)
		sprintf(text, "%d", value.freq);
	else
		sprintf(text, "%.1fk", (float)value.freq / 1000);
	update_();
return 0;
}



int BC_ISlider::get_value()
{
	return value;
return 0;
}

int BC_ISlider::get_length()          // get total length of slider
{
	return maxvalue - minvalue;
return 0;
}

float BC_FSlider::get_value()
{
	return value;
}

float BC_FSlider::get_length()          // get total length of slider
{
	return maxvalue - minvalue;
}


int BC_ISlider::update(char *value_)
{
	update(atol(value_));
return 0;
}

int BC_FSlider::update(char *value_)
{
	if(!strcmp(value_, "oo")) update(INFINITYGAIN);
	else update(atof(value_));
return 0;
}

int BC_QSlider::update(char *value_)
{
	update(atol(value_));
return 0;
}


// =============================== event handlers


int BC_FSlider::button_press_()
{
	if(cursor_x > 0 && cursor_x < w
		 && cursor_y > 0 && cursor_y < h)
	{
		if(w > h)
// horizontal slider
			base_pixel = cursor_x;
		else
// vertical slider
			base_pixel = (h - cursor_y);

		base_pixel -= (int)(virtual_pixels * (value - minvalue) / (maxvalue - minvalue));
		buttondown = 1;
		if(get_active_tool() != this){ activate(); }
		return 1;
	}
	return 0;
return 0;
}

int BC_ISlider::button_press_()
{
	if(cursor_x > 0 && cursor_x < w
		 && cursor_y > 0 && cursor_y < h)
	{
		if(w > h)
// horizontal slider
			base_pixel = cursor_x;
		else
// vertical slider
			base_pixel = (h - cursor_y);

		base_pixel -= (int)(virtual_pixels * (float)(value - minvalue) / (maxvalue - minvalue));
		buttondown = 1;
		if(get_active_tool() != this){ activate(); }
		return 1;
	}
	return 0;
return 0;
}

int BC_QSlider::button_press_()
{
	if(cursor_x > 0 && cursor_x < w
		 && cursor_y > 0 && cursor_y < h)
	{
		if(w > h)
// horizontal slider
			base_pixel = cursor_x;
		else
// vertical slider
			base_pixel = (h - cursor_y);
		
		base_pixel -= (int)(virtual_pixels * (float)(value.freq - minvalue.freq) / (maxvalue.freq - minvalue.freq));
		buttondown = 1;
		if(get_active_tool() != this){ activate(); }
		return 1;
	}
	return 0;
return 0;
}

int BC_FSlider::cursor_motion_derived()
{
	int virtual_pixel;

	if(w > h)
		virtual_pixel = cursor_x - base_pixel;
	else
		virtual_pixel = (h - cursor_y) - base_pixel;
	
	update(minvalue + (maxvalue - minvalue) / virtual_pixels * virtual_pixel);
	handle_event();
	return 1;
return 0;
}

int BC_ISlider::cursor_motion_derived()
{
	int virtual_pixel;
	
	if(w > h)
		virtual_pixel = cursor_x - base_pixel;
	else
		virtual_pixel = (h - cursor_y) - base_pixel;
	
	update(minvalue + (float)(maxvalue - minvalue) / virtual_pixels * virtual_pixel);
	handle_event();
	return 1;
return 0;
}

int BC_QSlider::cursor_motion_derived()
{
	int virtual_pixel;
	
	if(w > h)
		virtual_pixel = cursor_x - base_pixel;
	else
		virtual_pixel = (h - cursor_y) - base_pixel;
	
	update((minvalue.freq + (float)(maxvalue.freq - minvalue.freq) / virtual_pixels) * virtual_pixel);
	handle_event();
	return 1;
return 0;
}



// ======================================== keypress handlers

int BC_ISlider::decrease_level()
{
	value--;
	if(value < minvalue) value = minvalue;
	update(value);
return 0;
}

int BC_FSlider::decrease_level()
{
	value -= 0.1;
	if(value < minvalue) value = minvalue;
	update(value);
return 0;
}

int BC_QSlider::decrease_level()
{
	--value;
	update(value.freq);
return 0;
}

int BC_ISlider::increase_level()
{
	value++;
	if(value > maxvalue) value = maxvalue;
	update(value);
return 0;
}

int BC_FSlider::increase_level()
{
	value += 0.1;
	if(value > maxvalue) value = maxvalue;
	update(value);
return 0;
}

int BC_QSlider::increase_level()
{
	++value;
	update(value.freq);
return 0;
}
