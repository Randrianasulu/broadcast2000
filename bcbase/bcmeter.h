#ifndef BCMETER_H
#define BCMETER_H

#include "bcmeter.inc"
#include "bctool.h"
#include "units.h"

class BC_Meter : public BC_Tool
{
public:
	BC_Meter(int x_, int y_, int w_, int h_, 
		float min = -80, 
		int mode = METER_DB, 
		long over_delay = 150,
		long peak_delay = 15);
	virtual ~BC_Meter();
	int create_tool_objects();
	int init_graphics();
	int set_delays(int over_delay, int peak_delay);

	int draw();
	int update(float new_value, int over);
	int resize_tool(int x, int y, int w, int h);
	int reset();
	int reset_over();
	int get_divisions(int *low_division, 
			  int *medium_division,
			  int total_width, 
			  char **titles, 
			  int *title_x, 
			  int vertical);
	int change_format(int mode);

	float peak;
	int peak_timer;
	int peak_x, level_x, peak_x1, peak_x2;
	int over_count, over_timer;
	int total_width, vertical;
	int low_division;
	int medium_division;
	long low_color;
	long medium_color;
	long high_color;
	DB db;
	int title_x[METER_TITLES];
	char *db_titles[METER_TITLES];
	float min;
	int mode;
	long over_delay;       // Number of updates the over warning lasts.
	long peak_delay;       // Number of updates the peak lasts.
};

#endif
