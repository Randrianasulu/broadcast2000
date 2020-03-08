#include "bcresources.h"
#include "bctextbox.h"
#include "colors.h"
#include <ctype.h>
#include "cursors.h"
#include "keys.h"

#include <string.h>

BC_TextBox::BC_TextBox(int x, int y, int w, int rows, char *text, int has_border, int font)
 : BC_SubWindow(x, y, w, 0, -1)
{
	reset_parameters(rows, has_border, font);
	strcpy(this->text, text);
}

BC_TextBox::BC_TextBox(int x, int y, int w, int rows, long text, int has_border, int font)
 : BC_SubWindow(x, y, w, 0, -1)
{
	reset_parameters(rows, has_border, font);
	sprintf(this->text, "%ld", text);
}

BC_TextBox::BC_TextBox(int x, int y, int w, int rows, float text, int has_border, int font)
 : BC_SubWindow(x, y, w, 0, -1)
{
	reset_parameters(rows, has_border, font);
	sprintf(this->text, "%0.2f", text);
}

BC_TextBox::BC_TextBox(int x, int y, int w, int rows, int text, int has_border, int font)
 : BC_SubWindow(x, y, w, 0, -1)
{
	reset_parameters(rows, has_border, font);
	sprintf(this->text, "%d", text);
}

BC_TextBox::~BC_TextBox()
{
}

int BC_TextBox::reset_parameters(int rows, int has_border, int font)
{
	this->rows = rows;
	this->has_border = has_border;
	this->font = font;
	text_start = 0;
	text_end = 0;
	highlight_letter1 = highlight_letter2 = 0;
	highlight_letter3 = highlight_letter4 = 0;
	ibeam_letter = 0;
	active = 0;
	text_selected = word_selected = 0;
	text_x = 0;
	enabled = 1;
	return 0;
}

int BC_TextBox::initialize()
{
// Get dimensions
	text_ascent = get_text_ascent(font) + 1;
	text_descent = get_text_descent(font) + 1;
	text_height = text_ascent + text_descent;
	ibeam_letter = strlen(text);
	if(has_border)
	{ 
		left_margin = right_margin = 4;
		top_margin = bottom_margin = 2;
	}
	else 
	{ 
		left_margin = right_margin = 2;
		top_margin = bottom_margin = 0;
	}
	h = get_row_h(rows);
	text_x = left_margin;
	find_ibeam();

// Create the subwindow
	BC_SubWindow::initialize();

	if(has_border)
	{
		back_color = WHITE;
		high_color = LTYELLOW;
	}
	else 
	{
		high_color = LTGREY;
		back_color = bg_color;
	}

	draw();
	set_cursor(IBEAM_CURSOR);
	return 0;
}

int BC_TextBox::update(char *text)
{
	int text_len = strlen(text);
	strcpy(this->text, text);
	if(highlight_letter1 > text_len) highlight_letter1 = text_len;
	if(highlight_letter2 > text_len) highlight_letter2 = text_len;
	ibeam_letter = text_len;
	draw();
	return 0;
}

int BC_TextBox::update(long value)
{
	sprintf(this->text, "%ld", value);
	int text_len = strlen(text);
	if(highlight_letter1 > text_len) highlight_letter1 = text_len;
	if(highlight_letter2 > text_len) highlight_letter2 = text_len;
	ibeam_letter = text_len;
	draw();
	return 0;
}

int BC_TextBox::update(float value)
{
	sprintf(this->text, "%0.2f", value);
	int text_len = strlen(text);
	if(highlight_letter1 > text_len) highlight_letter1 = text_len;
	if(highlight_letter2 > text_len) highlight_letter2 = text_len;
	ibeam_letter = text_len;
	draw();
	return 0;
}

void BC_TextBox::disable()
{
	if(enabled)
	{
		enabled = 0;
		if(active) top_level->deactivate();
		draw();
	}
}

void BC_TextBox::enable()
{
	if(!enabled)
	{
		enabled = 1;
		draw();
	}
}

char* BC_TextBox::get_text()
{
	return text;
}

int BC_TextBox::get_row_h(int rows)
{
	return rows * text_height + top_margin + bottom_margin;
}

int BC_TextBox::reposition_window(int x, int y, int w, int rows)
{
	int new_h = get_h();
	if(rows != -1)
	{
		new_h = get_row_h(rows);
		this->rows = rows;
	}
	BC_WindowBase::reposition_window(x, y, w, new_h);
	draw();
	return 0;
}

void BC_TextBox::draw_border()
{
// Clear margins
	set_color(background_color);
	draw_box(0, 0, left_margin, get_h());
	draw_box(get_w() - right_margin, 0, right_margin, get_h());

	if(has_border)
	{
		if(highlighted)
			draw_3d_border(0, 0, w, h,
				top_level->get_resources()->button_shadow, 
				RED, 
				LTPINK,
				top_level->get_resources()->button_light);
		else
			draw_3d_border(0, 0, w, h, 
				top_level->get_resources()->button_shadow, 
				BLACK, 
				top_level->get_resources()->button_up,
				top_level->get_resources()->button_light);
	}
}

void BC_TextBox::draw_cursor()
{
	set_color(background_color);
	set_inverse();
	draw_box(ibeam_x, ibeam_y, BCCURSORW, text_height);
	set_opaque();
}


void BC_TextBox::draw()
{
	int i, j, k, text_len;
	int row_begin, row_end;
	int highlight_x1, highlight_x2;
	int need_ibeam = 1;

// Background
	if(has_border)
	{
		background_color = WHITE;
	}
	else
	{
		if(highlighted)
		{
			background_color = high_color;
		}
		else
		{
			background_color = back_color;
		}
	}

	set_color(background_color);
	draw_box(0, 0, w, h);

// Draw text with selection
	set_font(font);
	text_len = strlen(text);
	for(i = 0, k = text_y; i < text_len && k < get_h(); k += text_height)
	{
// Draw row of text
		row_begin = i;
		for(j = 0; text[i] != '\n' && i < text_len; j++, i++)
		{
			text_row[j] = text[i];
		}
		row_end = i;

		text_row[j] = 0;

		if(k > -text_height + top_margin && k < get_h() - bottom_margin)
		{
// Draw highlighted region of row
			if(highlight_letter2 > highlight_letter1 &&
				((row_begin <= highlight_letter1 && row_end >= highlight_letter2) ||
				 (row_begin < highlight_letter2 && row_end >= highlight_letter2) ||
				 (row_begin <= highlight_letter1 && row_end > highlight_letter1)))
			{
				if(active && enabled)
					set_color(top_level->get_resources()->text_highlight);
				else
					set_color(MEGREY);

				if(highlight_letter1 >= row_begin && highlight_letter1 < row_end)
					highlight_x1 = get_text_width(font, text_row, highlight_letter1 - row_begin);
				else
					highlight_x1 = 0;

				if(highlight_letter2 > row_begin && highlight_letter2 <= row_end)
					highlight_x2 = get_text_width(font, text_row, highlight_letter2 - row_begin);
				else
					highlight_x2 = get_w();

				draw_box(highlight_x1 + text_x, 
					k, 
					highlight_x2 - highlight_x1, 
					text_height);
			}

// Draw text over highlight
			if(enabled)
				set_color(BLACK);
			else
				set_color(MEGREY);

			draw_text(text_x, k + text_ascent, text_row);
// Get ibeam location
			if(ibeam_letter >= row_begin || ibeam_letter <= row_end)
			{
				need_ibeam = 0;
				ibeam_y = k;
				ibeam_x = text_x + get_text_width(font, text_row, ibeam_letter - row_begin);
			}
		}
	}

	if(need_ibeam)
	{
		ibeam_x = text_x;
		ibeam_y = text_y;
	}

// Draw solid cursor
	if(!active)
	{
		draw_cursor();
	}

// Border
	draw_border();
	flash();
}

int BC_TextBox::cursor_enter_event()
{
	if(top_level->event_win == win)
	{
		if(!highlighted)
		{
			highlighted = 1;
			draw_border();
			flash();
		}
	}
	return 0;
}

int BC_TextBox::cursor_leave_event()
{
	if(highlighted)
	{
		highlighted = 0;
		draw_border();
		flash();
	}
	return 0;
}

int BC_TextBox::button_press_event()
{
	int cursor_letter = 0;
	int text_len = strlen(text);

	if(!enabled) return 0;

	if(top_level->event_win == win)
	{
		if(!active)
		{
			top_level->deactivate();
			activate();
		}

		cursor_letter = get_cursor_letter(top_level->cursor_x, top_level->cursor_y);
		if(get_double_click())
		{
			word_selected = 1;
			select_word(highlight_letter1, highlight_letter2, cursor_letter);
			highlight_letter3 = highlight_letter1;
			highlight_letter4 = highlight_letter2;
			ibeam_letter = highlight_letter2;
		}
		else
		{
			text_selected = 1;
			highlight_letter3 = highlight_letter4 = 
				ibeam_letter = highlight_letter1 = 
				highlight_letter2 = cursor_letter;
		}
		
		if(ibeam_letter < 0) ibeam_letter = 0;
		if(ibeam_letter > text_len) ibeam_letter = text_len;
		draw();
		return 1;
	}
	else
	if(active)
	{
		top_level->deactivate();
	}

	return 0;
}

int BC_TextBox::button_release_event()
{
	if(active)
	{
		if(text_selected || word_selected)
		{
			text_selected = 0;
			word_selected = 0;
		}
	}
	return 0;
}

int BC_TextBox::cursor_motion_event()
{
	int cursor_letter, text_len = strlen(text), letter1, letter2;
	if(active)
	{
		if(text_selected || word_selected)
		{
			cursor_letter = get_cursor_letter(top_level->cursor_x, top_level->cursor_y);
			if(word_selected)
			{
				select_word(letter1, letter2, cursor_letter);
			}
			else
			if(text_selected)
			{
				letter1 = letter2 = cursor_letter;
			}

			if(letter1 <= highlight_letter3)
			{
				highlight_letter1 = letter1;
				highlight_letter2 = highlight_letter4;
				ibeam_letter = letter1;
			}
			else
			if(letter2 >= highlight_letter4)
			{
				highlight_letter2 = letter2;
				highlight_letter1 = highlight_letter3;
				ibeam_letter = letter2;
			}
			
			find_ibeam();
			draw();
			return 1;
		}
	}
	return 0;
}

int BC_TextBox::activate()
{
	top_level->active_subwindow = this;
	active = 1;
	draw();
	top_level->set_repeat(top_level->get_resources()->blink_rate);
	return 0;
}

int BC_TextBox::deactivate()
{
	active = 0;
	top_level->unset_repeat(top_level->get_resources()->blink_rate);
	draw();
	return 0;
}

int BC_TextBox::repeat_event(long duration)
{
	if(duration == top_level->get_resources()->blink_rate && 
		active)
	{
		draw_cursor();
		flash();
		return 1;
	}
	return 0;
}

int BC_TextBox::keypress_event()
{
// Result == 2 contents changed
// Result == 1 trapped keypress
// Result == 0 nothing
	int result = 0;
	int text_len;
	int dispatch_event = 0;

	if(!active || !enabled) return 0;

	text_len = strlen(text);
	switch(top_level->get_keypress())
	{
		case ESC:
			top_level->deactivate();
			result = 0;
			break;

		case RETURN:
			if(rows == 1)
			{
				top_level->deactivate();
				dispatch_event = 1;
				result = 0;
			}
			break;

		case TAB:
			top_level->cycle_textboxes(1);
			result = 1;
			break;

		case LEFTTAB:
			top_level->cycle_textboxes(-1);
			result = 1;
			break;

		case LEFT:
			if(ibeam_letter > 0)
			{
// Extend selection
				if(top_level->shift_down())
				{
// Initialize highlighting
					if(highlight_letter1 == highlight_letter2)
					{
						highlight_letter1 = ibeam_letter - 1;
						highlight_letter2 = ibeam_letter;
					}
					else
// Extend left highlight
					if(highlight_letter1 == ibeam_letter)
					{
						highlight_letter1--;
					}
					else
// Shrink right highlight
					if(highlight_letter2 == ibeam_letter)
					{
						highlight_letter2--;
					}
				}
				else
					highlight_letter1 = highlight_letter2;

				ibeam_letter--;

				find_ibeam();
				draw();
			}
			result = 1;
			break;

		case RIGHT:
			if(ibeam_letter < text_len)
			{
// Extend selection
				if(top_level->shift_down())
				{
// Initialize highlighting
					if(highlight_letter1 == highlight_letter2)
					{
						highlight_letter1 = ibeam_letter;
						highlight_letter2 = ibeam_letter + 1;
					}
					else
// Shrink left highlight
					if(highlight_letter1 == ibeam_letter)
					{
						highlight_letter1++;
					}
					else
// Expand right highlight
					if(highlight_letter2 == ibeam_letter)
					{
						highlight_letter2++;
					}
				}
				else
					highlight_letter1 = highlight_letter2;

				ibeam_letter++;

				find_ibeam();
				draw();
			}
			result = 1;
			break;
		
		case END:
			if(top_level->shift_down())
			{
				if(highlight_letter1 == highlight_letter2)
				{
					highlight_letter2 = text_len;
					highlight_letter1 = ibeam_letter;
				}
				else
				if(highlight_letter1 == ibeam_letter)
				{
					highlight_letter1 = highlight_letter2;
					highlight_letter2 = text_len;
				}
				else
				if(highlight_letter2 == ibeam_letter)
				{
					highlight_letter2 = text_len;
				}
			}
			else
				highlight_letter1 = highlight_letter2;

			ibeam_letter = text_len;
			find_ibeam();
			draw();
			result = 1;
			break;
		
		case HOME:
			if(top_level->shift_down())
			{
				if(highlight_letter1 == highlight_letter2)
				{
					highlight_letter2 = ibeam_letter;
					highlight_letter1 = 0;
				}
				else
				if(highlight_letter1 == ibeam_letter)
				{
					highlight_letter1 = 0;
				}
				else
				if(highlight_letter2 == ibeam_letter)
				{
					highlight_letter2 = highlight_letter1;
					highlight_letter1 = 0;
				}
			}
			else
				highlight_letter1 = highlight_letter2;

			ibeam_letter = 0;
			find_ibeam();
			draw();
			result = 1;
			break;

    	case BACKSPACE:
			if(highlight_letter1 == highlight_letter2)
			{
				if(ibeam_letter > 0)
				{
					delete_selection(ibeam_letter - 1, ibeam_letter, text_len);
					ibeam_letter--;
				}
			}
			else
			{
				delete_selection(highlight_letter1, highlight_letter2, text_len);
				highlight_letter2 = ibeam_letter = highlight_letter1;
			}

			find_ibeam();
			draw();
			dispatch_event = 1;
			result = 1;
    		break;

		case DELETE:
			if(highlight_letter1 == highlight_letter2)
			{
				if(ibeam_letter < text_len)
				{
					delete_selection(ibeam_letter, ibeam_letter + 1, text_len);
				}
			}
			else
			{
				delete_selection(highlight_letter1, highlight_letter2, text_len);
				highlight_letter2 = ibeam_letter = highlight_letter1;
			}
			
			find_ibeam();
			draw();
			dispatch_event = 1;
			result = 1;
			break;

		default:
    		if((top_level->get_keypress() == NEWLINE) ||
				(top_level->get_keypress() > 30 && top_level->get_keypress() < 127))
			{
				temp_string[0] = top_level->get_keypress();
				temp_string[1] = 0;
				insert_text(temp_string);
				find_ibeam();
				draw();
				dispatch_event = 1;
				result = 1;
			}
			break;
	}

	if(dispatch_event) handle_event();
	return result;
}

int BC_TextBox::uses_text()
{
	return 1;
}

void BC_TextBox::delete_selection(int letter1, int letter2, int text_len)
{
	int i, j;
	
	for(i = letter1, j = letter2; j < text_len; i++, j++)
	{
		text[i] = text[j];
	}
	text[i] = 0;
}

void BC_TextBox::insert_text(char *string)
{
	int i, j, text_len, string_len;

	string_len = strlen(string);
	text_len = strlen(text);
	if(highlight_letter1 < highlight_letter2)
	{
		delete_selection(highlight_letter1, highlight_letter2, text_len);
		highlight_letter2 = ibeam_letter = highlight_letter1;
	}

	text_len = strlen(text);

	for(i = text_len, j = text_len + string_len; i >= ibeam_letter; i--, j--)
		text[j] = text[i];

	for(i = ibeam_letter, j = 0; j < string_len; j++, i++)
		text[i] = string[j];

	ibeam_letter += string_len;
}

void BC_TextBox::get_ibeam_position(int &x, int &y)
{
	int i, j, k, row_begin, row_end, text_len;

	text_len = strlen(text);
	for(i = 0, k = 0; i < text_len; k += text_height)
	{
		row_begin = i;
		for(j = 0; text[i] != '\n' && i < text_len; j++, i++)
		{
			text_row[j] = text[i];
		}

		row_end = i;
		text_row[j] = 0;

		if(ibeam_letter >= row_begin && ibeam_letter <= row_end)
		{
			x = get_text_width(font, text_row, ibeam_letter - row_begin);
			y = k;
			return;
		}
		if(text[i] == '\n') i++;
	}

	x = 0;
	y = 0;
	return;
}

void BC_TextBox::find_ibeam()
{
	int x, y;

	get_ibeam_position(x, y);
	if(left_margin + text_x + x >= get_w() - right_margin - BCCURSORW)
	{
		text_x = -(x - (get_w() - get_w() / 4)) + left_margin;
		if(text_x > left_margin) text_x = left_margin;
	}
	else
	if(left_margin + text_x + x < left_margin)
	{
		text_x = -(x - (get_w() / 4)) + left_margin;
		if(text_x > left_margin) text_x = left_margin;
	}

	if((y >= get_h() - text_height - bottom_margin) || 
		(y < top_margin))
	{
		text_y = -(y - (get_h() / 2)) + top_margin;
		if(text_y > top_margin) text_y = top_margin;
	}
}

int BC_TextBox::get_cursor_letter(int cursor_x, int cursor_y)
{
	int i, j, k, l, row_begin, row_end, text_len, result = 0, done = 0;
	text_len = strlen(text);

	if(cursor_y >= get_h() - bottom_margin) result = text_len;

	for(i = 0, k = text_y; i < text_len && !done; k += text_height)
	{
		row_begin = i;
		for(j = 0; text[i] != '\n' && i < text_len; j++, i++)
		{
			text_row[j] = text[i];
		}
		row_end = i;
		text_row[j] = 0;

		if(cursor_y >= k && cursor_y < k + text_height)
		{
			for(j = 0; j <= row_end - row_begin && !done; j++)
			{
				l = get_text_width(font, text_row, j) + text_x;
				if(l > cursor_x)
				{
					done = 1;
					result = row_begin + j - 1;
				}
			}
			if(!done) result = row_end;
		}
		if(text[i] == '\n') i++;
	}
	if(result < 0) result = 0;
	if(result > text_len) result = text_len;
	return result;
}

void BC_TextBox::select_word(int &letter1, int &letter2, int ibeam_letter)
{
	int text_len = strlen(text);
	letter1 = letter2 = ibeam_letter;
	do
	{
		if(isalnum(text[letter1])) letter1--;
	}while(letter1 > 0 && isalnum(text[letter1]));
	if(!isalnum(text[letter1])) letter1++;

	do
	{
		if(isalnum(text[letter2])) letter2++;
	}while(letter2 < text_len && isalnum(text[letter2]));
	if(letter2 < text_len && text[letter2] == ' ') letter2++;

	if(letter1 < 0) letter1 = 0;
	if(letter2 < 0) letter2 = 0;
	if(letter1 > text_len) letter1 = text_len;
	if(letter2 > text_len) letter2 = text_len;
}
















BC_PopupTextBoxText::BC_PopupTextBoxText(BC_PopupTextBox *popup, int x, int y)
 : BC_TextBox(x, y, popup->text_w, 1, popup->default_text)
{
	this->popup = popup;
}

int BC_PopupTextBoxText::handle_event()
{
	return 1;
}

BC_PopupTextBoxList::BC_PopupTextBoxList(BC_PopupTextBox *popup, int x, int y)
 : BC_ListBox(x,
 	y,
	popup->text_w,
	popup->list_h,
	LISTBOX_TEXT,
	popup->list_items,
	0,
	0,
	1,
	0,
	1)
{
	this->popup = popup;
}
int BC_PopupTextBoxList::handle_event()
{
	popup->textbox->update(get_selection(0, 0)->get_text());
	popup->handle_event();
	return 1;
}




BC_PopupTextBox::BC_PopupTextBox(BC_WindowBase *parent_window, 
		ArrayList<BC_ListBoxItem*> *list_items,
		char *default_text,
		int x, 
		int y, 
		int text_w,
		int list_h)
{
	this->x = x;
	this->y = y;
	this->list_h = list_h;
	this->default_text = default_text;
	this->text_w = text_w;
	this->parent_window = parent_window;
	this->list_items = list_items;
}

BC_PopupTextBox::~BC_PopupTextBox()
{
	delete textbox;
	delete listbox;
}

int BC_PopupTextBox::create_objects()
{
	int x = this->x, y = this->y;
	parent_window->add_subwindow(textbox = new BC_PopupTextBoxText(this, x, y));
	x += textbox->get_w();
	parent_window->add_subwindow(listbox = new BC_PopupTextBoxList(this, x, y));
	return 0;
}

void BC_PopupTextBox::update(char *text)
{
	textbox->update(text);
}

void BC_PopupTextBox::update_list(ArrayList<BC_ListBoxItem*> *data)
{
	listbox->update(data, 0, 1);
}


char* BC_PopupTextBox::get_text()
{
	return textbox->get_text();
}

int BC_PopupTextBox::get_number()
{
	return listbox->get_selection_number(0, 0);
}

int BC_PopupTextBox::get_w()
{
	return textbox->get_w() + listbox->get_w();
}

int BC_PopupTextBox::get_h()
{
	return textbox->get_h();
}

int BC_PopupTextBox::handle_event()
{
	return 1;
}






BC_TumbleTextBoxText::BC_TumbleTextBoxText(BC_TumbleTextBox *popup, int x, int y)
 : BC_TextBox(x, y, popup->text_w, 1, popup->default_value)
{
	this->popup = popup;
}

int BC_TumbleTextBoxText::handle_event()
{
	return 1;
}

BC_TumbleTextBoxTumble::BC_TumbleTextBoxTumble(BC_TumbleTextBox *popup, 
		long min,
		long max,
		int x, 
		int y)
 : BC_ITumbler(popup->textbox, min, max, x, y)
{
	this->popup = popup;
}

BC_TumbleTextBox::BC_TumbleTextBox(BC_WindowBase *parent_window, 
		long default_value,
		long min,
		long max,
		int x, 
		int y, 
		int text_w)
{
	this->x = x;
	this->y = y;
	this->min = min;
	this->max = max;
	this->default_value = default_value;
	this->text_w = text_w;
	this->parent_window = parent_window;
}

BC_TumbleTextBox::~BC_TumbleTextBox()
{
	delete textbox;
	delete tumbler;
}

int BC_TumbleTextBox::create_objects()
{
	int x = this->x, y = this->y;
	parent_window->add_subwindow(textbox = new BC_TumbleTextBoxText(this, x, y));
	x += textbox->get_w();
	parent_window->add_subwindow(tumbler = new BC_TumbleTextBoxTumble(this, min, max, x, y));
	return 0;
}

char* BC_TumbleTextBox::get_text()
{
	return textbox->get_text();
}

int BC_TumbleTextBox::get_w()
{
	return textbox->get_w() + tumbler->get_w();
}

int BC_TumbleTextBox::get_h()
{
	return textbox->get_h();
}

int BC_TumbleTextBox::handle_event()
{
	return 1;
}

void BC_TumbleTextBox::set_boundaries(long min, long max)
{
	tumbler->set_boundaries(min, max);
}

