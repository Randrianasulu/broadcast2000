#include <string.h>
#include "bcfont.h"
#include "bckeys.h"
#include "bctextbox.h"
#include "bcresources.h"
#include "bcwindow.h"

#include <ctype.h>

BC_TextBox::BC_TextBox(int x, int y, int w, char *text, int has_border)
	: BC_Tool(x, y, w, 25)
{
	strcpy(this->text, text);
	this->has_border = has_border;
}

BC_TextBox::BC_TextBox(int x, int y, int w, int text, int has_border)
	: BC_Tool(x, y, w, 25)
{
	sprintf(this->text, "%d", text);
	this->has_border = has_border;
}

BC_TextBox::BC_TextBox(int x, int y, int w, float text, int has_border)
	: BC_Tool(x, y, w, 25)
{
	sprintf(this->text, "%.3f", text);
	this->has_border = has_border;
}

int BC_TextBox::create_tool_objects()
{
	start = 0;
	x1 = x2 = -1;
	if(has_border)
	{
		back_color = WHITE;
		high_color = LTYELLOW;
  		h = get_text_height(MEDIUMFONT) + 8;
	}
	else 
	{
		high_color = LTGREY;
		back_color = subwindow->get_color();
  		h = get_text_height(MEDIUMFONT) + 2;
	}

	text_color = BLACK;
	text_ascent = get_text_ascent(MEDIUMFONT);

	selecttext = selectword = highlighted = 0;
	create_window(x, y, w, h, back_color);
	update(text);
return 0;
}

int BC_TextBox::resize_tool(int x, int y)
{
	resize_window(x, y, w, h);
	update();
return 0;
}

int BC_TextBox::draw()
{
	update();
return 0;
}

int BC_TextBox::cursor_left_()
{
	if(get_active_tool() != this && enabled)
	{
		if(cursor_x < 0 || cursor_x > w ||
			 cursor_y < 0 || cursor_y > h)
		{
			if(highlighted)
			{
				highlighted = 0;
				update();
				return 0;
			}
		}
	}
return 0;
}

int BC_TextBox::button_press_()
{
// Result == 1      trap event
// Result == 2      handle event
	int result = 0;
	if(get_event_win() != get_top_win()) return 0;
	if(!enabled) return 0;

	last_button = get_buttonpress();
	if(
// Subwindow area
		subwindow->get_cursor_x() > 0 && subwindow->get_cursor_x() < subwindow->get_w()
			&& subwindow->get_cursor_y() > 0 && subwindow->get_cursor_y() < subwindow->get_h()
// Tool area
		&& get_cursor_x() > 0 && get_cursor_y() > 0
		&& get_cursor_x() < w && get_cursor_y() < h)
	{
// cursor was inside boundaries
		result = 1;

		if(top_level->get_double_click())
		{    
// double click
			selectword = 1;         // set center of selection
			center = x1 = x2 = charof(cursor_x + 2) + start;
                                    // set word of selection
			while(isalnum(text[x1]) && x1 > 0) x1--;
			if(!isalnum(text[x1]) && x1 < center) x1++;
			while(isalnum(text[x2]) && x2 < strlen(text)) x2++;

			wordx1 = x1;
			wordx2 = x2;            // set original word selected
			copy_text();     // copy to the clipboard
		}
		else
		{                                 /* set center of selection */
			center = x1 = x2 = start + charof(cursor_x + 2);
			selecttext = 1; // select on

			if(get_buttonpress() == 2)
			{
// middle mouse button so paste at this point
				char string[BC_TEXTBOXMAX];
				top_level->from_clipboard(string, BC_TEXTBOXMAX);
				insert_text(string);
				result = 2;
			}
		}

		if(!is_active()) activate(); else update();
	}
	else
	{
 // cursor was outside boundary
		if(is_active())
		{
			deactivate();
		}
	}
	if(result == 2) handle_event();
	return result;
return 0;
}

int BC_TextBox::copy_text()
{
	char string[BC_TEXTBOXMAX];
	if(x2 > x1)
	{
		int i, j;
		for(i = 0, j = x1; j < x2; i++, j++)
		{
			string[i] = text[j];
		}
		string[i] = 0;
		top_level->to_clipboard(string);
	}
return 0;
}

int BC_TextBox::cursor_motion_()
{
	int result;
	result = 0;

	if((selecttext || selectword) && last_button != 2)
	{
		result = 1;
// Store the old selection for comparison
		x1_ = x1;
		x2_ = x2;

// decide which side to extend
		if(cursor_x < 0 && start > 0)
		{
			center_ = --start;
		}
		else if(cursor_x > w && x2 < strlen(text))
		{
			center_ = charof(w) + start;
			start++;
		}
		else 
		{
			center_ = charof(cursor_x) + start;
		}

// Change left selection boundary
		if(center_ < center)
		{
			if(selectword)
			{
				while(center_ > 0 && isalnum(text[center_])) center_--;

				if(center_ < x2 && !isalnum(text[center_])) center_++;
				x1 = center_;
				x2 = wordx2;        /* reset right side */
			}else{
				x1 = center_;       /* extend left side */
				x2 = center;        /* reset right side */
			}
		}
		else
		if(center_ >= center)
		{
// Change right selection boundary
			if(selectword)
			{        /* extend right word */
				int len;

				len = strlen(text);
				while(center_ < len && isalnum(text[center_])) center_++;

				x2 = center_;
				x1 = wordx1;
			}
			else
			{
				x2 = center_;        /* extend right side */
				x1 = center;         /* reset right side */
			}
		}

		if(x1_ != x1 || x2_ != x2)
		{
// Selection is different.  Display it.
			update();
		}
	}
	else
	{
// Change highlighting
		if(get_event_win() != get_top_win()) return 0;
		if(!enabled) return 0;
		if(get_button_down()) return 0;

		if(
			subwindow->get_cursor_x() > 0 && subwindow->get_cursor_x() < subwindow->get_w()
			&& subwindow->get_cursor_y() > 0 && subwindow->get_cursor_y() < subwindow->get_h()
			&& get_cursor_x() > 0 && get_cursor_x() < w
			&& get_cursor_y() > 0 && get_cursor_y() < h)
		{
			if(!highlighted)
			{
				top_level->unhighlight();
		  		highlighted = 1;
				update();
			}
			result = 1;
		}
		else
 		if(highlighted)
		{
		  	if(get_active_tool() != this)
		  	{
		  		highlighted = 0;
				update();
			}
		}
	}
	return result;
return 0;
}

int BC_TextBox::unhighlight_()
{
	if(highlighted)
	{
		highlighted = 0;
		update();
	}
	return 0;
return 0;
}

int BC_TextBox::button_release_()
{
	if(selecttext || selectword)
	{
  		selecttext = 0;
  		selectword = 0;
// need to update everything in window
  		top_level->set_button_just_released();
  		set_button_down(0);
		if(x2 > x1) copy_text();     // copy to the clipboard
	}
return 0;
}

int BC_TextBox::keypress_event_()
{
// Result == 2 user event handler
// Result == 1 trap keypress
// Result == 0 nothing
	int result = 0, i;
	if(get_active_tool() != this) return 0;
	if(!enabled) return 0;

	switch(top_level->get_key_pressed())
	{
		case 13:
			deactivate();
			result = 2;
			break;

		case ESC:
			deactivate();
			result = 1;
			break;

		case TAB:
			top_level->cycle_textboxes();
			result = 1;
			break;

		case LEFT:
			if(!top_level->shift_down() && x1 < x2) x2 = x1;

			if(top_level->shift_down()) 
			{
				if(x1 == x2)
				{
					if(x1 > 0) x1--;
					center = 0;
				}
				else
				if(center)
				{
					if(x2 > x1) x2--;
					else
					if(x1 > 0){
						x1--;
						center = 0;
					}
				}
				else
				if(x1 > 0) x1--;
			}
			else
			{
				if(x1 > 0){
					x1--;
					x2--;
				}
			}

			if(x1 < start) start = x1;
			update();
			result = 1;
			break;

    	case RIGHT:
    		if(!top_level->shift_down() && x1 < x2) x1 = x2;

    		if(top_level->shift_down())
    		{
        		if(x1 == x2)
        		{
        		  	if(x2 < strlen(text)){
						x2++;
            			center = 1;
					}
				}
				else
        		if(center)
        		{
        			if(x2 < strlen(text)) x2++;
        		}
        		else
        		if(x1 < x2) x1++;
        		else
        		if(x2 < strlen(text)){
        		  x2++;
        		  center = 1;
        		}
    		}
    		else
    		{
        		if(x2 < strlen(text)){
        			x2++;
        			x1++;
				}
    		}

    		while(XTextWidth(top_level->get_font(MEDIUMFONT), &text[start], x2 - start) + 2 > w - 2) start++;
    		update();
			result = 1;
    		break;

    	case BACKSPACE:
    		if(x1 == x2)
			{                    /* single character */
				if(x1 > 0)
				{
					for(i = x1 - 1; i < strlen(text); i++)
					{
						text[i] = text[i+1];
					}
					x1--;
					x2--;
					if(start) start--;     /* scroll left if extended */
				}
    		}
			else
			{
				delete_selection();
			}

    		if(x2 < start) start = x2;

    		update();
    		result = 2;
    		break;

		case DELETE:
			if(x1 == x2)
			{                     /* single character */
				if(x1 < strlen(text))
				{
					for(i = x1; i < strlen(text); i++)
					{
						text[i] = text[i+1];
					}
				}
			}
			else
			{
				delete_selection();
			}

    		if(x2 < start) start = x2;

    		update();
    		result = 2;
			break;

	    default:
    		if(top_level->get_keypress() > 30 && top_level->get_keypress() < 127)
    		{
				char string[2];
				string[0] = top_level->get_keypress();
				string[1] = 0;
				insert_text(string);
        		result = 2;
			}
			break;
	}

	if(result == 2) handle_event();
	trap_keypress();         // trap key
	return result;
return 0;
}

int BC_TextBox::delete_selection()
{
	int i;
	for(i = x1; x2 < strlen(text); i++, x2++)
	{
        text[i] = text[x2];
	}
	text[i] = 0;
	x2 = x1;
return 0;
}


int BC_TextBox::insert_text(char *string)
{
	int len = strlen(string);

	for(int i = 0, j; i < len && x2 < BC_TEXTBOXMAX; i++)
	{
    	if(x1 == x2)
		{            // insert and move all chars back 1
			for(j = strlen(text) + 1; j > x1; j--)
			{
				text[j] = text[j - 1];
			}
    	}
		else
		{       // replace and move all chars up to x1 + 1
        	for(j = x1+1; x2 <= strlen(text);)
        	{
            	text[j++] = text[x2++];
        	}
        	x2 = x1;
    	}

    	text[x1] = string[i];

    	x1++;
    	x2++;
	}

// fit text into window
    while(XTextWidth(top_level->get_font(MEDIUMFONT), &text[start], x2 - start) + 2 > w - 8) start++;

    update();
	return 0;
return 0;
}

// return char position in string of pixel x
int BC_TextBox::charof(int x_)
{
	int len = strlen(&text[start]);
	
	x_ -= 5;
	
  for(; len > 0 && XTextWidth(top_level->get_font(MEDIUMFONT), &text[start], len) > x_; len--)
    ;

  return len;
return 0;
}

char* BC_TextBox::get_text()
{
	return text;
}

int BC_TextBox::update(char *text_)
{
	strcpy(text, text_);

	int len;                          // length of string to print
	start = 0;
	len = strlen(&text[start]);       // cut length until it fits in box
	while(XTextWidth(top_level->get_font(MEDIUMFONT), &text[start], len) > w - 8)
	{ len--;   start++; }

	update();
return 0;
}

int BC_TextBox::update(int value)
{
	sprintf(text, "%d", value);
	update(text);
return 0;
}

int BC_TextBox::update(float value)
{
	sprintf(text, "%.3f", value);
	update(text);
return 0;
}

int BC_TextBox::update()
{
	int column1, column2;                 // columns for highlight box
	int len;                              // length of string to print
	int distance_from_edge, distance_from_top;
	
	if(has_border) { distance_from_edge = 4; distance_from_top = h - 7; }
	else { distance_from_edge = 2; distance_from_top = text_ascent; }
	
	if(has_border)
	{
		if(highlighted)
			draw_3d_big(0, 0, w, h, 
				top_level->get_resources()->button_shadow, 
				RED, 
				WHITE, 
				LTPINK,
				top_level->get_resources()->button_light);
		else
			draw_3d_big(0, 0, w, h, 
				top_level->get_resources()->button_shadow, 
				BLACK, 
				WHITE, 
				top_level->get_resources()->button_up,
				top_level->get_resources()->button_light);
	}
	else
	{
		if(highlighted)
		{
			set_color(high_color);
			draw_box(0, 0, w, h);
		}
		else
		{
			set_color(back_color);
			draw_box(0, 0, w, h);
		}
	}
	
	// draw text without cursor
  len = strlen(&text[start]);       // cut length until it fits in box
  while(XTextWidth(top_level->get_font(MEDIUMFONT), &text[start], len) > w - distance_from_edge * 2){
  	len--;
  }

	set_color(text_color);
	set_font(MEDIUMFONT);
	XDrawString(top_level->display, pixmap, top_level->gc, distance_from_edge, distance_from_top, &text[start], len);
	
	if(get_active_tool() == this) 
	{
// draw selection
  	if(start <= x1)
  	{        
// x1 after start
    	column1 = XTextWidth(top_level->get_font(MEDIUMFONT), &text[start], x1 - start) + distance_from_edge;
  	}
  	else
  	{                // x1 before view
    	column1 = -1;
  	}

  	if(x2 >= start)
  	{        
// x2 after start
    	column2 = XTextWidth(top_level->get_font(MEDIUMFONT), &text[start], x2 - start) + distance_from_edge;
  	}
  	else
  	{
    	column2 = -1;         // x2 before view
  	}

  	if(column1 < w - 2 && column2 >= 2)
  	{  
// selection in view
    	if(column1 < distance_from_edge) column1 = distance_from_edge;      // x1 before view
    	if(column2 >= w - distance_from_edge) column2 = w - distance_from_edge - 1;  // x2 after view

  		set_inverse();
  		set_color(back_color);    // set inverse
			
			column2++;           // add one for single cursor
			
			if(has_border)
    		draw_box(column1, 3, column2 - column1, h - 6);
    	else
    		draw_box(column1, 1, column2 - column1, h - 2);
    	set_opaque();
  	}

  	set_opaque();
  }

	flash();
return 0;
}

int BC_TextBox::activate_()
{
	if(x1 == -1)
	{
  		x2 = x1 = 0;
	}
	highlighted = 1;

	update();
return 0;
}

int BC_TextBox::deactivate_()
{
  	selecttext = 0; // draw a text box with no cursor
  	selectword = 0;
  	x1 = x2 = -1;
	set_active_tool(0);
	highlighted = 0;

	update();
return 0;
}

int BC_TextBox::uses_text()
{             // set to 1 if tool uses text input
	return 1;
return 0;
}
