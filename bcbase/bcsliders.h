#ifndef BCSLIDERS_H
#define BCSLIDERS_H

#include "bccolors.h"
#include "bctool.h"
#include "bcwindow.inc"
#include "units.h"

class BC_Slider_Base : public BC_Tool
{
public:
	BC_Slider_Base(int x, 
					int y, 
					int w, 
					int h, 
					int virtual_pixels, 
					int ltface, 
					int dkface, 
					int fader, 
					int caption);

	BC_Slider_Base(int x, 
					int y, 
					int w, 
					int h, 
					int virtual_pixels, 
					int fader, 
					int caption);

// base event dispatch handlers
	int keypress_event_();
	int cursor_motion_();
	int cursor_left_();
	int button_release_();
	int unhighlight_();
	virtual int button_press_() { return 0; };
	virtual int cursor_motion_derived() { return 0; };
	int resize_tool(int x, int y, int w, int h, int virtual_pixels = -1);

	int update_();
	int change_backcolor(int newcolor);
	virtual int increase_level() { return 0; };
	virtual int decrease_level() { return 0; };

	static int hs;      // handle size

	int base_pixel;
	int virtual_pixels;
	int backcolor;
	int position;
	int ltface, dkface, fader;
	int highlighted;
	int buttondown;
	int caption;       // draw the value
	char text[256];


// derived dispatch handler
	virtual int get_new_value(int result, float x_, float y_) { return 0; };
};

// Integer slider
class BC_ISlider : public BC_Slider_Base
{
public:
	BC_ISlider(int x, int y, int w, int h, int virtual_pixels, int value, int minvalue, int maxvalue, int ltface, int dkface, int fader, int caption = 1);
	BC_ISlider(int x, int y, int w, int h, int virtual_pixels, int value, int minvalue, int maxvalue, int fader, int caption = 1);

	int create_tool_objects();
// put new value in slider
	int update(int value_);
	int update(char *value_);
	int get_value();
	int get_length();          // get total length of slider
	int set_position(int virtual_pixels, int value, int minvalue, int maxvalue);

	int button_press_();
	int cursor_motion_derived();

	int increase_level();
	int decrease_level();

	int value, minvalue, maxvalue;
};

// Float slider
class BC_FSlider : public BC_Slider_Base
{
public:
	BC_FSlider(int x, int y, int w, int h, int virtual_pixels, float value, float minvalue, float maxvalue, int ltface, int dkface, int fader, int caption = 1);
	BC_FSlider(int x, int y, int w, int h, int virtual_pixels, float value, float minvalue, float maxvalue, int fader, int caption = 1);

	int create_tool_objects();
// put new value in slider
	int update(float value_);
	int update(char *value_);
	float get_value();
	float get_length();          // get total length of slider

	int button_press_();
	int cursor_motion_derived();

	int increase_level();
	int decrease_level();

	float value, minvalue, maxvalue;
};

// Frequency slider
class BC_QSlider : public BC_Slider_Base
{
public:
	BC_QSlider(int x, int y, int w, int h, int virtual_pixels, int value, int minvalue, int maxvalue, int ltface, int dkface, int fader, int caption = 1);
	BC_QSlider(int x, int y, int w, int h, int virtual_pixels, int value, int minvalue, int maxvalue, int fader, int caption = 1);

	int create_tool_objects();

// put new value in slider
	int update(int value_);
	int update(char *value_);

	int button_press_();
	int cursor_motion_derived();

	int increase_level();
	int decrease_level();

	Freq value, minvalue, maxvalue;
};

#endif
