#ifndef TRACKCANVAS_H
#define TRACKCANVAS_H

#include "bcbase.h"
#include "mainwindow.inc"
#include "tracks.inc"

class TrackCanvas : public BC_Canvas
{
public:
	TrackCanvas(MainWindow *mwindow, int x, int y, int w, int h);
	~TrackCanvas();

// event handlers
	int button_press();
	int button_release();
	int cursor_motion();
	int draw_playback_cursor(int pixel, int flash = 1);
	int draw_loop_point(long position, int flash);

	int draw_floating_handle(int flash);

	MainWindow *mwindow;
private:
	long align_to_frames(long sample);
	int end_translation();

// ====================================== cursor selection type
	int auto_selected;               // 1 if automation selected
	int translate_selected;          // 1 if video translation selected

	int handle_selected;       // if a handle is selected
								// 1 if not floating yet
								// 2 if floating
	int which_handle;           // 1 left or 2 right handle
	long handle_oldposition;       // original position of handle
	long handle_position;           // current position of handle
	int handle_pixel;                // original pixel position of pointer in window
	int handle_mode;   // Determined by which button was pressed

	int current_end;       // end of selection 1 left 2 right
	long selection_midpoint1, selection_midpoint2;        // division between current ends
	int region_selected;         // 1 if region selected
	int selection_type;  // Whether an edit or a sample is selected

	int auto_reposition(int &cursor_x, int &cursor_y, long cursor_position);
	int update_selection(long cursor_position);
	int update_handle_selection(long cursor_position);
	int end_handle_selection();
};

#endif
