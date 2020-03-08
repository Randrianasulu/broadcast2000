#ifndef AUTOS_H
#define AUTOS_H

// base class for automation lists

#include "auto.h"
#include "bcbase.h"
#include "filehtal.inc"
#include "track.inc"

#define AUTOS_VIRTUAL_HEIGHT 160

class Autos : public List<Auto>
{
public:
	Autos(Track *track, 
			int color, 
			float default_, 
			int stack_number = 0, 
			int stack_total = 1);
	virtual ~Autos();

	int create_objects();
	int clear_all();
	int insert(long start, long end);
	int paste_silence(long start, long end);
	int copy(long start, long end, FileHTAL *htal, int autos_follow_edits);
	int paste(long start, long end, long total_length, FileHTAL *htal, char *end_string, int autos_follow_edits, int shift_autos = 1);
	virtual int paste_derived(FileHTAL *htal, long start);
	int clear(long start, long end, int autos_follow_edits, int shift_autos);
	int clear_auto(long position);
	int save(FileHTAL *htal);
	virtual int load(FileHTAL *htal, char *end_string);
	int draw(BC_Canvas *canvas, int pixel, int zoom_track, float units_per_pixel, float view_start, int vertical);
	virtual int swap_out_selected();        // don't draw new auto position
	virtual int swap_in_selected();     
	virtual int slope_adjustment(long ax, float slope);
	virtual int get_track_pixels(int zoom_track, int pixel, int &center_pixel, float &yscale) { return 0; };
	virtual int draw_joining_line(BC_Canvas *canvas, int vertical, int center_pixel, int x1, int y1, int x2, int y2) { return 0; };
	int draw_floating_autos(BC_Canvas *canvas, int pixel, int zoom_track, float units_per_pixel, float view_start, int vertical, int flash);
	int draw_floating(BC_Canvas *canvas, int pixel, int zoom_track, float units_per_pixel, float view_start, int vertical, int flash);
	int select_auto(BC_Canvas *canvas, int pixel, int zoom_track, float units_per_pixel, float view_start, int cursor_x, int cursor_y, int vertical);
	virtual int get_testy(float slope, int cursor_x, int ax, int ay) { return 0; };
	int move_auto(BC_Canvas *canvas, int pixel, int zoom_track, float units_per_pixel, float view_start, int cursor_x, int cursor_y, int shift_down, int vertical);
	virtual float fix_value(float value) { return 0; };
	int release_auto();
	virtual int release_auto_derived() { return 0; };
	virtual Auto* add_auto(long position, float value) { printf("virtual Autos::add_auto\n"); return 0; };
	virtual Auto* append_auto() { printf("virtual Autos::append_auto();\n"); return 0; };
	int scale_time(float rate_scale, int scale_edits, int scale_autos, long start, long end);

// rendering utilities
	int get_neighbors(long start, long end, Auto **before, Auto **after);
	int automation_is_constant(long start, long end, Auto **before, Auto **after);       // 1 if automation doesn't change
	float get_automation_constant(long start, long end, Auto **before, Auto **after);
	int init_automation(long &buffer_position,
				long &input_start, 
				long &input_end, 
				int &automate, 
				float &constant, 
				long input_position,
				long buffer_len,
				Auto **before, 
				Auto **after,
				int reverse);

	int init_slope(Auto **current_auto, 
				float &slope_start, 
				float &slope_value,
				float &slope_position, 
				long &input_start, 
				long &input_end, 
				Auto **before, 
				Auto **after,
				int reverse);

	int get_slope(Auto **current_auto, 
				float &slope_start, 
				float &slope_end, 
				float &slope_value,
				float &slope, 
				long buffer_len, 
				long buffer_position,
				int reverse);

	int advance_slope(Auto **current_auto, 
				float &slope_start, 
				float &slope_value,
				float &slope_position, 
				int reverse);

	Auto* autoof(long position);   // return nearest auto equal to or after position
										                  // 0 if after all autos
	Auto* nearest_before(long position);    // return nearest auto before or 0
	Auto* nearest_after(long position);     // return nearest auto after or 0

	Track *track;
	int color;
	float default_;
	Auto *selected;
	int skip_selected;      // if selected was added
	long selected_position, selected_position_;      // original position for moves
	float selected_value, selected_value_;      // original position for moves
	float virtual_h;  // height cursor moves to cover entire range when track height is less than this
	float min, max;    // default value of this auto
	int virtual_center;
	int stack_number;
	int stack_total;
};






#endif
