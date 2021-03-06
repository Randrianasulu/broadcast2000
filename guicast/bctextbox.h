#ifndef BCTEXTBOX_H
#define BCTEXTBOX_H

#include "bclistbox.h"
#include "bcsubwindow.h"
#include "bctumble.h"
#include "fonts.h"

#define BCCURSORW 2



class BC_TextBox : public BC_SubWindow
{
public:
	BC_TextBox(int x, int y, int w, int rows, char *text, int has_border = 1, int font = MEDIUMFONT);
	BC_TextBox(int x, int y, int w, int rows, long text, int has_border = 1, int font = MEDIUMFONT);
	BC_TextBox(int x, int y, int w, int rows, int text, int has_border = 1, int font = MEDIUMFONT);
	BC_TextBox(int x, int y, int w, int rows, float text, int has_border = 1, int font = MEDIUMFONT);
	virtual ~BC_TextBox();

	virtual int handle_event() { return 0; };
	int update(char *text);
	int update(long value);
	int update(float value);
	void disable();
	void enable();

	int initialize();
	int cursor_enter_event();
	int cursor_leave_event();
	int cursor_motion_event();
	int button_press_event();
	int button_release_event();
	int repeat_event(long repeat_id);
	int keypress_event();
	int activate();
	int deactivate();
	char* get_text();
	int reposition_window(int x, int y, int w = -1, int rows = -1);
	int uses_text();

private:
	int reset_parameters(int rows, int has_border, int font);
	void draw();
	void draw_border();
	void draw_cursor();
	void delete_selection(int letter1, int letter2, int text_len);
	void insert_text(char *string);
	void get_ibeam_position(int &x, int &y);
	void find_ibeam();
	void select_word(int &letter1, int &letter2, int ibeam_letter);
	int get_cursor_letter(int cursor_x, int cursor_y);
	int get_row_h(int rows);


// Top left of text relative to window
	int text_x, text_y;
// Top left of cursor relative to window
	int ibeam_x, ibeam_y;

	int ibeam_letter;
	int highlight_letter1, highlight_letter2;
	int highlight_letter3, highlight_letter4;
	int text_x1, text_start, text_end, text_selected, word_selected;
	int text_ascent, text_descent, text_height;
	int left_margin, right_margin, top_margin, bottom_margin;
	int has_border;
	int font;
	int rows;
	int highlighted;
	int high_color, back_color;
	int background_color;
	char text[BCTEXTLEN], text_row[BCTEXTLEN], temp_string[2];
	int active;
	long repeat_id;
	int enabled;
};

class BC_PopupTextBoxText;
class BC_PopupTextBoxList;

class BC_PopupTextBox
{
public:
	BC_PopupTextBox(BC_WindowBase *parent_window, 
		ArrayList<BC_ListBoxItem*> *list_items,
		char *default_text,
		int x, 
		int y, 
		int text_w,
		int list_h);
	virtual ~BC_PopupTextBox();
	int create_objects();
	virtual int handle_event();
	char* get_text();
	int get_number();
	int get_w();
	int get_h();
	void update(char *text);
	void update_list(ArrayList<BC_ListBoxItem*> *data);

	friend BC_PopupTextBoxText;
	friend BC_PopupTextBoxList;

private:
	int x, y, text_w, list_h;
	char *default_text;
	ArrayList<BC_ListBoxItem*> *list_items;
	BC_PopupTextBoxText *textbox;
	BC_PopupTextBoxList *listbox;
	BC_WindowBase *parent_window;
};

class BC_PopupTextBoxText : public BC_TextBox
{
public:
	BC_PopupTextBoxText(BC_PopupTextBox *popup, int x, int y);
	int handle_event();
	BC_PopupTextBox *popup;
};

class BC_PopupTextBoxList : public BC_ListBox
{
public:
	BC_PopupTextBoxList(BC_PopupTextBox *popup, int x, int y);
	int handle_event();
	BC_PopupTextBox *popup;
};


class BC_TumbleTextBoxText;
class BC_TumbleTextBoxTumble;

class BC_TumbleTextBox
{
public:
	BC_TumbleTextBox(BC_WindowBase *parent_window, 
		long default_value,
		long min,
		long max,
		int x, 
		int y, 
		int text_w);
	virtual ~BC_TumbleTextBox();
	int create_objects();
	virtual int handle_event();
	char* get_text();
	int get_w();
	int get_h();
	void set_boundaries(long min, long max);
	
	friend BC_TumbleTextBoxText;
	friend BC_TumbleTextBoxTumble;

private:
	int x, y, text_w;
	long default_value, min, max;
	BC_TumbleTextBoxText *textbox;
	BC_TumbleTextBoxTumble *tumbler;
	BC_WindowBase *parent_window;
};

class BC_TumbleTextBoxText : public BC_TextBox
{
public:
	BC_TumbleTextBoxText(BC_TumbleTextBox *popup, int x, int y);
	int handle_event();
	BC_TumbleTextBox *popup;
};

class BC_TumbleTextBoxTumble : public BC_ITumbler
{
public:
	BC_TumbleTextBoxTumble(BC_TumbleTextBox *popup, 
		long min,
		long max,
		int x, 
		int y);
	
	BC_TumbleTextBox *popup;
};


#endif
