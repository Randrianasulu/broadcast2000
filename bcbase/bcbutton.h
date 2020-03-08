#ifndef BCBUTTON_H
#define BCBUTTON_H

class BC_Button;

#include "bccolors.h"
#include "bctool.h"
#include "bcwindow.h"

class BC_Button : public BC_Tool
{
public:
	BC_Button(int x, int y, char *text, int big);
	BC_Button(int x, int y, int w, char *text, int big);
	virtual ~BC_Button();
	int create_tool_objects();
	int resize_tool(int x, int y);

	int cursor_left_();
	int button_press_();
	int cursor_motion_();
	int button_release_();
	int keypress_event_();
	int unhighlight_();

	int repeat_();      // must return 0 to avoid trapping
	virtual int draw() = 0;              // defined in button class
	int resize(int x, int y);
	int update(char *text);
	char* get_text();
	int get_down();
	int set_down(int new_value);
	int get_highlighted();
	int is_big();      // if the button has a 2 pixel bevel or a 1 pixel bevel
	int draw_small_box();

private:
	char *text;
	int highlighted;
	int down, button_down, cursor_over;
	int big;
};

// Up triangle
class BC_UpTriangleButton : public BC_Button  // zoom out button
{
public:
	BC_UpTriangleButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
};

// Down triangle
class BC_DownTriangleButton : public BC_Button  // zoom in button
{
public:
	BC_DownTriangleButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
};

class BC_LeftTriangleButton : public BC_Button  // zoom out button
{
public:
	BC_LeftTriangleButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
};

class BC_RightTriangleButton : public BC_Button  // zoom in button
{
public:
	BC_RightTriangleButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
};

// Small button
class BC_SmallButton : public BC_Button     // small button
{
public:
	BC_SmallButton(int x, int y, char *text, int big = 0);
	BC_SmallButton(int x, int y, int w, char *text, int big = 0);
	int create_tool_objects();
	int draw();
};

// Big button
class BC_BigButton : public BC_Button      // big button
{
public:
	BC_BigButton(int x, int y, char *text, int big = 1);
	int create_tool_objects();
	int draw();
};

// Record button
class BC_RecButton : public BC_Button     // record button
{
public:
	BC_RecButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
};

// Forward Record 1 frame
class BC_FrameRecButton : public BC_Button
{
public:
	BC_FrameRecButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
};

// Duplex button
class BC_DuplexButton : public BC_Button      // play button
{
public:
	BC_DuplexButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
};

// Fast Rewind button
class BC_FastRewindButton : public BC_Button      // Fast rewind button
{
public:
	BC_FastRewindButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
	int update();      // toggle between on and off
	
	int is_play;
};


// Stop button
class BC_StopButton : public BC_Button        // stop button
{
public:
	BC_StopButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
};

// Rewind button
class BC_RewindButton : public BC_Button        // Rewind button
{
public:
	BC_RewindButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
};

// End button
class BC_EndButton : public BC_Button        // End button
{
public:
	BC_EndButton(int x, int y, int w, int h, int big = 0);
	int create_tool_objects();
	int draw();
};

// =================================== playback buttons

// Playback button base class
// User shouldn't use this
class BC_PlayButton : public BC_Button      // play button
{
public:
	BC_PlayButton(int x, int y, int w, int h, int big = 0);
	
	int update(int value);      // toggle between play, pause, and paused
								// 0 - paused   1 - play    2 - pause
	
	int create_tool_objects();
	int draw();
	virtual int draw_polygon() { return 0; };
	int set_orange(int value);
	int reset_button();
	int draw_pause();
	int get_mode();
	
	int mode;       // 0 - paused   1 - play    2 - pause
	int orange;
};

// Forward Play
// User should use these for play buttons.
class BC_ForwardButton : public BC_PlayButton      // play button
{
public:
	BC_ForwardButton(int x, int y, int w, int h, int big = 0);
	int draw_polygon();
};

// Forward Play 1 frame
// User should use these for play buttons.
class BC_FrameForwardButton : public BC_PlayButton      // play button
{
public:
	BC_FrameForwardButton(int x, int y, int w, int h, int big = 0);
	int draw_polygon();
};

// Fast Playback button
class BC_FastForwardButton : public BC_PlayButton      // Fast play button
{
public:
	BC_FastForwardButton(int x, int y, int w, int h, int big = 0);
	int draw_polygon();
};

// Reverse playback button
class BC_ReverseButton : public BC_PlayButton      // play button
{
public:
	BC_ReverseButton(int x, int y, int w, int h, int big = 0);
	int draw_polygon();
};

// Reverse playback button 1 frame
class BC_FrameReverseButton : public BC_PlayButton      // play button
{
public:
	BC_FrameReverseButton(int x, int y, int w, int h, int big = 0);
	int draw_polygon();
};

// Fast Reverse Playback button
class BC_FastReverseButton : public BC_PlayButton      // Fast play button
{
public:
	BC_FastReverseButton(int x, int y, int w, int h, int big = 0);
	int draw_polygon();
};


#endif
