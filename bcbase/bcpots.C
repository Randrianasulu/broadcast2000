#include <string.h>
#include "bcfont.h"
#include "bcpots.h"
#include "bcresources.h"

int BC_Pot_Base::draw_pot()
{
	draw_3d_circle(0, 0, w, h, 
		top_level->get_resources()->button_light, 
		highlighted ? ltface : dkface, 
		highlighted ? ltface : dkface, 
		top_level->get_resources()->button_shadow, 
		BLACK);

// draw pointing line
	if(ltface == BLACK || dkface == BLACK) 
	set_color(WHITE);
	else
	set_color(BLACK);

	draw_line((w - 1) / 2, (h - 2) / 2, x2, y2);

// draw text
	if(ltface == RED || dkface == RED) set_color(WHITE);
	else
	if(ltface == BLACK || dkface == BLACK) set_color(MEYELLOW);
	else set_color(RED);
	set_font(SMALLFONT);
	draw_center_text(w / 2, (int)(h - 10), text, SMALLFONT);
	set_font(LARGEFONT);

	flash();
return 0;
}

BC_Pot_Base::BC_Pot_Base(int x_, int y_, int w_, int h_, int ltface_, int dkface_)
	: BC_Tool(x_, y_, w_, h_)
{
	buttondown = 0;
	highlighted = 0;
	dkface = dkface_; ltface = ltface_;
}

int BC_Pot_Base::change_y_(int y)
{
	y2 += y;
return 0;
}

int BC_Pot_Base::update_()
{
// get angle of pointer

  y2 = (int)(cos((angle + .25) * M_PI) * (h - 2) / 2);
  x2 = -(int)(sin((angle + .25) * M_PI) * (w - 1) / 2);
  
  x2 += (w - 1) / 2;
  y2 += (h - 2) / 2;

	draw_pot();    // draw pot
return 0;
}


int BC_Pot_Base::keypress_event_()
{
	int result = 0;
	if(get_active_tool() == this){
		if(top_level->get_keypress() == LEFT) { decrease_level(); result = 3; }
		else
		if(top_level->get_keypress() == RIGHT) { increase_level(); result = 3; }
		else
		if(top_level->get_keypress() == DOWN) { decrease_level(); result = 3; }
		else
		if(top_level->get_keypress() == UP) { increase_level(); result = 3; }
	}
	if(result == 3) { trap_keypress(); handle_event(); }          // give to user event
	return result;
return 0;
}

int BC_Pot_Base::button_release_()
{
	if(buttondown) buttondown = 0;
return 0;
}

int BC_Pot_Base::button_press_()
{
	int result;
	float x_, y_;
	result = 0;
	
	if(cursor_x > 0 && cursor_x < w
		 && cursor_y > 0 && cursor_y < h)
	{
		x_ = cursor_x - w / 2;
		y_ = cursor_y - h / 2;

// flag for selection just initiated
		result = buttondown = 2;
		if(get_active_tool() != this){ activate(); }
		get_arc_length(result, x_, y_);
		return 1;
	}
	return 0;
return 0;
}

int BC_Pot_Base::cursor_motion_()
{
	float x_, y_;
	int result;
	result = 0;
	
	if(buttondown)
	{
		x_ = cursor_x - w / 2;
		y_ = cursor_y - h / 2;
		result = 1;
		get_arc_length(result, x_, y_);
	}
	else
	if(cursor_x < 0 || cursor_x > w ||
		 cursor_y < 0 || cursor_y > h)
	{
		if(highlighted)
		{   // draw highlighted
			highlighted = 0;
			draw_pot();
		}
	}
	else
	if(cursor_x > 0 && cursor_x < w &&
		 cursor_y > 0 && cursor_y < h)
	{
		if(!highlighted)
		{   // draw highlighted
			top_level->unhighlight();
			highlighted = 1;
			draw_pot();
		}
	}
	return result;
return 0;
}

int BC_Pot_Base::cursor_left_()
{
	if(highlighted)
	{
		if(cursor_x < 0 || cursor_x > w ||
			 cursor_y < 0 || cursor_y > h)
		{
			highlighted = 0;
			draw_pot();
		}
	}
return 0;
}

int BC_Pot_Base::get_arc_length(int result, float x_, float y_)
{
// find arc length travelled by pointer
	if(result == 1 || result == 2)
	{
// for initial button press
		if(result == 2)
		{
			base_angle = angle;
			negative = 0;
		}

// get the new angle pointed at by cursor
		base3 = get_angle(x_, y_);

// make the new angle the base angle for the initial selection
		if(result == 2)
		{
			base1 = base2 = base3;
		}

		switch(negative)
		{
			case 0:
				if(base3 > 1.75 && base2 < .25) negative = -1;   // move into negative
				else if(base3 < .25 && base2 > 1.75) negative = 1;   // move into positive
				break;
			
			case -1:
				if(base3 < .25 && base2 > 1.75) negative = 0;
				break;
			
			case 1:
				if(base3 > 1.75 && base2 < .25) negative = 0;
				break;
		}
		
		base2 = base3;
		switch(negative)
		{
			case -1:
				base3 -= 2;
				break;
			
			case 1:
				base3 += 2;
				break;
			
			default:
				break;
		}
		
		angle = base_angle + (base3 - base1);
		if(angle < 0) angle = 0;
		if(angle > 1.5) angle = 1.5;

// send to the derived pot
		handle_event_derived();
	}
	if(result) handle_event();    // give to user event handler
return 0;
}

float BC_Pot_Base::get_angle(float x_, float y_)
{
	float angle;
	float x1, y1;
	
	if(x_ < 0 && y_ > 0){
		x1 = y_;
		y1 = x_;
	}else{
		x1 = x_;
		y1 = y_;
	}

	if(!y_ || !x_){
		if(x_ < 0) angle = .5;
		else
		if(x_ > 0) angle = 1.5;
		else
		if(y_ < 0) angle = 1;
		else
		if(y_ > 0) angle = 0;
	}else{
		angle = atan(y1 / x1);
		angle /= M_PI;
	}

	// fix angle

	if(x_ < 0 && y_ < 0){
		angle += .5;
	}else
	if(x_ > 0 && y_ > 0){
		angle += 1.5;
	}else
	if(x_ > 0 && y_ < 0){
		angle += 1.5;
	}

	if(x_ < 0 && y_ > 0) angle = -angle;
	return angle;
}
	
BC_IPot::BC_IPot(int x_, int y_, int w_, int h_, int value_, int minvalue_, int maxvalue_, int ltface_, int dkface_)
	: BC_Pot_Base(x_, y_, w_, h_, ltface_, dkface_)
{
	minvalue = minvalue_;
	maxvalue = maxvalue_;
	value = value_;
}

BC_FPot::BC_FPot(int x_, int y_, int w_, int h_, float value_, float minvalue_, float maxvalue_, int ltface_, int dkface_)
	: BC_Pot_Base(x_, y_, w_, h_, ltface_, dkface_)
{
	minvalue = minvalue_;
	maxvalue = maxvalue_;
	value = value_;
}

BC_QPot::BC_QPot(int x_, int y_, int w_, int h_, int value_, int minvalue_, int maxvalue_, int ltface_, int dkface_)
	: BC_Pot_Base(x_, y_, w_, h_, ltface_, dkface_)
{
	minvalue = minvalue_;
	maxvalue = maxvalue_;
	value = value_;
}

BC_QPot::BC_QPot(int x_, int y_, int w_, int h_, Freq value_, Freq minvalue_, Freq maxvalue_, int ltface_, int dkface_)
	: BC_Pot_Base(x_, y_, w_, h_, ltface_, dkface_)
{
	minvalue = minvalue_;
	maxvalue = maxvalue_;
	value = value_;
}


int BC_IPot::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	update(value);
return 0;
}

int BC_FPot::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	update(value);
return 0;
}

int BC_QPot::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	update(value.freq);
return 0;
}

int BC_IPot::update(int value_)
{
	value = value_;
	angle = (float)(value - minvalue) / (maxvalue - minvalue) * 1.5;
	sprintf(text, "%d", value);
	update_();
return 0;
}

int BC_IPot::get_value()
{
	return value;
return 0;
}

int BC_QPot::get_value()
{
	return value.freq;
return 0;
}

// Freq BC_QPot::get_value()
// {
// 	return value;
// }

float BC_FPot::get_value()
{
	return value;
}


int BC_FPot::update(float value_)
{
	value = value_;
	angle = (value - minvalue) / (maxvalue - minvalue) * 1.5;

	if(value >= 0) sprintf(text, "+%.1f", value);
	else if(value > INFINITYGAIN) sprintf(text, "%.1f", value);
	else sprintf(text, "oo");
	
	update_();
return 0;
}
	
int BC_QPot::update(int value_)
{
	value = value_;
	angle = (float)(value.fromfreq() - minvalue.fromfreq()) / (maxvalue.fromfreq() - minvalue.fromfreq()) * 1.5;
	if(value.freq < 1000) sprintf(text, "%d", value.freq);
	else sprintf(text, "%.1fk", (float)value.freq / 1000);

	update_();
return 0;
}

int BC_QPot::update(Freq value_)
{
	update(value_.freq);
return 0;
}

int BC_IPot::update(char *value_)
{
	update(atol(value_));
return 0;
}

int BC_FPot::update(char *value_)
{
	if(!strcmp(value_, "oo")) update(INFINITYGAIN);
	else update(atof(value_));
return 0;
}

int BC_QPot::update(char *value_)
{
	update(atol(value_));
return 0;
}

int BC_IPot::handle_event_derived()
{
	value = (int)(angle / 1.5 * (maxvalue - minvalue) + minvalue);
	update(value);      // draw pot
return 0;
}

int BC_FPot::handle_event_derived()
{
	value = angle / 1.5 * (maxvalue - minvalue) + minvalue;

	if(value >= -0.2 && value <= 0.2) value = 0;  // notched pot
	else{
		value *= 10;             // set precision to .1
		value = int(value);
		value /= 10;
	}
	update(value);      // draw pot
return 0;
}

int BC_QPot::handle_event_derived()
{
	value.tofreq((int)(angle / 1.5 * (maxvalue.fromfreq() - minvalue.fromfreq()) + minvalue.fromfreq()));
	update(value.freq);      // draw pot
return 0;
}

int BC_IPot::decrease_level()
{
	value--;
	if(value < minvalue) value = minvalue;
	update(value);
return 0;
}

int BC_FPot::decrease_level()
{
	value -= 0.1;
	if(value < minvalue) value = minvalue;
	update(value);
return 0;
}

int BC_QPot::decrease_level()
{
	--value;
	update(value.freq);
return 0;
}

int BC_IPot::increase_level()
{
	value++;
	if(value > maxvalue) value = maxvalue;
	update(value);
return 0;
}

int BC_FPot::increase_level()
{
	value += 0.1;
	if(value > maxvalue) value = maxvalue;
	update(value);
return 0;
}

int BC_QPot::increase_level()
{
	++value;
	update(value.freq);
return 0;
}
