#ifndef BEZIERAUTOS_H
#define BEZIERAUTOS_H

#include "auto.inc"
#include "autos.h"
#include "bcbase.h"
#include "bezierauto.inc"
#include "filehtal.inc"
#include "mainwindow.inc"
#include "track.inc"

// how to draw a bezier curve
//
// x(t)=(1-t)^3*x0+3*t*(1-t)^2*x1+3*t^2*(1-t)*x2+t^3*x3
// y(t)=(1-t)^3*y0+3*t*(1-t)^2*y1+3*t^2*(1-t)*y2+t^3*y3
//
// 0 <= t <= 1
// (x1, y1)  (x3, y3) control points
// (x0, y0)  (x2, y2) end points

class BezierAutos : public Autos
{
public:
	BezierAutos(MainWindow *mwindow, 
				Track *track, 
				int color, 
				float center_x,
				float center_y, 
				float center_z, 
				float frame_w,
				float frame_h,
				float virtual_w = 160,
				float virtual_h = 160);
	~BezierAutos();

	int paste_derived(FileHTAL *htal, long start);

	int draw(BC_Canvas *canvas, 
				int pixel, 
				int zoom_track, 
				float units_per_pixel, 
				float view_start, 
				int vertical);

	int get_center(float &x, float &y, float &z, float frame,
				int reverse,
				BezierAuto **before, BezierAuto **after);

	int draw_floating_autos(BC_Canvas *canvas, 
				int pixel, 
				int zoom_track, 
				float units_per_pixel, 
				float view_start, 
				int vertical, 
				int flash);

	int select_auto(BC_Canvas *canvas, 
					int pixel, 
					int zoom_track, 
					float units_per_pixel, 
					float view_start, 
					int cursor_x, 
					int cursor_y, 
					int shift_down,
					int ctrl_down,
					int mouse_button,
					int vertical);

	int move_auto(BC_Canvas *canvas, 
					int pixel, 
					int zoom_track, 
					float units_per_pixel, 
					float view_start, 
					int cursor_x, 
					int cursor_y, 
					int shift_down, 
					int vertical);

	int release_auto_derived();
	int scale_video(float scale, int *offsets);

	Auto* add_auto(long frame, float x, float y, float z);
	Auto* append_auto();
	int get_frame_half(float scale, int vertical, float units_per_pixel);
// Get the pixel on the track display of the center of an auto
	int get_auto_pixel(long position, float view_start, float units_per_pixel, int frame_half);
// Get the frame on the track of the cursor pixel
	long get_auto_frame(int position, float view_start, float units_per_pixel, int vertical);

// need frame dimensions here so same auto can be camera or projector
	float frame_w, frame_h;
	float virtual_center_x, virtual_center_y;
	float virtual_w;
	float center_x, center_y, center_z;   // default values
	BezierAuto *old_selected, *new_selected;

// selection type is
// 0 - none
// 1 - frame number
// 2 - xy
// 3 - zoom
// 4 - control_in_xy
// 5 - control_out_xy
// 6 - control_in_zoom
// 7 - control_out_zoom
	int selection_type;
	MainWindow *mwindow;

private:
	int get_virtual_center(float &x, float &y, int cursor_x, int cursor_y, int vertical, float scale);
// don't draw the new auto position
	int swap_out_selected();         
	int swap_in_selected();
};




#endif
