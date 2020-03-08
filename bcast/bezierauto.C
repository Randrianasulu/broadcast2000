#include <string.h>
#include "bezierauto.h"
#include "bezierautos.h"
#include "filehtal.h"

BezierAuto::BezierAuto(BezierAutos *autos)
 : Auto((Autos*)autos)
{
	WIDTH = 10;
	HEIGHT = 10;
	center_x = 0;
	center_y = 0;
	center_z = 1;
	control_in_x = 0;
	control_in_y = 0;
	control_in_z = 1;
	control_out_x = 0;
	control_out_y = 0;
	control_out_z = 1;
}

BezierAuto::~BezierAuto()
{
}

int BezierAuto::save(FileHTAL *htal)
{
	htal->tag.set_title("AUTO");
	htal->tag.set_property("CENTER_X", center_x);
	htal->tag.set_property("CENTER_Y", center_y);
	htal->tag.set_property("CENTER_Z", center_z);
	htal->tag.set_property("CONTROL_IN_X", control_in_x);
	htal->tag.set_property("CONTROL_IN_Y", control_in_y);
	htal->tag.set_property("CONTROL_IN_Z", control_in_z);
	htal->tag.set_property("CONTROL_OUT_X", control_out_x);
	htal->tag.set_property("CONTROL_OUT_Y", control_out_y);
	htal->tag.set_property("CONTROL_OUT_Z", control_out_z);
	htal->tag.set_property("FRAME", position);
	htal->append_tag();
	htal->append_newline();
return 0;
}

int BezierAuto::load(FileHTAL *htal)
{
	position = htal->tag.get_property("FRAME", (long)0);
	center_x = htal->tag.get_property("CENTER_X", (float)0);
	center_y = htal->tag.get_property("CENTER_Y", (float)0);
	center_z = htal->tag.get_property("CENTER_Z", (float)1);
	control_in_x = htal->tag.get_property("CONTROL_IN_X", (float)0);
	control_in_y = htal->tag.get_property("CONTROL_IN_Y", (float)0);
	control_in_z = htal->tag.get_property("CONTROL_IN_Z", (float)1);
	control_out_x = htal->tag.get_property("CONTROL_OUT_X", (float)0);
	control_out_y = htal->tag.get_property("CONTROL_OUT_Y", (float)0);
	control_out_z = htal->tag.get_property("CONTROL_OUT_Z", (float)1);
return 0;
}

int BezierAuto::copy(long start, long end, FileHTAL *htal)
{
//printf("BezierAuto::copy %ld %ld %ld\n", start, end, position);
	if(position >= start && position <= end)
	{
		htal->tag.set_title("AUTO");
		htal->tag.set_property("CENTER_X", center_x);
		htal->tag.set_property("CENTER_Y", center_y);
		htal->tag.set_property("CENTER_Z", center_z);
		htal->tag.set_property("CONTROL_IN_X", control_in_x);
		htal->tag.set_property("CONTROL_IN_Y", control_in_y);
		htal->tag.set_property("CONTROL_IN_Z", control_in_z);
		htal->tag.set_property("CONTROL_OUT_X", control_out_x);
		htal->tag.set_property("CONTROL_OUT_Y", control_out_y);
		htal->tag.set_property("CONTROL_OUT_Z", control_out_z);
		htal->tag.set_property("FRAME", position - start);
//printf("BezierAuto::copy %ld\n", position - start);
		htal->append_tag();
	}
return 0;
}

int BezierAuto::draw(BC_Canvas *canvas, 
				int x, 
				int center_pixel, 
				float scale,
				int vertical,
				int show_value)
{
	show_value = 0;    // skip drawing value for now
	static int control_x[5], control_y[5];
	static int i;

	get_control_points(x, center_pixel, scale, control_x, control_y, vertical);

	canvas->draw_disc(control_x[0] - WIDTH/2, control_y[0] - HEIGHT/2, WIDTH, HEIGHT);
	for(i = 1; i < 5; i++)
	{
// Draw zooms hollow and pan opaque.
		if(i < 3) 
			canvas->draw_box(control_x[i] - WIDTH/2, control_y[i] - HEIGHT/2, WIDTH, HEIGHT);
		else
			canvas->draw_rectangle(control_x[i] - WIDTH/2, control_y[i] - HEIGHT/2, WIDTH, HEIGHT);

		canvas->draw_line(control_x[i], control_y[i], control_x[0], control_y[0]);
	}

	if(show_value && ((BezierAutos*)autos)->selection_type > 1)
	{
		static char string[16];
		static int text_x, text_y; // text position relative to canvas
		
		value_to_str(string);

		text_x = control_x[((BezierAutos*)autos)->selection_type - 2] + 20;
		text_y = control_y[((BezierAutos*)autos)->selection_type - 2] + 20;
		//canvas->set_font(SMALLFONT);

		canvas->draw_text(text_x, text_y, string);
		//canvas->set_font(MEDIUMFONT);
	}
return 0;
}

int BezierAuto::select(BC_Canvas *canvas, 
				int x, 
				int center_pixel, 
				float scale,
				int cursor_x, 
				int cursor_y, 
				int shift_down,
				int ctrl_down,
				int mouse_button,
				int vertical)
{
	int control_x[5], control_y[5];
	get_control_points(x, center_pixel, scale, control_x, control_y, vertical);

	if(vertical)
	{
		cursor_x ^= cursor_y;
		cursor_y ^= cursor_x;
		cursor_x ^= cursor_y;
	}

	if(shift_down)
	{
// test control_in_xy
		if(mouse_button == 1 
			&& test_control_point(control_x[0], control_y[0], control_x[1], control_y[1], cursor_x, cursor_y))
		{ return 4; }

// test control_out_xy
		if(mouse_button == 1 
			&& test_control_point(control_x[0], control_y[0], control_x[2], control_y[2], cursor_x, cursor_y))
		{ return 5; }

// test control_in_zoom
		if(mouse_button == 3
			&& test_control_point(control_x[0], control_y[0], control_x[3], control_y[3], cursor_x, cursor_y))
		{ return 6; }

// test control_out_zoom
		if(mouse_button == 3 
			&& test_control_point(control_x[0], control_y[0], control_x[4], control_y[4], cursor_x, cursor_y))
		{ return 7; }
	}
	else
	if(cursor_x > control_x[0] - WIDTH/2 &&
	cursor_x < control_x[0] + WIDTH/2 &&
	cursor_y > control_y[0] - HEIGHT/2 &&
	cursor_y < control_y[0] + HEIGHT/2)
	{
		if(mouse_button == 1)
		{
			if(ctrl_down)
				return 2;      // move xy
			else
				return 1;      // move frame
		}
		else
		if(mouse_button == 3)
		{
			if(ctrl_down)
				return 3;      // move zoom
		}
	}

	return 0;
return 0;
}

int BezierAuto::test_control_point(int center_x, int center_y, int x, int y, int cursor_x, int cursor_y)
{
	if(cursor_x > x - WIDTH/2 &&
		cursor_x < x + WIDTH/2 &&
		cursor_y > y - HEIGHT/2 &&
		cursor_y < y + HEIGHT/2) return 1;

// 	if(((x > center_x) ? cursor_x > center_x - WIDTH/2 : cursor_x > x - WIDTH/2) &&
// 		((x > center_x) ? cursor_x < x + WIDTH/2 : x < center_x + WIDTH/2) &&
// 		((y > center_y) ? cursor_y > center_y - HEIGHT/2 : cursor_y > y - HEIGHT/2) &&
// 		((y > center_y) ? cursor_y < y - HEIGHT/2 : cursor_y < center_y + HEIGHT/2))
// 	{
// 		float A = cursor_x - center_x;
// 		float B = cursor_y - center_y;
// 		float C = x - center_x;
// 		float D = y - center_y;
// 		float dist = (A * D - B * C) / sqrt(C * C + D * D);
// 
// printf("%f\n", dist);
// 		if(dist < WIDTH / 2) return 1;
// 	}
	
	return 0;
return 0;
}

float BezierAuto::get_distance(int x1, int y1, int x2, int y2)
{
	return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
}

int BezierAuto::get_control_points(int x, int center_pixel, float scale, int *control_x, int *control_y, int vertical)
{
// get control point locations relative to canvas
	int y = center_pixel;
	float zoom_scale;
	if(vertical)
	{
		y = x;
		x = center_pixel;
	}

	zoom_scale = ((BezierAutos*)autos)->frame_h * scale;

// frame point
	control_x[0] = x;
	control_y[0] = y;
// control_in_xy
	control_x[1] = (int)(control_in_x * scale + x);
	control_y[1] = (int)(control_in_y * scale + y);
// control_out_xy
	control_x[2] = (int)(control_out_x * scale + x);
	control_y[2] = (int)(control_out_y * scale + y);
// zoom_in_control
	control_x[3] = (int)x;
	control_y[3] = (int)(control_in_z * zoom_scale + y);
//printf("BezierAuto::get_control_points %d %f %f %d\n", control_y[3], control_in_z, scale, y);
// zoom_out_control
	control_x[4] = (int)x;
	control_y[4] = (int)(control_out_z * zoom_scale + y);
return 0;
}

int BezierAuto::value_to_str(char *string)
{
// not used
	if(((BezierAutos*)autos)->frame_w)
	switch(((BezierAutos*)autos)->selection_type)
	{
		case 2:
			sprintf(string, "%.0f, %.0f", center_x, center_y);
			break;

		case 3:
			sprintf(string, "%.0f, %.0f", control_in_x, control_in_y);
			break;

		case 4:
			sprintf(string, "%.0f, %.0f", control_out_x, control_out_y);
			break;
	}
	else
	switch(((BezierAutos*)autos)->selection_type)
	{
		case 2:
			sprintf(string, "%.0f", center_y);
			break;

		case 3:
			sprintf(string, "%.0f", control_in_y);
			break;

		case 4:
			sprintf(string, "%.0f", control_out_y);
			break;
	}
return 0;
}



