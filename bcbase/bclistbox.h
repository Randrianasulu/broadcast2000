#ifndef LISTBOX_H
#define LISTBOX_H

class BC_ListBox;

#include "bccolors.h"
#include "bcscrollbar.h"
#include "bctool.h"
#include "bcwindow.inc"

class BC_ListBoxYScroll : public BC_YScrollBar
{
public:
	BC_ListBoxYScroll(BC_ListBox *listbox,  
	                  int total_height,
					  int view_height,
	                  int position);
	int handle_event();
	
	BC_ListBox *listbox;
};

class BC_ListBoxXScroll : public BC_XScrollBar
{
public:
	BC_ListBoxXScroll(BC_ListBox *listbox, 
	                  int total_width, 
					  int view_width,
	                  int position);
	int handle_event();
	
	BC_ListBox *listbox;
};

class BC_ListBoxItem
{
public:
	BC_ListBoxItem();
	BC_ListBoxItem(char *text, int color = BLACK);
	virtual ~BC_ListBoxItem();

	friend BC_ListBox;

	BC_ListBoxItem& operator=(BC_ListBoxItem& item);
	int set_text(char *text);
	char *text;
	int color;

private:
	int y;		// y position in listbox relative to top.
				// This is determined in the drawing routine.
};

class BC_ListBox : public BC_Tool
{
public:
// Content can be specified here or with set_contents.
	BC_ListBox(int x, int y, int w, int h, 
						ArrayList<BC_ListBoxItem*> *data = 0, // Each column has an ArrayList of BC_ListBoxItems.
						const char **column_titles = 0, // Titles for columns.  Set to 0 for no titles
						int columns = 0, // Total columns.
						int yposition = 0, // Pixel of top of window.
						int currentitem = -1); // Current selected item.
	virtual ~BC_ListBox();

	friend BC_ListBoxXScroll;
	friend BC_ListBoxYScroll;

// user event handler for new selections
	virtual int selection_changed() { return 0; };

// Put text of current selection in the string.
	int get_selection(char *string, int column = 0);
// Get pointer to the text of the selection.
	const char *get_selection(int column = 0);
// Get the number of the item selected or -1 if no selection.
	int get_selection_number();
	int get_yposition();
// change window size and content
	int resize(int x, int y, int w, int h, 
						ArrayList<BC_ListBoxItem*> *data,
						const char **column_titles,
						int columns,
						int yposition = 0,
						int currentitem = -1);
// change the window size
	int set_size(int x, int y, int w, int h);
// change the contents
	int set_contents(ArrayList<BC_ListBoxItem*> *data,
						const char **column_titles,
						int columns,
						int yposition = 0,
						int currentitem = -1);
// Set the selection to the number.
	int set_selection(int selection);
// force highlighting when inactive
	int stay_highlighted();

// ============================= called by BC_Tool.
	int create_tool_objects();
	int cursor_left_();
	int keypress_event_();
	int button_press_();
	int button_release_();
	int cursor_motion_();
	int set_current_item(int currentitem);
	int resize(int w, int h);
	int draw();
	int activate_();
	int deactivate_();
	int unhighlight_();

// crude search engine
	int query_list();
	int query_list(char *regexp);
	int set_query(char *new_query);
	int reset_query();
	int motion_update();

private:
	int get_totallines();        // Total lines of text.
	int get_total_width();     // Total pixels.
	int get_total_height();     // Total pixels.
	int get_titleheight();     // pixels taken up by titles
	int get_visible_height();
	int create_column_width();      // create *column_width
	int get_column_width(int column); // get width of a single column from *column_width
	int fix_item_y();       // fix y positions on all items
	int fix_scrollbars();   // Decide what scrollbars are needed.  Delete if necessary.
// List box creates two scrollbars itself as needed.
	BC_ListBoxXScroll *xscrollbar;
	BC_ListBoxYScroll *yscrollbar;

	ArrayList<BC_ListBoxItem*> *data;  // Array of lists of pointers to list box items.
	const char **column_titles;      // Array of text strings.
	int *column_width;        // Array of numbers
	int columns;              // Total members in each array.
	char query[1024];
	int deactivates_highlight;   // stay highlighted when inactive
	int query_x;
	int highlighted;  // item cursor is over
	int currentitem;  // item currently highlighted
	int buttondown;   // button currently down
	int yposition;    // pixel of top of display relative to top of list
	int xposition;    // pixel of left display relative to first column
	int itemheight;   // height of a row in a list
	int text_descent;   // font size parameter
};

#endif
