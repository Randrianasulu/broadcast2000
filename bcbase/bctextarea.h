#ifndef TEXTAREA_H
#define TEXTAREA_H

class BC_TextArea;

#include "bcscrollbar.h"
#include "bctool.h"
#include <string.h>

class BC_TextAreaScroll : public BC_YScrollBar
{
public:
	BC_TextAreaScroll(BC_TextArea *textarea, int totallines_, int yposition);
	int handle_event();

	BC_TextArea *textarea;
};

class BC_TextArea : public BC_Tool
{
public:
	BC_TextArea(int x_, int y_, int w_, int h_, char *text, int yposition = 0);
	virtual ~BC_TextArea();
	int create_tool_objects();

// ================================ user commands

	int set_size(int x_, int y_, int w_, int h_);
	int set_contents(char *text, int yposition = 0);
	int append_contents(char *new_text);
	int set_yposition(int yposition);
	int get_yposition();






// ================================ tool event handlers

	int resize(int w, int h);
	int draw();
	int deactivate();
	int button_press_();
	int cursor_left_();
	int unhighlight_();
	int get_total_lines();
	int cursor_motion_();
	int button_release_();


	BC_TextAreaScroll *scrollbar;
	char *text;
	int yposition;  // item of top line in list display
	int itemheight;   // height of a row in a list
	int button_down;    // scroll by pointing
	int highlighted;
};

#endif
