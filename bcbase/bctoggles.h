#ifndef BCTOGGLES_H
#define BCTOGGLES_H

#include "bccolors.h"
#include "bctool.h"
#include "bcwindow.h"

class BC_Toggle : public BC_Tool // Base class
{
public:
	BC_Toggle(int x_, int y_, int w_, int h_, int down, const char *text = 0);
	int create_tool_objects();

	// update status
	int resize(int x_, int y_, int w_, int h_, int down);
	int resize_tool(int x, int y);
	int update(int down_);
	int get_value();
	int draw_disc();

// ============================== user event handlers	
	virtual int draw() { return 0; };
	virtual int cursor_moved_over() { return 0; };
	virtual int button_press() { return 0; };
	virtual int button_release() { return 0; };

// ================================ tool event handlers
	int cursor_left_();
	int button_press_();
	int cursor_motion_();
	int button_release_();
	int unhighlight_();

	int highlighted;
	int down;
	const char *text;
};

class BC_Radial : public BC_Toggle
{
public:
	BC_Radial(int x_, int y_, int w_, int h_, int down, const char *text = 0);
	int create_tool_objects();

	int draw();
};


// Specify a string for *text to print a string next to the box.
// Specify a single ascii character for letter to have a letter
// printed in the box instead of a check.
class BC_CheckBox : public BC_Toggle
{
public:
	BC_CheckBox(int x_, int y_, int w_, int h_, int down, const char *text = 0, char letter = 0);
	int create_tool_objects();

	int draw();
	char letter;	
};

class BC_RecordPatch : public BC_Toggle
{
public:
	BC_RecordPatch(int x_, int y_, int w_, int h_, int down);
	int create_tool_objects();

	int draw();
};

class BC_PlayPatch : public BC_Toggle
{
public:
	BC_PlayPatch(int x_, int y_, int w_, int h_, int down);
	int create_tool_objects();

	int draw();
};

class BC_Label : public BC_Toggle
{
public:
	BC_Label(int x_, int y_, int w_, int h_, int down);
	int create_tool_objects();

	int set_status(int down_);
	int draw();
};

#endif
