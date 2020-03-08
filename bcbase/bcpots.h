#ifndef BCPOTS_H
#define BCPOTS_H

#include "bccolors.h"
#include "bckeys.h"
#include "bctool.h"
#include "bcwindow.h"
#include "units.h"

class BC_Pot_Base : public BC_Tool
{
public:
	BC_Pot_Base(int x_, int y_, int w_, int h_, int ltface_, int dkface_);
	virtual int create_tool_objects() { return 0; };
	
// event handler for derived pots
	virtual int handle_event_derived() { return 0; };
	int get_arc_length(int result, float x_, float y_);

// dispatch event handlers
	int cursor_left_();
	int keypress_event_();
	int button_release_();
	int cursor_motion_();
	int button_press_();

	int update_();
	int change_y_(int y);       // required since additional values are affected
	virtual int update() { return 0; };
	virtual int increase_level() { return 0; };
	virtual int decrease_level() { return 0; };

	int x2, y2;      // position of pointer relative to x, y
	float base1, base2, base3, base_angle;
	int negative;
	
// redraw the pot
	int draw_pot();
	float get_angle(float x_, float y_);
	float angle;              // static angle of this pointer
	int buttondown;           // mouse button is down
	int highlighted;
	int ltface, dkface;
	char text[256];
};

// Integer pot
class BC_IPot : public BC_Pot_Base
{
public:
	BC_IPot(int x_, int y_, int w_, int h_, int value_, int minvalue_, int maxvalue_, int ltface_, int dkface_);
	int create_tool_objects();
	
// put new value in pot
	int update(int value_);
	int update(char *value_);
	int get_value();
	int handle_event_derived();

	int increase_level();
	int decrease_level();

	int value, minvalue, maxvalue;
};

// Float pot
class BC_FPot : public BC_Pot_Base
{
public:
	BC_FPot(int x_, int y_, int w_, int h_, float value_, float minvalue_, float maxvalue_, int ltface_, int dkface_);
	int create_tool_objects();
	
// put new value in pot
	int update(float value_);
	int update(char *value_);
	int handle_event_derived();
	int increase_level();
	int decrease_level();
	float get_value();
		
	float value, minvalue, maxvalue;
};

// Frequency pot
class BC_QPot : public BC_Pot_Base
{
public:
	BC_QPot(int x_, int y_, int w_, int h_, int value_, int minvalue_, int maxvalue_, int ltface_, int dkface_);
	BC_QPot(int x_, int y_, int w_, int h_, Freq value_, Freq minvalue_, Freq maxvalue_, int ltface_, int dkface_);
	int create_tool_objects();

// put new value in pot
	int update(int value_);
	int update(Freq value_);
	int update(char *value_);
	int handle_event_derived();
	int increase_level();
	int decrease_level();
	int get_value();
	
	Freq value, minvalue, maxvalue;
};

#endif
