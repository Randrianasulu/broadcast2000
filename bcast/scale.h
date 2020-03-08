#ifndef SCALE_H
#define SCALE_H

#include "bcbase.h"
#include "mainwindow.inc"
#include "scale.inc"

class Scale : public BC_MenuItem
{
public:
	Scale(MainWindow *mwindow);
	~Scale();
	
	int handle_event();
	
	ScaleThread *thread;
	MainWindow *mwindow;
};

class ScaleWindow;

class ScaleThread : public Thread
{
public:
	ScaleThread(MainWindow *mwindow);
	~ScaleThread();

	void run();

	int update_window(int offset_updated = 0);
	int update_aspect(ScaleWindow *window);

// change a value to negative to signal it was the value changed
	int dimension[4];        // new dimensions to apply
	int orig_dimension[4];   // dimensions for getting scale
	int offsets[4];          // Offsets of new images in old images
	float ratio[4];
	float aspect_w, aspect_h;
	int constrain_ratio;
	int scale_data;
	int auto_aspect;
	int already_running;

	MainWindow *mwindow;
	ScaleWindow *window;
};

class ScaleSizeText;
class ScaleRatioText;
class ScalePosition;
class ScaleOffsetText;
class ScaleAspectW;
class ScaleAspectH;

class ScaleWindow : public BC_Window
{
public:
	ScaleWindow(ScaleThread *thread);
	~ScaleWindow();
	
	int create_objects();
	ScaleThread *thread;
	ScaleSizeText *dimension[4];
	ScaleRatioText *ratio[4];
//	ScaleOffsetText *offsets[4];
	ScaleAspectW *aspect_w;
	ScaleAspectH *aspect_h;
//	ScalePosition *position1, *position2;
};

class ScaleSizeText : public BC_TextBox
{
public:
	ScaleSizeText(int x, int y, ScaleThread *thread, int *output);
	~ScaleSizeText();
	int handle_event();
	ScaleThread *thread;
	int *output;
};


class ScaleOffsetText : public BC_TextBox
{
public:
	ScaleOffsetText(int x, int y, ScaleThread *thread, int *output);
	~ScaleOffsetText();
	int handle_event();
	ScaleThread *thread;
	int *output;
};

class ScaleRatioText : public BC_TextBox
{
public:
	ScaleRatioText(int x, int y, ScaleThread *thread, float *output);
	~ScaleRatioText();
	int handle_event();
	ScaleThread *thread;
	float *output;
};

class ScaleAspectAuto : public BC_CheckBox
{
public:
	ScaleAspectAuto(int x, int y, ScaleThread *thread);
	~ScaleAspectAuto();
	int handle_event();
	ScaleThread *thread;
};

class ScaleAspectW : public BC_TextBox
{
public:
	ScaleAspectW(int x, int y, ScaleThread *thread, float *output, char *string);
	~ScaleAspectW();
	int handle_event();
	ScaleThread *thread;
	float *output;
};

class ScaleAspectH : public BC_TextBox
{
public:
	ScaleAspectH(int x, int y, ScaleThread *thread, float *output, char *string);
	~ScaleAspectH();
	int handle_event();
	ScaleThread *thread;
	float *output;
};


class ScaleData : public BC_CheckBox
{
public:
	ScaleData(int x, int y, ScaleThread *thread);
	~ScaleData();
	int handle_event();
	ScaleThread *thread;
};

class ScaleConstrain : public BC_CheckBox
{
public:
	ScaleConstrain(int x, int y, ScaleThread *thread);
	~ScaleConstrain();
	int handle_event();
	ScaleThread *thread;
};

class ScaleOK : public BC_BigButton
{
public:
	ScaleOK(int x, int y, ScaleWindow *window);
	~ScaleOK();
	int handle_event();
	int keypress_event();
	
	ScaleWindow *window;
};

class ScaleCancel : public BC_BigButton
{
public:
	ScaleCancel(int x, int y, ScaleWindow *window);
	~ScaleCancel();
	int handle_event();
	int keypress_event();
	
	ScaleWindow *window;
};

class ScalePosition : public BC_Canvas
{
public:
	ScalePosition(int x, int y, ScaleThread *thread, ScaleWindow *window, 
		int *orig_dimension, int *scale_dimension, int *offsets);

	~ScalePosition();

	int draw();
	int button_press();
	int cursor_motion();
	int button_release();
	int get_scale();

	int *orig_dimension;
	int *scale_dimension;
	int *offsets;
	ScaleThread *thread;
	ScaleWindow *window;
	int button_down;
	int center_x, center_y, offset_x, offset_y;
	float hscale, vscale;
};



#endif
