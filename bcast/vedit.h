#ifndef VEDIT_H
#define VEDIT_H

#include "bcbase.h"
#include "bezierauto.h"
#include "edit.h"
#include "mainwindow.inc"
#include "pluginbuffer.inc"
#include "vedits.inc"
#include "vframe.inc"

// UNITS ARE FRAMES

class VEdit : public Edit
{
public:
	VEdit(MainWindow *mwindow, VEdits *edits);
	~VEdit();

	int load_properties_derived(FileHTAL *htal);

// ========================================= drawing

	int draw(int flash, int center_pixel, int x, int w, int y, int h, int set_index_file = 0);

// ========================================= editing

	int select_translation(int cursor_x, int cursor_y, float view_start, float zoom_units); // select coordinates of frame
	int update_translation(int cursor_x, int cursor_y, int shift_down, float view_start, float zoom_units);  // move coordinates of frame
	int reset_translation(long start, long end);
	int end_translation();
	int copy_properties_derived(FileHTAL *htal, long length_in_selection);
	int get_handle_parameters(long &left, long &right, long &left_sample, long &right_sample, float view_start, float zoom_units);

// ========================================== rendering
	long compressed_frame_size(long render_video_position);

// Read a frame for direct copy rendering or hardware decompression.
	int read_compressed_frame(VFrame *frame, 
				long project_position);

// Read frame into frame buffer
	int read_raw_frame(VFrame *frame, 
				PluginBuffer *output_buffer, 
				long output_byte_offset, 
				long project_position);

// Read frame into virtual console.				
	int render(VFrame **output, 
					PluginBuffer *output_buffer,
					long output_byte_offset, 
					long input_len, long input_position, float step, 
					BezierAuto **before, BezierAuto **after, int autos_on);
	int dump_derived();
	long get_source_end();

	int center_x, center_y;
	float center_z;      // main camera coordinates
	int layer;

private:
	int get_frame_dimensions(float &frame_w, float &frame_h, float &picture_zoom, float &x_zoom, float &y_zoom);
// get input and output dimensions for drawing a frame on screen
// relative to 0,0
	int get_bounding_box(float &in_x1, float &in_y1, float &in_x2, float &in_y2,
						float &out_x1, float &out_y1, float &out_x2, float &out_y2, 
						long frame, float picture_zoom, int do_output, 
						BezierAuto **before, BezierAuto **after, int autos_on = 1);
// get virtual center point for translation adjustment
//	int get_virtual_center(int cursor_x, int cursor_y, float frame_w, float frame_h, float picture_zoom);
//	int translation_selected;  // if this track is in the middle of coordinate adjustment
// 	int translation_zoom;      // if the zoom is being adjusted
// 	int virtual_center_x, virtual_center_y; // virtual coords of center for translation
	VEdits *vedits;
};




#endif
