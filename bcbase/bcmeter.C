#include <string.h>
#include "bccolors.h"
#include "bcfont.h"
#include "bcmeter.h"
#include "bcresources.h"
#include "bcwindow.h"

BC_Meter::BC_Meter(int x, int y, int w, int h, 
	float min, 
	int mode, 
	long over_delay,
	long peak_delay)
	:	BC_Tool(x, y, w, h)
{
	this->over_delay = over_delay;
	this->peak_delay = peak_delay;
	this->min = min;
	this->mode = mode;
	init_graphics();
}

BC_Meter::~BC_Meter()
{
	for(int i = 0; i < METER_TITLES; i++) delete db_titles[i];
}

int BC_Meter::set_delays(int over_delay, int peak_delay)
{	
	this->over_delay = over_delay;
	this->peak_delay = peak_delay;
return 0;
}

int BC_Meter::init_graphics()
{
	low_color = GREEN;
	medium_color = MEYELLOW;
	high_color = RED;
	vertical = h > w;
	total_width = vertical ? h : w;
	for(int i = 0; i < METER_TITLES; i++) db_titles[i] = new char[6];
	
// calibrate the db titles
	get_divisions(&low_division, 
				&medium_division, 
				total_width, 
				db_titles, 
				title_x, 
				vertical);
return 0;
}

int BC_Meter::get_divisions(int *low_division, 
						int *medium_division, 
						int total_width, 
						char **titles, 
						int *title_x, 
						int vertical)
{
	int i;
	float j, j_step;
	int division, division_step;
	
	division = 0;
	division_step = total_width / 4;
	j = min;     // number for title
	j_step = min / 4;

	for(i = 0; i < 4;)
	{
		sprintf(titles[i], "%.0f", j);

		if(i == 0) title_x[i] = division; else title_x[i] = division - 10;
		division += division_step;
		j -= j_step;
		i++;
	}
	
	sprintf(titles[4], "%.0f", j_step / 2);
	title_x[4] = (division + title_x[3]) / 2;
	
	sprintf(titles[5], "0");
	title_x[5] = division - 10;

	*low_division = (int)title_x[2] + 10;
	*medium_division = title_x[4] + 10;
return 0;
}

int BC_Meter::create_tool_objects()
{
	create_window(x, y, w, h, BLACK);
	peak_timer = 0;
	level_x = peak_x = 0;

	over_timer = 0;
	over_count = 0;
	draw();
return 0;
}

int BC_Meter::resize_tool(int x, int y, int w, int h)
{
	resize_window(x, y, w, h);
	for(int i = 0; i < 6; i++) delete db_titles[i];
	init_graphics();
	draw();
return 0;
}

int BC_Meter::reset()
{
	level_x = peak_x = 0;
	peak_timer = 0;
	over_timer = 0;
	over_count = 0;
	draw();
return 0;
}

int BC_Meter::reset_over()
{
	over_timer = 0;
return 0;
}

int BC_Meter::change_format(int mode)
{
	this->mode = mode;
return 0;
}

int BC_Meter::draw()
{
	int low_size = low_division;
	int medium_size = medium_division - low_division;
	int high_size = total_width - medium_division;

	draw_3d_big(0, 0, w, h, 
		top_level->get_resources()->button_shadow, 
		BLACK, 
		BLACK, 
		top_level->get_resources()->button_down,
		top_level->get_resources()->button_light);


// draw bars
	if(level_x > 0)
	{
		if(level_x < total_width) high_size = level_x - medium_division;
		if(level_x < medium_division) medium_size = level_x - low_division;
		if(level_x < low_division) low_size = level_x;

		if(low_size > 0)
		{
			set_color(low_color);
			if(vertical) 
			draw_box(2, h - low_size - 2, w - 4, low_size);
			else
			draw_box(2, 2, low_size, h - 4);
		}
		
		if(medium_size > 0)
		{
			set_color(medium_color);
			if(vertical)
			draw_box(2, h - low_division - medium_size - 2, w - 4, medium_size);
			else
			draw_box(low_division, 2, medium_size, h - 4);
		}
		
		if(high_size > 0)
		{
			set_color(high_color);
			if(vertical)
			draw_box(2, h - medium_division - high_size - 2, w - 4, high_size);
			else
			draw_box(medium_division, 2, high_size, h - 4);
		}
	}

// draw peak
	peak_x1 = peak_x - 2;
	peak_x2 = peak_x;
	
	if(peak_x1 < 0) peak_x1 = 0;
	if(peak_x > 0)
	{
		if(peak_x2 > medium_division) set_color(high_color);
		else
		if(peak_x2 > low_division) set_color(medium_color);
		else
		set_color(low_color);

		if(vertical)
		draw_box(2, h - peak_x1 - 4, w - 4, peak_x2 - peak_x1);
		else
		draw_box(peak_x1 + 2, 2, peak_x2 - peak_x1, h-4);
	}

// draw over
	if(over_timer) 
	{
		set_color(RED);

		if(vertical)
		draw_text(10, h - 25, "O");
		else
		draw_text(25, h / 2 + get_text_height(MEDIUMFONT) / 2 - 2, "OVER");

// force user to reset
		over_timer--;
	}
	flash();
	return 0;
}

int BC_Meter::update(float new_value, int over)
{
	peak_timer++;
	if(mode == METER_DB)
	{
		if(new_value == 0) new_value = min;
		else
		new_value = db.todb(new_value);        // db value
		
		level_x = total_width - 4 - (int)((new_value / min) * total_width);
	}
	
	if(mode == METER_INT)
	{
		level_x = (int)(new_value * total_width - 4);
	}
	
	if(level_x > total_width - 4) level_x = total_width - 4;
	if(level_x > peak_x)
	{
		peak_x = level_x;
		peak_timer = 0;
	}
	else
	if(peak_timer > peak_delay)
	{
		peak_x = level_x;
		peak_timer = 0;
	}

	if(over) over_timer = over_delay;	
// only draw if window is visible

	if(!top_level->get_hidden())
	{
		draw();
	}
return 0;
}
