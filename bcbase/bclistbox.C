#include <string.h>
#include "bcfont.h"
#include "bclistbox.h"
#include "bcresources.h"
#include "bcwindow.h"

// ====================================================== item

BC_ListBoxItem::BC_ListBoxItem()
{
	text = 0;
}

BC_ListBoxItem::BC_ListBoxItem(char *text, int color)
{
	this->text = new char[strlen(text) + 1];
	strcpy(this->text, text);
	this->color = color;
}

BC_ListBoxItem::~BC_ListBoxItem()
{
	if(text) delete text;
	text = 0;
}

int BC_ListBoxItem::set_text(char *text)
{
	if(this->text) delete this->text;
	this->text = 0;

	if(!this->text)
	{
		this->text = new char[strlen(text) + 1];
		strcpy(this->text, text);
	}
return 0;
}

BC_ListBoxItem& BC_ListBoxItem::operator=(BC_ListBoxItem& item)
{
	if(item.text) set_text(item.text);
	color = item.color;
	y = item.y;
return 0;
}





// ====================================================== box

BC_ListBox::BC_ListBox(int x, int y, int w, int h, 
						ArrayList<BC_ListBoxItem*> *data,
						char **column_titles,
						int columns,
						int yposition,
						int currentitem)
 : BC_Tool(x, y, w, h)
{
	column_width = 0;
	this->data = data;
	this->column_titles = column_titles;
	this->columns = columns;
	this->yposition = yposition;
	this->currentitem = currentitem;
	xposition = 0;
	buttondown = 0;
	highlighted = -1;
	xscrollbar = 0;
	yscrollbar = 0;
	reset_query();      // reset the search engine
	deactivates_highlight = 1;  // Default is to unhighlight when deactivated.
}

BC_ListBox::~BC_ListBox()
{
	if(xscrollbar) delete xscrollbar;
	if(yscrollbar) delete yscrollbar;
	if(column_width) delete column_width;
}

int BC_ListBox::create_tool_objects()
{
	itemheight = get_text_height(MEDIUMFONT) + 2;
	text_descent = get_text_descent(MEDIUMFONT);
	create_column_width();
	create_window(x, y, w, h, WHITE);     // create the list display

	fix_scrollbars();
// draw the initial list
	draw();
return 0;
}

int BC_ListBox::set_contents(ArrayList<BC_ListBoxItem*> *data,
						char **column_titles,
						int columns,
						int yposition,
						int currentitem)
{
	this->data = data;
	this->column_titles = column_titles;
	this->columns = columns;
	this->yposition = yposition;
	this->xposition = 0;
	this->currentitem = currentitem;
	create_column_width();

	fix_scrollbars();
	draw();
return 0;
}

int BC_ListBox::get_yposition()
{
	return yposition;
return 0;
}

int BC_ListBox::fix_scrollbars()
{
// create the scrollbars
	if(get_total_width() > get_w())
	{
		if(!xscrollbar)
			subwindow->add_tool(xscrollbar = new BC_ListBoxXScroll(this, get_total_width(), get_w(), xposition));

		xscrollbar->set_position(get_total_width(), xposition, get_w());
	}
	else
	if(xscrollbar)
	{
		delete xscrollbar;
		xscrollbar = 0;
	}

	if(get_total_height() > get_visible_height())
	{
		if(!yscrollbar)
			subwindow->add_tool(yscrollbar = new BC_ListBoxYScroll(this, get_total_height(), get_visible_height(), yposition));

		yscrollbar->set_position(get_total_height(), yposition, get_visible_height());
	}
	else
	if(yscrollbar)
	{
		delete yscrollbar;
		yscrollbar = 0;
	}
return 0;
}

int BC_ListBox::resize(int x, int y, int w, int h,
						ArrayList<BC_ListBoxItem*> *data,
						char **column_titles,
						int columns,
						int yposition,
						int currentitem)
{
	resize_window(x, y, w, h);

	set_contents(data,
				column_titles,
				columns,
				yposition,
				currentitem);
return 0;
}


int BC_ListBox::set_size(int x, int y, int w, int h)
{
	resize_window(x, y, w, h);
	fix_scrollbars();
	draw();
return 0;
}

int BC_ListBox::resize(int w, int h)
{
// just redraw
	//draw();
return 0;
}

int BC_ListBox::get_totallines()
{
	if(data)
		return data[0].total;
	else 
		return 0;
return 0;
}

int BC_ListBox::get_total_height()
{
	if(data)	
		return data[0].total * itemheight;
	else 
		return 0;
return 0;
}

int BC_ListBox::get_visible_height()
{
	return get_h() - get_titleheight();
return 0;
}

int BC_ListBox::get_titleheight()
{
	if(column_titles) return itemheight + 2;
	else
	return 0;
return 0;
}


int BC_ListBox::get_total_width()
{
 	int total = 0, j;
 	for(j = 0; j < columns; j++)
	{
		total += get_column_width(j);
 	}
	return total;
return 0;
}

int BC_ListBox::get_column_width(int column)
{
	if(column_width) return column_width[column];
	else return get_w();
return 0;
}

int BC_ListBox::fix_item_y()
{
	int column, i, item_y;
	for(column = 0; column < columns && data; column++)
	{
		for(i = 0, item_y = -yposition + get_titleheight();
			i < data[column].total; i++, item_y += itemheight)
		{
			data[column].values[i]->y = item_y;
		}
	}
return 0;
}

int BC_ListBox::create_column_width()
{
	if(column_width) delete column_width;
	column_width = 0;

	int i, j, widest, width;
	if(columns && data)
	{
		column_width = new int[columns];
		for(i = 0; i < columns; i++)
		{
			if(column_titles && column_titles[i])
				widest = get_text_width(MEDIUMFONT, column_titles[i]) + 15;
			else
				widest = 15;

			for(j = 0; j < data[i].total; j++)
			{
				if(data[i].values[j]->text)
				{
					width = get_text_width(MEDIUMFONT, data[i].values[j]->text) + 15;
					if(width > widest) widest = width;
				}
			}
			column_width[i] = widest;
		}

// Width the first column until it fills the entire box
		if(get_total_width() < get_w()) column_width[0] += get_w() - get_total_width();
	}
return 0;
}

int BC_ListBox::unhighlight_()
{
	if(highlighted != -1)
	{
		highlighted = -1;
		draw();
	}
	return 0;
return 0;
}

int BC_ListBox::draw()
{
	int item_y, item_x, i, item_h;
	int column, color;

// clear background
	set_color(top_level->get_resources()->text_background);
	draw_box(0, 0, get_w(), get_h());

//printf("BC_ListBox::draw xposition %d yposition %d data %x\n", xposition,yposition, data);
// draw columns
	item_x = -xposition;
	fix_item_y();
	for(column = 0; 
		column < columns && data; 
		item_x += get_column_width(column), column++)
	{
// column visible
		if(!(item_x + get_column_width(column) < 0 || item_x > get_w()))
		{
// column items
			for(i = 0;
				i < data[column].total; i++)
			{
				item_y = data[column].values[i]->y;
				if(!(item_y + itemheight < get_titleheight() || item_y > get_h()))
				{
// item visible
// get item boundaries
					if(item_y > get_h() - 2) item_h = itemheight - 2; else item_h = itemheight;

// only do the highlight box for the first column
					if(column == 0)
					{
						if(i == currentitem)
						{
// highlight selected item
							set_inverse();
							set_color(top_level->get_resources()->highlight_inverse);
							draw_box(2, item_y, get_w() - 4, item_h);
							set_opaque();
						}
						else
						if(i == highlighted)
						{
// highlight the item the cursor is over
							set_color(LTGREY);
							draw_box(2, item_y, get_w() - 4, item_h);
						}
					}

					if(i < get_totallines() && data[column].values[i]->text)
					{
						if(i == currentitem &&
							data[column].values[i]->color == 
							top_level->get_resources()->highlight_inverse ^ get_color()) color = BLACK;
						else
							color = data[column].values[i]->color;

						set_color(color);
						draw_text(item_x + 5, item_y + itemheight - text_descent, data[column].values[i]->text);
					}
				}  // item visible
			}  // column items

// column title overwrites items
			if(column_titles)
			{
				draw_3d_big(item_x, 0, get_column_width(column), 
					get_titleheight(), 
					top_level->get_resources()->button_light, 
					top_level->get_resources()->button_up, 
					top_level->get_resources()->button_up, 
					top_level->get_resources()->button_shadow,
					BLACK);
				//set_font(SMALLFONT);
				set_color(BLACK);
				draw_text(item_x + 5, get_titleheight() - text_descent, column_titles[column]);
			}
		}  // column visible
	}  // column

// draw the border
	draw_3d_border(0, 0, get_w(), get_h(), 
		top_level->get_resources()->button_shadow, 
		(is_active()) ? RED : BLACK, 
		(is_active()) ? RED : top_level->get_resources()->button_up, 
		top_level->get_resources()->button_light);

// flash just the box
	flash();
return 0;
}

int BC_ListBox::stay_highlighted()
{
	deactivates_highlight = 0;
return 0;
}

int BC_ListBox::activate_()
{
	set_active_tool(this);
	draw();
return 0;
}

int BC_ListBox::deactivate_()
{
	set_active_tool(0);
	if(deactivates_highlight) set_selection(-1);
	draw();
return 0;
}

int BC_ListBox::set_selection(int selection)
{
	currentitem = selection;
	draw();
return 0;
}

int BC_ListBox::set_current_item(int currentitem)
{
	this->currentitem = currentitem;
	int y = data[0].values[currentitem]->y;

//printf("y %d yposition %d get_h %d\n", y, yposition, get_h());
// make sure current is in the window
	if(y > get_h() - itemheight || y < 0)
	{
		yposition = currentitem * itemheight - get_h() / 2;
	}

	if(yposition < 0) yposition = 0;
	if(yscrollbar) yscrollbar->set_position(yposition);
	draw();
return 0;
}

int BC_ListBox::get_selection(char *string, int column)
{
	if(currentitem != -1 && 
		currentitem < get_totallines() && 
		get_totallines() > 0)
	{
		strcpy(string, data[column].values[currentitem]->text);
	}
	else
	{
		sprintf(string, "");
	}
return 0;
}

char* BC_ListBox::get_selection(int column)
{
	if(currentitem != -1) return data[column].values[currentitem]->text;
	else return "";
}

int BC_ListBox::get_selection_number()
{
	return currentitem;
return 0;
}


int BC_ListBox::query_list()
{
	query_list(query);
return 0;
}

int BC_ListBox::query_list(char *regexp)
{
	int total = get_totallines();
	if(query[0] == 0) currentitem = 0;
	for(int i = 0; i < total; i++)
	{
		if(strcmp(regexp, data[0].values[i]->text) > 0)
		{
			currentitem = i + 1;
		}
	}
	if(currentitem > total - 1) currentitem = total - 1;
// don't set to item 0
	if(currentitem > 0) set_current_item(currentitem);
return 0;
}

int BC_ListBox::reset_query()
{
	query[0] = 0;  // reset query
	query_x = 0;     // don't redraw list here
return 0;
}

int BC_ListBox::set_query(char *new_query)
{
	strcpy(query, new_query);
	query_list(query);
return 0;
}

int BC_ListBox::motion_update()
{
	set_current_item(currentitem);
	reset_query();
return 0;
}

int BC_ListBox::cursor_left_()
{
	if(highlighted != -1)
	{
			highlighted = -1;
			draw();
	}
return 0;
}

int BC_ListBox::keypress_event_()
{
	if(!is_active()) return 0;

	int result = 0;
	int redraw = 0;
	switch(top_level->get_keypress())
	{
		case UP:
			if(currentitem > 0 && currentitem < get_totallines()) currentitem--;
			else currentitem = 0;
			motion_update();
			result = 1;  // new item
			break;

		case DOWN:
			if(currentitem < get_totallines() - 1)
			{
				currentitem++;
			}
			else currentitem = get_totallines() - 1;
			motion_update();
			result = 1;  // new item
			break;

		case PGUP:
			currentitem -= get_h() / itemheight;
			if(currentitem > get_totallines() - 1) currentitem = get_totallines() - 1;
			if(currentitem < 0) currentitem = 0;
			motion_update();
			result = 1;  // new item
			break;

		case PGDN:
			currentitem += get_h() / itemheight;
			if(currentitem > get_totallines() - 1) currentitem = get_totallines() - 1;
			if(currentitem < 0) currentitem = 0;
			motion_update();
			result = 1;  // new item
			break;

		case 13:
			reset_query();
			result = 2;
			break;

		case ESC:
			deactivate();
			result = 0;
			break;

		default:
			// perform query
			if(top_level->get_keypress() > 30 && top_level->get_keypress() < 127)
			{
				query[query_x++] = top_level->get_keypress();
				query[query_x] = 0;
				query_list();
			}
			else if(top_level->get_keypress() == BACKSPACE)
			{
				if(query_x > 0) query[--query_x] = 0;
				query_list();
			}
			redraw = 1;
			result = 1;  // new item
			break;
	}

	if(result) top_level->set_key_pressed(0);         // trap key
	if(redraw) draw();
	if(result == 2) handle_event();     // item selected
	if(result == 1) selection_changed();     // new item selected
	return 1;  // trap keypress
return 0;
}

int BC_ListBox::button_press_()
{
	if(get_event_win() != get_top_win()) return 0;

	if(get_cursorx() > 0 && get_cursorx() < get_w() &&
		 get_cursory() > get_titleheight() && get_cursory() < get_h())
	{
// click inside box selects item
		int result = 0;
		int redraw = 0;
		int olditem = currentitem;
		int total = get_totallines();
		int item, item_y;

		activate();

		for(item = 0; item < total && data; item++)
		{
			item_y = data[0].values[item]->y;
			if(get_cursory() > item_y && get_cursory() < item_y + itemheight)
			{
				currentitem = item;
				break;
			}
		}

		if(currentitem >= total) currentitem = total - 1;
		if(olditem != currentitem)
		{
			result = redraw = 1;
		}

// double click on an item
		if(top_level->get_double_click() && olditem == currentitem)
		{
			reset_query();
			result = 2;
		}

// set the button down status
		buttondown = 1;

		if(redraw) draw();
		if(result == 2)
		{
			handle_event();     // item selected
		}
		
		if(result == 1) selection_changed();     // new item selected
		return 1;     // trap event
	}
	else
	if(get_active_tool() == this)
	{
		deactivate();
	}

	return 0;
return 0;
}

int BC_ListBox::button_release_()
{
	buttondown = 0;
return 0;
}

int BC_ListBox::cursor_motion_()
{
	int result = 0;
	int new_item = 0;
	int redraw = 0;
	int total = get_totallines();
	int item, oldhighlight;
	int olditem = currentitem;
	int oldposition = yposition;
	int oldxposition = xposition;

	if(!buttondown && 
		get_cursorx() > 0 && get_cursorx() < get_w() &&
		get_cursory() > 0 && get_cursory() < get_h())
	{
// cursor inside box and button is up
		//result = 1;
		oldhighlight = highlighted;
		highlighted = -1;
		for(item = 0; item < total && data; item++)
		{
// get item cursor is over
			if(get_cursory() > data[0].values[item]->y && get_cursory() < data[0].values[item]->y + itemheight)
			{
				highlighted = item;
				break;
			}
		}

// cursor is beyond end of list
		if(highlighted >= total) highlighted = total - 1;

// cursor moved over new item
		if(oldhighlight != highlighted)
		{
			redraw = 1;
//printf("BC_ListBox::cursor_motion_\n");
		}
	}
	else 
// cursor moved outside box when an item was still highlighted
	if(highlighted != -1)
	{
		if(get_cursorx() <= 0 || get_cursorx() >= get_w() ||
			 get_cursory() <= 0 || get_cursory() >= get_h())
		{
			unhighlight();
		}
	}

	if(buttondown)
	{
// button is down and cursor is in listbox
		if(get_cursorx() > 0 && get_cursorx() < get_w() &&
			 get_cursory() > 0 && get_cursory() < get_h())
		{
			for(item = 0; item < total && data; item++)
			{
// get new item selected
				if(get_cursory() > data[0].values[item]->y && get_cursory() < data[0].values[item]->y + itemheight)
				{
					currentitem = item;
					item = total;
				}
			}

// current selection is beyond end of list
			if(currentitem >= total) currentitem = total-1;
			
			if(olditem != currentitem)
			{
				redraw = 1;
				new_item = 1;
			}
			result = 1;
		}
		else
		{
// cursor outside box so test boundaries for scrolling
			if(get_cursory() < 0)
			{
// scroll window
				if(yposition > 0)
				{
					yposition -= itemheight;
				}

				if(yposition < 0) yposition = 0;

// change item selected
				if(currentitem > 0)
				{
					currentitem--;
				}
				
				if(yposition != oldposition || olditem != currentitem)
				{
					if(yscrollbar) yscrollbar->set_position(yposition);
					redraw = 1;
					new_item = 1;
				}
			}
			else 
			if(get_cursory() > get_h()) 
			{
// scroll window
				if(yposition < get_total_height() - get_visible_height())
				{
					yposition += itemheight;
				}

				if(yposition > get_total_height() - get_visible_height())
					yposition = get_total_height() - get_visible_height();

// change item selected
				if(currentitem < total)
				{
					currentitem++;
				}
				
				if(yposition != oldposition || olditem != currentitem)
				{
					if(yscrollbar) yscrollbar->set_position(yposition);
					redraw = 1;
					new_item = 1;  // new item
				}
			}

			
			if(get_cursorx() < 0 && xposition > 0)
			{
				if(xposition < -get_cursorx())
				xposition -= xposition;
				else
				xposition -= -get_cursorx();
				
				if(xposition != oldxposition)
				{
					if(xscrollbar) xscrollbar->set_position(xposition);
					redraw = 1;
				}
			}
			else
			if(get_cursorx() > get_w() && xposition < get_total_width() - get_w())
			{
				if(xposition + get_cursorx() - get_w() > get_total_width() - get_w())
					xposition = get_total_width() - get_w();
				else
					xposition += get_cursorx() - get_w();

				if(xposition != oldxposition)
				{
					if(xscrollbar) xscrollbar->set_position(xposition);
					redraw = 1;
				}
			}
			result = 1;
		}
	}
//printf("redraw %d result %d\n", redraw, result);

	if(redraw) draw();
	//if(result == 2) handle_event();     // item selected
	if(new_item == 1) selection_changed();     // new item selected
	return result;
return 0;
}








// ======================================= scrollbars

BC_ListBoxYScroll::BC_ListBoxYScroll(BC_ListBox *listbox, int total_height, int view_height, int position)
 : BC_YScrollBar(listbox->get_x() + listbox->get_w(), listbox->get_y(), BC_SCROLLBAR_SIZE, listbox->get_h(), 
 				total_height, position, view_height)
{
	this->listbox = listbox;
}

int BC_ListBoxYScroll::handle_event()
{
	if(listbox->yposition != get_position())
	{
		listbox->yposition = get_position();      // get new position
		listbox->draw();     // redraw
	}
return 0;
}

BC_ListBoxXScroll::BC_ListBoxXScroll(BC_ListBox *listbox, int total_width, int view_width, int position)
 : BC_XScrollBar(listbox->get_x(), listbox->get_y() + listbox->get_h(), listbox->get_w(), BC_SCROLLBAR_SIZE, 
 				listbox->get_total_width(), position, listbox->get_w())
{
	this->listbox = listbox;
}

int BC_ListBoxXScroll::handle_event()
{
	if(listbox->xposition != get_position())
	{
		listbox->xposition = get_position();      // get new position
		listbox->draw();     // redraw
	}
return 0;
}







