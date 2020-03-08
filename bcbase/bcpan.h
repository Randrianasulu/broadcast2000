#ifndef BCVECTOR_H
#define BCVECTOR_H

class BC_Pan;

#include "bctool.h"


// pan  angles
//
//        360/0
//
//     270      90
//
//         180

class BC_Pan : public BC_Tool
{
public:
	BC_Pan(int x, int y, int r, int virtual_r, float maxvalue, int total_values, int *value_positions, int stick_x, int stick_y);
	BC_Pan(int x, int y, int r, int virtual_r, float maxvalue, int total_values, int *value_positions, float *values);
	virtual ~BC_Pan();

	int initialize(int r, int virtual_r, float maxvalue, int total_values, int *value_positions);
	int create_tool_objects();

	int update(int x, int y);
	int resize_tool(int x, int y);

// queries
	int get_stick_x();
	int get_stick_y();
	float get_value(int channel);
	int get_total_values();

	int keypress_event_();
	int cursor_left_();
	int cursor_motion_();
	int button_press_();
	int button_release_();
	int unhighlight_();

// change radial positions of channels
	int change_channels(int new_channels, int *value_positions);
// get x and y positions of channels
	int get_channel_positions();
// update stick position from values
// only works for max and min values
	int get_stick_position(int total_values, int *value_positions, float *values, float maxvalue, int virtual_r);
// update values from stick position
	int get_values();

	int draw();
	int rdtoxy(int &x, int &y, int r, int a, int virtual_r);
	int distance(int x1, int x2, int y1, int y2);

	int button_down;
	int highlighted;
	int total_values;
	int r;
	float maxvalue;
	int virtual_r;
	float scale;
	int x_adjust, y_adjust;

private:
	float *values;
	int *value_positions;       // degree measures of channels
	int *value_x, *value_y;     // virtual x and y positions
	int stick_x, stick_y;    // position of stick
	char string[64];
};







#endif
