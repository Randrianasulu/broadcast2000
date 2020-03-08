#ifndef BCPAN_H
#define BCPAN_H

// pan  angles
//
//        360/0
//
//     270      90
//
//         180

#include "bcsubwindow.h"

class BC_Pan : public BC_SubWindow
{
public:
	BC_Pan(int x, 
		int y, 
		int virtual_r, 
		float maxvalue, 
		int total_values, 
		int *value_positions, 
		int stick_x, 
		int stick_y, 
		float *values);
	~BC_Pan();

	int initialize();
	int update(int x, int y);
// change radial positions of channels
	int change_channels(int new_channels, int *value_positions);
// update values from stick position
	int stick_to_values();
	int get_total_values();
	float get_value(int channel);
	int get_stick_x();
	int get_stick_y();

private:
	int draw();
// update values from stick position
	float distance(int x1, int x2, int y1, int y2);
// get x and y positions of channels
	int get_channel_positions();
	int rdtoxy(int &x, int &y, int r, int a, int virtual_r);

	int virtual_r;
	float maxvalue;
	int total_values;
	int *value_positions;
	int stick_x;
	int stick_y;
	float *values;
	int highlighted;
	int *value_x, *value_y;     // virtual x and y positions
	BC_Pixmap *bg_pixmap;
	BC_Pixmap *bg_pixmap_hi;
	BC_Pixmap *handle_pixmap, *channel_pixmap;
};

#endif
