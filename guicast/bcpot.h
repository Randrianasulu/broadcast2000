#ifndef BCPOT_H
#define BCPOT_H

#include "bcpixmap.h"
#include "vframe.inc"
#include "bcsubwindow.h"

#define POT_UP 0
#define POT_HIGH 1
#define POT_DN 2
#define POT_STATES 3

class  BC_FPot;
class  BC_IPot;
class  BC_PercentagePot;

class BC_Pot : public BC_SubWindow
{
public:
	BC_Pot(int x, int y, VFrame **data);
	virtual ~BC_Pot();

	friend BC_FPot;
	friend BC_IPot;
	friend BC_PercentagePot;

	int initialize();
	virtual float get_percentage() { return 0; };
	virtual int percentage_to_value(float percentage) { return 0; };
	virtual int handle_event() { return 0; };
	virtual const char* get_caption() { return ""; };
	virtual int increase_value() { return 0; };
	virtual int decrease_value() { return 0; };

	int reposition_window(int x, int y);
	int repeat_event(long repeat_id);
	int cursor_enter_event();
	int cursor_leave_event();
	int button_press_event();
	int button_release_event();
	int cursor_motion_event();
	int keypress_event();

private:
	int set_data(VFrame **data);
	int draw();
	float percentage_to_angle(float percentage);
	float angle_to_percentage(float angle);
	int angle_to_coords(int &x1, int &y1, int &x2, int &y2, float angle);
	float coords_to_angle(int x2, int y2);
	void show_value_tooltip();
	
	VFrame **data;
	BC_Pixmap *images[POT_STATES];
	char caption[BCTEXTLEN], temp_tooltip_text[BCTEXTLEN];
	int status;
	long keypress_tooltip_timer;
	float angle_offset;
	float start_cursor_angle;
	float start_needle_angle;
	float prev_angle, angle_correction;
};

class BC_FPot : public BC_Pot
{
public:
	BC_FPot(int x, 
		int y, 
		float value, 
		float minvalue, 
		float maxvalue, 
		VFrame **data = 0);
	~BC_FPot();

	char* get_caption();
	int increase_value();
	int decrease_value();
	float get_percentage();
	float get_value();
	int percentage_to_value(float percentage);
	void update(float value);

private:
	float value, minvalue, maxvalue;
};

class BC_IPot : public BC_Pot
{
public:
	BC_IPot(int x, 
		int y, 
		long value, 
		long minvalue, 
		long maxvalue, 
		VFrame **data = 0);
	~BC_IPot();

	char* get_caption();
	int increase_value();
	int decrease_value();
	float get_percentage();
	int percentage_to_value(float percentage);
	long get_value();
	void update(long value);

private:
	long value, minvalue, maxvalue;
};

class BC_PercentagePot : public BC_Pot
{
public:
	BC_PercentagePot(int x, 
		int y, 
		float value, 
		float minvalue, 
		float maxvalue, 
		VFrame **data = 0);
	~BC_PercentagePot();

	char* get_caption();
	int increase_value();
	int decrease_value();
	float get_percentage();
	float get_value();
	int percentage_to_value(float percentage);
	void update(float value);

private:
	float value, minvalue, maxvalue;
};

#endif
