#ifndef BCTEXTBOX_H
#define BCTEXTBOX_H

#include "bccolors.h"
#include "bctool.h"
#include "bcwindow.inc"

#define BC_TEXTBOXMAX 1024

class BC_TextBox : public BC_Tool
{
public:
	BC_TextBox(int x, int y, int w, char *text, int has_border = 1);
	BC_TextBox(int x, int y, int w, int text, int has_border = 1);
	BC_TextBox(int x, int y, int w, float text, int has_border = 1);

// ====================== user commands

	int update(char *text_);               // set text to a string
	int update(int value);                 // set text to a number
	int update(float value);                 // set text to a number
	char* get_text();                  // get the string contained in the textbox
	int resize_tool(int x, int y);

// ====================== tool commands
	int create_tool_objects();

	int cursor_left_();
	int button_press_();
	int cursor_motion_();
	int button_release_();
	int keypress_event_();
	int unhighlight_();
// activate this textbox
	int activate_();
// deactivate this textbox
	int deactivate_();
	int uses_text();       // obsoleted by event trapping

	int draw();
// put new text in box and set up the selection
	
// redraw the textbox
	int update();
// deactivate all the text boxes
	int deactivate_all();
	int highlighted;

private:
	int delete_selection();
	int insert_text(char *string);       // Insert text at the insertion point.
	int copy_text();               // copy the current selection into the cut buffer
	int charof(int x);
	char text[BC_TEXTBOXMAX];

	int x1, x2, x1_, x2_, center_, start, selecttext, selectword;
	int has_border;
	int text_color, back_color, high_color;
	int text_ascent;
	int center, wordx1, wordx2;
	int last_button;  // Last button pressed for motion events.
};

#endif
