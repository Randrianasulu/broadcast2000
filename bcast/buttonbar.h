#ifndef BUTTONBAR_H
#define BUTTONBAR_H

class ExpandX;
class ZoomX;
class ExpandY;
class ZoomY;
class ExpandTrack;
class ZoomTrack;
class ExpandVideo;
class ZoomVideo;
class Fit;
class LabelButton;
class Cut;
class Copy;
class Paste;
class PlayButton;
class FramePlayButton;
class ReverseButton;
class FrameReverseButton;
class FastReverseButton;
class FastPlayButton;
class RewindButton;
class StopButton;
class EndButton;

#include "bcbase.h"
#include "buttonbar.inc"
#include "mainwindow.inc"
#include "mainwindowgui.inc"
#include "record.inc"


class ButtonBar : public BC_SubWindow
{
public:
	ButtonBar(MainWindowGUI *gui, int x, int y, int w, int h);
	~ButtonBar();

	int create_objects();
	int resize_event(int w, int h);
	int flip_vertical(int w, int h);
	int keypress_event();
	int start_playback(BC_PlayButton *button, int reverse, float speed);
	int pause_playback();
	int resume_playback();
	int handle_transport(BC_PlayButton *button, int reverse, float speed); // execute transport commands
	int transport_keys(int key);  // translate keypresses into transport commands
	int reset_transport();
	int pause_transport(); // After frame advance

	MainWindowGUI *gui;
	MainWindow *mwindow;

	PlayButton *forward_play;
	FramePlayButton *frame_forward_play;
	ReverseButton *reverse_play;
	FrameReverseButton *frame_reverse_play;
	FastReverseButton *fast_reverse;
	FastPlayButton *fast_play;

// playback parameters
	int reverse;
	float speed;
	BC_PlayButton *active_button;    // button to update when pausing or resuming playback

	Record *record_button;

	RewindButton *rewind_button;
	StopButton *stop_button;
	EndButton *end_button;

	BC_Title *x_title;
	ExpandX *expand_x_button;
	ZoomX *zoom_x_button;

	BC_Title *y_title;
	ExpandY *expand_y_button;
	ZoomY *zoom_y_button;

	BC_Title *t_title;
	ExpandTrack *expand_t_button;
	ZoomTrack *zoom_t_button;

	BC_Title *v_title;
	ExpandVideo *expand_v_button;
	ZoomVideo *zoom_v_button;

	Fit *fit_button;
	LabelButton *label_button;
	Cut *cut_button;
	Copy *copy_button;
	Paste *paste_button;
};

class RecButton : public BC_RecButton
{
public:
	RecButton(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class RewindButton : public BC_RewindButton
{
public:
	RewindButton(int x, int y, MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

// playback

class FastReverseButton : public BC_FastReverseButton
{
public:
	FastReverseButton(MainWindow *mwindow, ButtonBar *bar, int x, int y);
	int handle_event();
	int button_press();
	int button_release();
	MainWindow *mwindow;
	ButtonBar *bar;
	int previous_mode;
};

class ReverseButton : public BC_ReverseButton
{
public:
	ReverseButton(MainWindow *mwindow, ButtonBar *bar, int x, int y);
	int handle_event();
	MainWindow *mwindow;
	ButtonBar *bar;
	int previous_mode;
};

class FrameReverseButton : public BC_FrameReverseButton
{
public:
	FrameReverseButton(MainWindow *mwindow, ButtonBar *bar, int x, int y);
	int handle_event();
	MainWindow *mwindow;
	ButtonBar *bar;
	int previous_mode;
};

class PlayButton : public BC_ForwardButton
{
public:
	PlayButton(MainWindow *mwindow, ButtonBar *bar, int x, int y);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
	ButtonBar *bar;
	int previous_mode;
};

class FramePlayButton : public BC_FrameForwardButton
{
public:
	FramePlayButton(MainWindow *mwindow, ButtonBar *bar, int x, int y);
	int handle_event();
	MainWindow *mwindow;
	ButtonBar *bar;
	int previous_mode;
};

class FastPlayButton : public BC_FastForwardButton
{
public:
	FastPlayButton(MainWindow *mwindow, ButtonBar *bar, int x, int y);
	int handle_event();
	int button_press();
	int button_release();
	MainWindow *mwindow;
	ButtonBar *bar;
	int previous_mode;
};



class EndButton : public BC_EndButton
{
public:
	EndButton(int x, int y, MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class StopButton : public BC_StopButton
{
public:
	StopButton(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class ExpandX : public BC_UpTriangleButton
{
public:
	ExpandX(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class ZoomX : public BC_DownTriangleButton
{
public:
	ZoomX(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class ExpandY : public BC_UpTriangleButton
{
public:
	ExpandY(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class ZoomY : public BC_DownTriangleButton
{
public:
	ZoomY(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class ExpandTrack : public BC_UpTriangleButton
{
public:
	ExpandTrack(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class ZoomTrack : public BC_DownTriangleButton
{
public:
	ZoomTrack(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class ExpandVideo : public BC_UpTriangleButton
{
public:
	ExpandVideo(int x, int y, MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class ZoomVideo : public BC_DownTriangleButton
{
public:
	ZoomVideo(int x, int y, MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class Fit : public BC_SmallButton
{
public:
	Fit(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class LabelButton : public BC_SmallButton
{
public:
	LabelButton(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class Cut : public BC_SmallButton
{
public:
	Cut(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class Copy : public BC_SmallButton
{
public:
	Copy(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

class Paste : public BC_SmallButton
{
public:
	Paste(int x, int y, MainWindow *mwindow);
	int handle_event();
	int keypress_event();
	MainWindow *mwindow;
};

#endif
