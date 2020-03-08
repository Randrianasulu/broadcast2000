#include "bcdragwindow.h"
#include "bclistbox.h"
#include "bcpixmap.h"
#include "bcresources.h"
#include "cursors.h"
#include "fonts.h"
#include "keys.h"
#include "vframe.h"

#include <string.h>

// ====================================================== item

BC_ListBoxItem::BC_ListBoxItem()
{
	initialize();
}

BC_ListBoxItem::BC_ListBoxItem(int x, 
	int y, 
	char *text, 
	BC_Pixmap *icon, 
	int color)
{
	initialize();
	this->x = x;
	this->y = y;
	this->text = new char[strlen(text) + 1];
	this->icon = icon;

	strcpy(this->text, text);
	this->color = color;
}

BC_ListBoxItem::BC_ListBoxItem(char *text, int color)
{
	initialize();
	this->text = new char[strlen(text) + 1];
	strcpy(this->text, text);
	this->color = color;
}

BC_ListBoxItem::~BC_ListBoxItem()
{
	if(text) delete text;
}

int BC_ListBoxItem::initialize()
{
	text = 0;
	color = BLACK;
	selected = 0;
	icon = 0;
	y = 0;
	x = 0;
	return 0;
}

int BC_ListBoxItem::get_x()
{
	return x;
}

int BC_ListBoxItem::get_y()
{
	return y;
}

int BC_ListBoxItem::get_icon_w()
{
	return icon->get_w();
}

int BC_ListBoxItem::get_icon_h()
{
	return icon->get_h();
}

void BC_ListBoxItem::set_text(char *new_text)
{
	if(this->text) delete this->text;
	this->text = 0;

	if(new_text)
	{
		this->text = new char[strlen(new_text) + 1];
		strcpy(this->text, new_text);
	}
}

char* BC_ListBoxItem::get_text()
{
	return text;
}

void BC_ListBoxItem::set_icon(BC_Pixmap *icon)
{
	this->icon = icon;
}

void BC_ListBoxItem::set_color(int color)
{
	this->color = color;
}

int BC_ListBoxItem::get_color()
{
	return color;
}


BC_ListBoxItem& BC_ListBoxItem::operator=(BC_ListBoxItem& item)
{
	if(item.text) set_text(item.text);
	color = item.color;
	y = item.y;
	return *this;
}


// ====================================================== scrollbars


BC_ListBoxYScroll::BC_ListBoxYScroll(BC_ListBox *listbox, 
	                  int total_height, 
					  int view_height, 
	                  int position)
 : BC_ScrollBar(listbox->get_yscroll_x(), listbox->get_yscroll_y(), SCROLL_VERT, 
 	listbox->get_yscroll_height(), total_height, position, view_height)
{
	this->listbox = listbox;
}

int BC_ListBoxYScroll::handle_event()
{
	listbox->set_yposition(get_value());
	return 1;
}

BC_ListBoxXScroll::BC_ListBoxXScroll(BC_ListBox *listbox, 
	                  int total_width, 
					  int view_width,
	                  int position)
 : BC_ScrollBar(listbox->get_xscroll_x(), listbox->get_xscroll_y(), SCROLL_HORIZ, 
 	listbox->get_xscroll_width(), total_width, position, view_width)
{
	this->listbox = listbox;
}

int BC_ListBoxXScroll::handle_event()
{
	listbox->set_xposition(get_value());
	return 1;
}


// ====================================================== box

BC_ListBox::BC_ListBox(int x, int y, int w, int h,
		int display_format,
		ArrayList<BC_ListBoxItem*> *data,
		char **column_titles,
		int *column_width,
		int columns,
		int yposition,
		int popup,
		int selection_mode,
		int icon_position,
		int assign_icon_coords,
		int allow_drag)
 : BC_SubWindow(x, y, w, h, -1)
{
	this->data = data;
	this->column_titles = column_titles;
	this->columns = columns;
	this->yposition = yposition;
	this->column_width = column_width;
	this->popup = popup;
	this->display_format = display_format;
	this->selection_mode = selection_mode;
	this->icon_position = icon_position;
	this->assign_icon_coords = assign_icon_coords;
	this->allow_drag = allow_drag;
	popup_w = w;
	popup_h = h;

	xposition = 0;
	highlighted_item = -1;
	highlighted = 0;
	xscrollbar = 0;
	yscrollbar = 0;
	in_division = 0;
	current_cursor = ARROW_CURSOR;
	gui = 0;
	view_h = 0;
	view_w = 0;
	title_h = 0;
	selection_active = 0;
	active = 0;
	new_value = 0;
	need_xscroll = 0;
	need_yscroll = 0;
	bg_tile = 0;
	drag_popup = 0;
	last_selection1 = last_selection2 = 0;
	bg_surface = 0;
	bg_pixmap = 0;
// reset the search engine
	reset_query();
}

BC_ListBox::~BC_ListBox()
{
	if(bg_surface) delete bg_surface;
	if(bg_pixmap) delete bg_pixmap;
	if(xscrollbar) delete xscrollbar;
	if(yscrollbar) delete yscrollbar;
	if(popup)
	{
		delete images[0];
		delete images[1];
		delete images[2];
	}
	delete drag_icon;
}

void BC_ListBox::reset_query()
{
	query[0] = 0;  // reset query
}

int BC_ListBox::evaluate_query(int list_item, char *string)
{
	return(strcmp(string, data[0].values[list_item]->text) <= 0);
}

void BC_ListBox::query_list()
{
	if(query[0] == 0) return;

	int done = 0, result;
	for(int i = 0; !done && i < data[0].total; i++)
	{
		if(evaluate_query(i, data[0].values[i]->text))
		{
			result = i;
			done = 1;
		}
	}

	if(done)
	{
		for(int i = 0; i < data[0].total; i++)
		{
			for(int j = 0; j < columns; j++)
			{
				data[j].values[i]->selected = 0;
			}
		}

		for(int j = 0; j < columns; j++)
		{
			data[j].values[result]->selected = 1;
		}
		center_selection(result);
	}
}

void BC_ListBox::init_column_width()
{
	if(!column_width && data)
	{
		int widest = 5, w;
		for(int i = 0; i < data[0].total; i++)
		{
			w = get_text_width(MEDIUMFONT, data[0].values[i]->get_text()) + 2 * LISTBOX_MARGIN;
			if(w > widest) widest = w;
		}
//		if(widest < popup_w - 4) widest = popup_w - 4;
		default_column_width[0] = widest;
		column_width = default_column_width;
	}
}

int BC_ListBox::initialize()
{
	if(popup)
	{
		images[0] = new BC_Pixmap(parent_window, BC_WindowBase::get_resources()->listbox_button[0], PIXMAP_ALPHA);
		images[1] = new BC_Pixmap(parent_window, BC_WindowBase::get_resources()->listbox_button[1], PIXMAP_ALPHA);
		images[2] = new BC_Pixmap(parent_window, BC_WindowBase::get_resources()->listbox_button[2], PIXMAP_ALPHA);
		w = images[0]->get_w();
		h = images[0]->get_h();
		gui = 0;
		status = LISTBOX_UP;
	}
	else
	{
		gui = this;
	}

	drag_icon = new BC_Pixmap(parent_window, BC_WindowBase::get_resources()->type_to_icon[ICON_UNKNOWN], PIXMAP_ALPHA);
	BC_SubWindow::initialize();

	init_column_width();

	if(assign_icon_coords) set_item_coords();

	if(top_level->get_resources()->listbox_bg)
		bg_pixmap = new BC_Pixmap(this, 
			get_resources()->listbox_bg, 
			PIXMAP_OPAQUE);

	draw_face();
	draw_items();
	if(!popup) gui->flash();
	return 0;
}

int BC_ListBox::draw_face()
{
// Draw the button for a popup listbox
	if(popup)
	{
		draw_top_background(parent_window, 0, 0, w, h);
		images[status]->write_drawable(pixmap, 
			0, 
			0,
			w,
			h,
			0,
			0);
		flash();
	}
	return 0;
}

int BC_ListBox::set_item_coords()
{
	if(!data) return 0;

	if(display_format == LISTBOX_TEXT)
	{
		for(int i = 0, y = 0; 
			i < data[0].total; 
			i++, y += get_text_height(MEDIUMFONT))
		{
			for(int j = 0, x = 0; j < columns; j++)
			{
				data[j].values[i]->y = y;
				data[j].values[i]->x = x;
				if(j < columns - 1) x += column_width[j];
			}
		}
	}
	else
	if(display_format == LISTBOX_ICONS)
	{
		int x = 0, y = 0;
		int row_h = 0, column_w = 0;

// Get row height
		for(int i = 0; i < data[0].total; i++)
		{
			int item_h = get_item_h(0, i);
			if(item_h > row_h) row_h = item_h;
		}

// Place items
		for(int i = 0; i < data[0].total; i++)
		{
			int item_w = get_item_w(0, i);
			int item_h = get_item_h(0, i);
			
// Insert values into all columns to satisfy get_items_height
			for(int j = 0; j < columns; j++)
			{
				data[j].values[i]->x = x;
				data[j].values[i]->y = y;
			}
			
			if(item_w > column_w) column_w = item_w;
			y += row_h;
			if(y >= get_h() - row_h)
			{
				x += column_w;
				y = 0;
				column_w = 0;
			}
		}
	}
	return 0;
}

int BC_ListBox::get_display_mode()
{
	return display_format;
}

int BC_ListBox::get_yposition()
{
	return yposition;
}

int BC_ListBox::get_xposition()
{
	return xposition;
}

int BC_ListBox::get_item_x(int column, int item)
{
	return data[column].values[item]->x - xposition + 2;
}

int BC_ListBox::get_item_y(int column, int item)
{
	int result;
	result = data[column].values[item]->y - yposition + title_h + 2;

	return result;
}

int BC_ListBox::get_item_w(int column, int item)
{
	if(display_format == LISTBOX_ICONS)
	{
		int x, y, w, h;
		get_icon_mask(column, item, x, y, w, h);
		int icon_w = w;
		get_text_mask(column, item, x, y, w, h);
		int text_w = w;

		if(icon_position == ICON_LEFT)
			return icon_w + text_w;
		else
			return (icon_w > text_w) ? icon_w : text_w;
	}
	else
	{
		return get_text_width(MEDIUMFONT, data[column].values[item]->text) + 2 * LISTBOX_MARGIN;
	}
}

int BC_ListBox::get_item_h(int column, int item)
{
	if(display_format == LISTBOX_ICONS)
	{
		int x, y, w, h;
		get_icon_mask(column, item, x, y, w, h);
		int icon_h = h;
		get_text_mask(column, item, x, y, w, h);
		int text_h = h;

		if(icon_position == ICON_LEFT)
			return (icon_h > text_h) ? icon_h : text_h;
		else
			return icon_h + text_h;
	}
	else
	{
		return get_text_height(MEDIUMFONT);
	}
	return 0;
}


int BC_ListBox::get_icon_w(int column, int item)
{
	BC_Pixmap *icon = data[column].values[item]->icon;
	if(icon) return icon->get_w();
	return 0;
}

int BC_ListBox::get_icon_h(int column, int item)
{
	BC_Pixmap *icon = data[column].values[item]->icon;
	if(icon) return icon->get_h();
	return 0;
}

int BC_ListBox::get_items_width()
{
	int widest = 0;

	if(display_format == LISTBOX_ICONS)
	{
		for(int i = 0; i < columns; i++)
		{
			for(int j = 0; j < data[i].total; j++)
			{
				int x1, x, y, w, h;
				x1 = data[i].values[j]->x;

				get_icon_mask(i, j, x, y, w, h);
				if(x1 + w > widest) widest = x1 + w;

				if(display_format == LISTBOX_ICONS && icon_position == ICON_LEFT)
					x1 += w;

				get_text_mask(i, j, x, y, w, h);
				if(x1 + w > widest) widest = x1 + w;
			}
		}
	}
	else
	if(display_format == LISTBOX_TEXT)
	{
		return get_column_offset(columns);
	}
	return widest;
}

int BC_ListBox::get_items_height()
{
	int highest = 0;

	for(int i = 0; i < columns; i++)
	{
		for(int j = 0; j < data[i].total; j++)
		{
			int y1, x, y, w, h;
			y1 = data[i].values[j]->y;

			get_icon_mask(i, j, x, y, w, h);
			if(y1 + h > highest) highest = y1 + h;
			get_text_mask(i, j, x, y, w, h);
			if(y1 + h > highest) highest = y1 + h;
		}
	}
	if(display_format == LISTBOX_TEXT) highest += LISTBOX_MARGIN;

	return highest;
}

int BC_ListBox::set_yposition(int position)
{
	this->yposition = position;
	draw_items();
	gui->flash();
	return 0;
}

int BC_ListBox::set_xposition(int position)
{
	this->xposition = position;
	draw_items();
	gui->flash();
	return 0;
}

int BC_ListBox::get_w()
{
	if(popup)
		return BCPOPUPLISTBOX_W;
	else
		return popup_w;
}

int BC_ListBox::get_h()
{
	if(popup)
		return BCPOPUPLISTBOX_H;
	else
		return popup_h;
}

int BC_ListBox::get_yscroll_x()
{
	if(popup)
		return popup_w - SCROLL_SPAN;
	else
		return get_x() + popup_w - SCROLL_SPAN;
}

int BC_ListBox::get_yscroll_y()
{
	if(popup)
		return 0;
	else
		return get_y();
}

int BC_ListBox::get_yscroll_height()
{
	return popup_h - (need_xscroll ? SCROLL_SPAN : 0);
}

int BC_ListBox::get_xscroll_x()
{
	if(popup)
		return 0;
	else
		return get_x();
}

int BC_ListBox::get_xscroll_y()
{
	if(popup)
		return popup_h - SCROLL_SPAN;
	else
		return get_y() + popup_h - SCROLL_SPAN;
}

int BC_ListBox::get_xscroll_width()
{
	return popup_w - (need_yscroll ? SCROLL_SPAN : 0);
}

int BC_ListBox::get_column_offset(int column)
{
	int x = 0;
	while(column > 0)
	{
		x += column_width[--column];
	}
	return x;
}

void BC_ListBox::column_width_boundaries()
{
	for(int i = 0; i < columns; i++)
		if(column_width[i] < 10) column_width[i] = 10;
}

int BC_ListBox::get_column_width(int column)
{
	if(column < columns - 1)
		return column_width[column];
	else
		return popup_w + 
			xposition - 
			get_column_offset(column);
}

BC_Pixmap* BC_ListBox::get_item_pixmap(int item)
{
	return data[0].values[item]->icon;
}

int BC_ListBox::get_icon_mask(int column, int item, int &x, int &y, int &w, int &h)
{
	if(display_format == LISTBOX_ICONS)
	{
		x = get_item_x(0, item);
		y = get_item_y(0, item);
		w = get_icon_w(0, item) + ICON_MARGIN * 2;
		h = get_icon_h(0, item) + ICON_MARGIN * 2;
	}
	else
	if(display_format == LISTBOX_TEXT)
	{
		x = y = w = h = 0;
	}
	return 0;
}

int BC_ListBox::get_text_mask(int column, int item, int &x, int &y, int &w, int &h)
{
	x = get_item_x(column, item);
	y = get_item_y(column, item);

	if(display_format == LISTBOX_ICONS)
	{
		if(icon_position == ICON_LEFT)
		{
			x += get_icon_w(column, item) + ICON_MARGIN * 2;
			y += get_icon_h(column, item) - get_text_height(MEDIUMFONT); - ICON_MARGIN * 2;
		}
		else
		{
			y += get_icon_h(column, item) + ICON_MARGIN;
		}

		w = get_text_width(MEDIUMFONT, data[column].values[item]->text) + ICON_MARGIN * 2;
		h = get_text_height(MEDIUMFONT) + ICON_MARGIN * 2;
	}
	else
	if(display_format == LISTBOX_TEXT)
	{
		w = get_text_width(MEDIUMFONT, data[column].values[item]->text) + LISTBOX_MARGIN * 2;
		h = get_text_height(MEDIUMFONT);
	}
	return 0;
}

int BC_ListBox::get_item_highlight(int column, int item)
{
	if(data[column].values[item]->selected)
		return BLUE;
	else
	if(highlighted_item == item)
		return LTGREY;
	else
		return WHITE;
}

int BC_ListBox::get_item_color(int column, int item)
{
	int color = data[column].values[item]->color;
	if(get_item_highlight(column, item) == color)
		return BLACK;
	else
		return color;
}


BC_ListBoxItem* BC_ListBox::get_selection(int column, int selection_number)
{
	for(int i = 0; i < data[0].total; i++)
	{
		if(data[0].values[i]->selected)
		{
			if(!selection_number)
			{
				return data[column].values[i];
			}
			selection_number--;
		}
	}
	return 0;
}

int BC_ListBox::get_selection_number(int column, int selection_number)
{
	for(int i = 0; i < data[0].total; i++)
	{
		if(data[0].values[i]->selected)
		{
			if(!selection_number)
			{
				return i;
			}
			selection_number--;
		}
	}
	return -1;
}

int BC_ListBox::set_selection_mode(int mode)
{
	this->selection_mode = mode;
	return 0;
}

int BC_ListBox::update(ArrayList<BC_ListBoxItem*> *data,
						char **column_titles,
						int columns,
						int yposition,
						int xposition, 
						int currentitem,
						int set_coords)
{
	this->data = data;
	this->column_titles = column_titles;
	this->columns = columns;

	this->yposition = yposition;
	this->xposition = xposition;
	highlighted_item = -1;

	for(int i = 0; i < data[0].total; i++)
	{
		for(int j = 0; j < columns; j++)
		{
			if(i == currentitem)
				data[j].values[i]->selected = 1;
			else
				data[j].values[i]->selected = 0;
		}
	}

	init_column_width();

	if(set_coords)
		set_item_coords();

	if(gui)
	{
		draw_items();
		update_scrollbars();
		gui->flash();
	}
	return 0;
}

void BC_ListBox::move_vertical(int pixels)
{
}

void BC_ListBox::move_horizontal(int pixels)
{
}

void BC_ListBox::fix_positions()
{
	if(yposition < 0) yposition = 0;
	else
	if(yposition > get_items_height() - view_h)
		yposition = get_items_height() - view_h;

	if(yposition < 0) yposition = 0;

	if(xposition < 0) xposition = 0;
	else
	if(xposition >= get_items_width() - view_w)
		xposition = get_items_width() - view_w;

	if(xposition < 0) xposition = 0;
}

void BC_ListBox::center_selection(int selection)
{
	if(data[0].values[selection]->y - yposition  > view_h - get_text_height(MEDIUMFONT) ||
		data[0].values[selection]->y - yposition < 0)
	{
		yposition = selection * get_text_height(MEDIUMFONT) - view_h / 2;
	}
	
	if(display_format == LISTBOX_ICONS)
	{
		if(data[0].values[selection]->x - xposition > view_w ||
			data[0].values[selection]->x - xposition < 0)
		{
			xposition = data[0].values[selection]->x - view_w / 2;
		}
	}
}

void BC_ListBox::update_scrollbars()
{
	int h_needed = get_items_height();
	int w_needed = get_items_width();

	if(xscrollbar)
	{
		if(xposition != xscrollbar->get_value())
			xscrollbar->update_value(xposition);

		if(w_needed != xscrollbar->get_length() || view_w != xscrollbar->get_handlelength())
			xscrollbar->update_length(w_needed, xposition, view_w);
	}

	if(yscrollbar)
	{
		if(yposition != yscrollbar->get_value())
			yscrollbar->update_value(yposition);

		if(h_needed != yscrollbar->get_length() || view_h != yscrollbar->get_handlelength())
			yscrollbar->update_length(h_needed, yposition, view_h);
	}
}

void BC_ListBox::test_drag_scroll(int &redraw, int cursor_x, int cursor_y)
{
	if(in_division)
		return;
	else
	if(cursor_y < 2)
	{
		yposition -= (title_h + 2) - cursor_y;
		redraw = 1;
	}
	else
	if(cursor_y >= view_h + title_h + 4)
	{
		yposition += cursor_y - (view_h + title_h + 4);
		redraw = 1;
	}

	if(cursor_x < 2)
	{
		xposition -= 2 - cursor_x;
		redraw = 1;
	}
	else
	if(cursor_x >= view_w + 2)
	{
		xposition += cursor_x - (view_w + 2);
		redraw = 1;
	}
}

int BC_ListBox::cursor_item(int cursor_x, int cursor_y)
{
	if(display_format == LISTBOX_ICONS)
	{
		for(int i = 0; i < columns; i++)
		{
			for(int j = data[i].total - 1; j >= 0; j--)
			{
				int icon_x, icon_y, icon_w, icon_h;
				int text_x, text_y, text_w, text_h;
				get_icon_mask(i, j, icon_x, icon_y, icon_w, icon_h);
				get_text_mask(i, j, text_x, text_y, text_w, text_h);

				if((cursor_x >= icon_x && cursor_x < icon_x + icon_w &&
					cursor_y >= icon_y && cursor_y < icon_y + icon_h) ||
					(cursor_x >= text_x && cursor_x < text_x + text_w &&
					cursor_y >= text_y && cursor_y < text_y + text_h))
				{
					return j;
				}
			}
		}
	}
	else
	if(display_format == LISTBOX_TEXT)
		if(cursor_x >= 0 && 
			cursor_x < (xscrollbar ? gui->get_w() - SCROLL_SPAN : gui->get_w()) &&
			cursor_y > 0 && 
			cursor_y < gui->get_h())
		{
			for(int i = 0; i < data[0].total; i++)
			{
				if(cursor_y >= get_item_y(0, i) &&
					cursor_y < get_item_y(0, i) + get_item_h(0, i))
				{
					return i;
				}
			}
		}
	return -1;
}

int BC_ListBox::repeat_event(long duration)
{
	if(duration == top_level->get_resources()->tooltip_delay &&
		tooltip_text[0] != 0 &&
		popup &&
		status == LISTBOX_HIGH &&
		!tooltip_done)
	{
		show_tooltip();
		tooltip_done = 1;
		return 1;
	}
	return 0;
}


int BC_ListBox::cursor_enter_event()
{
	int result = 0;
	
//	if(active) result = 1;

	if(popup)
	{
		if(top_level->event_win == win)
		{
			tooltip_done = 0;
			if(top_level->button_down)
			{
				status = LISTBOX_DN;
			}
			else
			if(status == LISTBOX_UP)
			{
				status = LISTBOX_HIGH;
			}
			draw_face();
			result = 1;
		}
	}

	if(gui && top_level->event_win == gui->win)
	{
		if(!highlighted)
		{
			highlighted = 1;
			draw_border();
			flash();
		}
		result = 1;
	}
	return result;
}

int BC_ListBox::cursor_leave_event()
{
	if(popup)
	{
		hide_tooltip();
		if(status == LISTBOX_HIGH)
		{
			status = LISTBOX_UP;
			draw_face();
		}
	}

	if(gui && highlighted)
	{
		highlighted = 0;
		if(highlighted_item >= 0) 
		{
			highlighted_item = -1;
			draw_items();
		}
		else
			draw_border();
		gui->flash();
	}
	update_cursor(0);
	return 0;
}

int BC_ListBox::button_press_event()
{
// Selected item
	int selection = -1;
	int redraw = 0;
	int result = 0;

	hide_tooltip();
	if(popup)
	{
		if(top_level->event_win == win)
		{
			status = LISTBOX_DN;
			draw_face();
// Deploy listbox
			if(!active)
			{
				top_level->deactivate();
				activate();
			}

			result = 1;
		}
		else
		if((xscrollbar && top_level->event_win == xscrollbar->win) ||
			(yscrollbar && top_level->event_win == yscrollbar->win) ||
			(gui && top_level->event_win == gui->win))
		{
			result = 0;
		}
		else
		if(active)
		{
			deactivate();
			result = 1;
		}
	}

	if(gui && top_level->event_win == gui->win)
	{
		if(!active)
		{
			top_level->deactivate();
			activate();
			selection_active = !popup;
		}

		last_selection2 = last_selection1;
		selection = cursor_item(top_level->cursor_x, top_level->cursor_y);
		last_selection1 = selection;

// Pressed over a title division
		if(in_division)
		{
			;
		}
		else
// Pressed over an item
		if(selection >= 0)
		{
			for(int j = 0; j < columns; j++)
			{
				if(selection_mode == LISTBOX_SINGLE)
				{
					for(int k = 0; k < data[0].total; k++)
						data[j].values[k]->selected = 0;

					data[j].values[selection]->selected = 1;
				}
				else
				{
					data[j].values[selection]->selected = !data[j].values[selection]->selected;
					new_value = data[j].values[selection]->selected;
				}
			}

			highlighted_item = -1;
			selection_active = 1;
			reset_query();
			redraw = 1;
			result = 1;
		}
		else
// Pressed over nothing
		{	for(int j = 0; j < columns; j++)
			{
				for(int k = 0; k < data[0].total; k++)
				{
					if(data[j].values[k]->selected)
					{	
						redraw = 1;
						result = 1;
						reset_query();
					}
					data[j].values[k]->selected = 0;
				}
			}
		}
	}

	if(redraw)
	{
		draw_items();
		gui->flash();
		selection_changed();
	}

	return result;
}

int BC_ListBox::button_release_event()
{
	int result = 0;
	selection_active = 0;
	new_value = 0;
	int cursor_x, cursor_y;
	Window tempwin;

	in_division = 0;

// Popup window
	if(popup)
	{
		hide_tooltip();
		button_releases++;
		if(status == LISTBOX_DN)
		{
			status = LISTBOX_HIGH;
			draw_face();
			result = 1;
		}

// Second button release inside button
		if(top_level->event_win == win && 
			cursor_inside() && 
			button_releases > 1)
		{
			deactivate();
			result = 1;
		}
// First button release inside button
		else
		if(top_level->event_win == win && cursor_inside())
		{
			result = 1;
		}
// Button release in popup window
		else
		if(gui && 
			(top_level->event_win == win || top_level->event_win == gui->win))
		{
			XTranslateCoordinates(top_level->display, 
				top_level->event_win, 
				gui->win, 
				top_level->cursor_x, 
				top_level->cursor_y, 
				&cursor_x, 
				&cursor_y, 
				&tempwin);

			selection = cursor_item(cursor_x, cursor_y);

			if(selection >= 0)
			{
				handle_event();
			}
			deactivate();
			result = 1;
		}
// Button release outside all ranges
		else
		if(active)
		{
			deactivate();
			result = 1;
		}
	}
	else
// No popup window
	if(gui &&
		top_level->event_win == gui->win &&
		top_level->get_double_click() &&
		last_selection2 == last_selection1)
	{
		handle_event();
		result = 1;
	}
	return result;
}

int BC_ListBox::get_title_h()
{
	if(display_format != LISTBOX_ICONS)
		return column_titles ? (get_text_height(MEDIUMFONT) + 4) : 0;
	else
		return 0;
}

void BC_ListBox::update_cursor(int in_division)
{
	if(in_division && current_cursor == ARROW_CURSOR)
	{
		current_cursor = HSEPARATE_CURSOR;
		if(popup)
			gui->set_cursor(current_cursor);
		else
			set_cursor(current_cursor);
	}
	else
	if(!in_division && current_cursor == HSEPARATE_CURSOR)
	{
		current_cursor = ARROW_CURSOR;
		if(popup)
			gui->set_cursor(current_cursor);
		else
			set_cursor(current_cursor);
	}
}

int BC_ListBox::cursor_motion_event()
{
	int selection = -1, redraw = 0, result = 0;
	int cursor_x, cursor_y;
	Window tempwin;

	if(popup && 
		top_level->event_win == win && 
		status == LISTBOX_DN && 
		!cursor_inside())
	{
		status = LISTBOX_UP;
		draw_face();
	}

	if(gui && 
		(top_level->event_win == win || 
		(popup && top_level->event_win == gui->win)))
	{
		XTranslateCoordinates(top_level->display, 
			top_level->event_win, 
			gui->win, 
			top_level->cursor_x, 
			top_level->cursor_y, 
			&cursor_x, 
			&cursor_y, 
			&tempwin);

		result = 1;
		selection = cursor_item(cursor_x, cursor_y);

// Cursor just moved in after pressing popup button
		if(top_level->get_button_down() && selection >= 0 && !selection_active) 
			selection_active = 1;

//printf("BC_ListBox::cursor_motion_event %d %d\n", selection, selection_active);
// Move division
		if(in_division && top_level->get_button_down())
		{
			column_width[in_division - 1] = cursor_x - xposition - get_column_offset(in_division - 1);
			column_width_boundaries();
// Update x coords of items
			set_item_coords();
			redraw = 1;
		}
		else
// Cursor is inside and selecting an item
		if(selection >= 0 && selection_active)
		{
			for(int j = 0; j < columns; j++)
			{
				if(selection_mode == LISTBOX_SINGLE)
				{
					for(int k = 0; k < data[0].total; k++)
					{
						if(k != selection && data[j].values[k]->selected)
						{
							redraw = 1;
							data[j].values[k]->selected = 0;
						}
						else
						if(k == selection && !data[j].values[k]->selected)
						{
							redraw = 1;
							data[j].values[selection]->selected = 1;
						}
					}
				}
				else
				{
					if(data[j].values[selection]->selected != new_value)
					{
						data[j].values[selection]->selected = new_value;
						redraw = 1;
					}
				}
			}
		}
		else
// Test if cursor moved over a title division
		{
			in_division = 0;

			if(column_titles && cursor_y < get_title_h())
			{

				for(int i = 1; i < columns; i++)
				{
					if(cursor_x > -xposition + get_column_offset(i) - 5 &&
						cursor_x <  -xposition + get_column_offset(i) + 5)
					{
						in_division = i;
						break;
					}
				}
			}

			update_cursor(in_division);
		}

		if(top_level->get_button_down() && selection_active)
		{
			test_drag_scroll(redraw, cursor_x, cursor_y);
		}
		else
		if(highlighted_item != selection)
		{
			highlighted_item = selection;
			redraw = 1;
		}
	}

	if(redraw)
	{
		fix_positions();
		draw_items();
		update_scrollbars();
		gui->flash();
		if(selection_active) selection_changed();
		return 1;
	}
	else
	if(!result && highlighted_item >= 0)
	{
		highlighted_item = -1;
		draw_items();
		gui->flash();
		return 0;
	}
	return result;
}

int BC_ListBox::drag_start_event()
{
	if(gui && 
		top_level->event_win == gui->win && 
		allow_drag)
	{
		selection = cursor_item(top_level->cursor_x, top_level->cursor_y);
		if(selection >= 0)
		{
			BC_Pixmap *pixmap = get_item_pixmap(selection) ? get_item_pixmap(selection) : drag_icon;
			drag_popup = new BC_DragWindow(this, 
				pixmap, 
				get_cursor_x() - pixmap->get_w() / 2,
				get_cursor_y() - pixmap->get_h() / 2);
			return 1;
		}
	}
	return 0;
}

int BC_ListBox::drag_motion_event()
{
	if(drag_popup)
	{
		int redraw = 0;
		test_drag_scroll(redraw, top_level->cursor_x, top_level->cursor_y);
		if(redraw)
		{
			fix_positions();
			draw_items();
			update_scrollbars();
			gui->flash();
		}

		return drag_popup->cursor_motion_event();
		return 1;
	}
	return 0;
}

int BC_ListBox::drag_stop_event()
{
	if(drag_popup)
	{
// Inside window boundary
		if(top_level->cursor_x > 0 && 
			top_level->cursor_x < gui->get_w() - drag_popup->get_w() / 2 && 
			top_level->cursor_y > 0 &&
			top_level->cursor_y < gui->get_h() - drag_popup->get_h() / 2)
		{
			data[0].values[selection]->x = top_level->cursor_x + drag_popup->get_offset_x() - LISTBOX_MARGIN - 2 + xposition;
			data[0].values[selection]->y = top_level->cursor_y + drag_popup->get_offset_y() - LISTBOX_MARGIN - 2 + yposition;
			draw_items();
			gui->flash();
		}
		else
			drag_popup->drag_failure_event();

		delete drag_popup;
		drag_popup = 0;
		selection_active = 0;
		new_value = 0;
		return 1;
	}
	return 0;
}

BC_DragWindow* BC_ListBox::get_drag_popup()
{
	return drag_popup;
}

int BC_ListBox::translation_event()
{
	if(popup && gui)
	{
		int new_x = gui->get_x() + (top_level->last_translate_x - top_level->x);
		int new_y = gui->get_y() + (top_level->last_translate_y - top_level->y);

		gui->reposition_window(new_x, new_y);
		
	}
	return 0;
}

int BC_ListBox::reposition_window(int x, int y, int w, int h)
{
	if(w != -1)
	{
		if(!popup)
		{
			popup_w = w;
			popup_h = h;
			if(xscrollbar)
				xscrollbar->reposition_window(get_xscroll_x(), 
					get_xscroll_y(), 
					get_xscroll_width(),
					SCROLL_SPAN);
			if(yscrollbar)
				yscrollbar->reposition_window(get_yscroll_x(), 
					get_yscroll_y(), 
					SCROLL_SPAN,
					get_yscroll_height());
		}
	}

	BC_WindowBase::reposition_window(x, y, w, h);
	draw_face();
	draw_items();
	flash();
	return 0;
}

int BC_ListBox::deactivate()
{
	if(active)
	{
		active = 0;
		if(popup)
		{
			if(gui) delete gui;
			xscrollbar = 0;
			yscrollbar = 0;
			gui = 0;
		}
		top_level->active_subwindow = 0;
	}
	return 0;
}

int BC_ListBox::activate()
{
	if(!active)
	{
		top_level->active_subwindow = this;
		active = 1;
		button_releases = 0;

		if(popup)
		{
			Window tempwin;
			int new_x, new_y;
			XTranslateCoordinates(top_level->display, 
				parent_window->win, 
				top_level->rootwin, 
				get_x() - popup_w + get_w(), 
				get_y() + get_h(), 
				&new_x, 
				&new_y, 
				&tempwin);

			if(new_x < 0) new_x = 0;
			if(new_y + popup_h > top_level->get_root_h()) new_y -= get_h() + popup_h;

			add_subwindow(gui = new BC_Popup(this, 
				new_x, 
				new_y, 
				popup_w, 
				popup_h, 
				-1,
				0,
				0));
			draw_items();
			gui->flash();
		}
	}
	return 0;
}

int BC_ListBox::keypress_event()
{
	if(!active) return 0;
	
	int result = 0, redraw = 0, done, view_items = view_h / get_text_height(MEDIUMFONT);
	int new_item = -1, new_selection = 0;
	switch(top_level->get_keypress())
	{
		case ESC:
		case RETURN:
			top_level->deactivate();
			result = 0;
			break;

		case UP:
			if(selection_mode == LISTBOX_SINGLE)
			{
				done = 0;
				for(int i = data[0].total - 1; !done && i > 0; i--)
				{
					if(data[0].values[i]->selected)
					{
						new_item = i - 1;
						for(int j = 0; j < columns; j++)
						{
							data[j].values[i]->selected = 0;
							data[j].values[new_item]->selected = 1;
							redraw = 1;
							result = 1;
							new_selection = 1;
							center_selection(new_item);
						}
						done = 1;
					}
				}
			}
			break;

		case DOWN:
			if(selection_mode == LISTBOX_SINGLE)
			{
				done = 0;
				for(int i = 0; !done && i < data[0].total - 1; i++)
				{
					if(data[0].values[i]->selected)
					{
						new_item = i + 1;
						for(int j = 0; j < columns; j++)
						{
							data[j].values[i]->selected = 0;
							data[j].values[new_item]->selected = 1;
							redraw = 1;
							result = 1;
							new_selection = 1;
							center_selection(new_item);
						}
						done = 1;
					}
				}
			}
			
			break;

		case PGUP:
			if(selection_mode == LISTBOX_SINGLE)
			{
				done = 0;
				for(int i = data[0].total - 1; !done && i > 0; i--)
				{
					if(data[0].values[i]->selected)
					{
						new_item = i - view_items;
						if(new_item < 0) new_item = 0;

						for(int j = 0; j < columns; j++)
						{
							data[j].values[i]->selected = 0;
							data[j].values[new_item]->selected = 1;
							redraw = 1;
							result = 1;
							new_selection = 1;
							center_selection(new_item);
						}
						done = 1;
					}
				}
			}
			break;

		case PGDN:
			if(selection_mode == LISTBOX_SINGLE)
			{
				done = 0;
				for(int i = 0; !done && i < data[0].total - 1; i++)
				{
					if(data[0].values[i]->selected)
					{
						new_item = i + view_items;
						if(new_item >= data[0].total) new_item = data[0].total - 1;

						for(int j = 0; j < columns; j++)
						{
							data[j].values[i]->selected = 0;
							data[j].values[new_item]->selected = 1;
							redraw = 1;
							result = 1;
							new_selection = 1;
							center_selection(new_item);
						}
						done = 1;
					}
				}
			}
			break;

		case LEFT:
			xposition -= 10;
			redraw = 1;
			result = 1;
			break;

		case RIGHT:
			xposition += 10;
			redraw = 1;
			result = 1;
			break;

		default:
			if(top_level->get_keypress() > 30 && top_level->get_keypress() < 127)
			{
				int query_len = strlen(query);
				query[query_len++] = top_level->get_keypress();
				query[query_len] = 0;
				query_list();
			}
			else
			if(top_level->get_keypress() == BACKSPACE)
			{
				int query_len = strlen(query);
				if(query_len > 0) query[--query_len] = 0;
				query_list();
			}

			redraw = 1;
			result = 1;
			break;
	}

	if(redraw)
	{
		fix_positions();
		draw_items();
		update_scrollbars();
		gui->flash();
	}
	
	if(new_selection)
	{
		selection_changed();
	}

	return result;
}

int BC_ListBox::get_scrollbars()
{
	int h_needed = get_items_height();
	int w_needed = get_items_width();

	title_h = get_title_h();

	view_h = popup_h - title_h - 4;
	view_w = popup_w - 4;

// Create scrollbars as needed
	for(int i = 0; i < 2; i++)
	{
		if(w_needed > view_w)
		{
			need_xscroll = 1;
			view_h = popup_h - title_h - SCROLL_SPAN - 4;
		}
		else
		{
			need_xscroll = 0;
		}

		if(h_needed > view_h)
		{
			need_yscroll = 1;
			view_w = popup_w - SCROLL_SPAN - 4;
		}
		else
		{
			need_yscroll = 0;
		}
	}

// Update subwindow size
	int new_w = popup_w, new_h = popup_h;
	if(need_xscroll) new_h -= SCROLL_SPAN;
	if(need_yscroll) new_w -= SCROLL_SPAN;

	if(!popup)
		if(new_w != BC_WindowBase::get_w() || new_h != BC_WindowBase::get_h())
			gui->resize_window(new_w, new_h);

	BC_WindowBase *destination = (popup ? gui : parent_window);
	if(need_xscroll)
	{
		if(!xscrollbar)
		{
			destination->add_subwindow(xscrollbar = 
				new BC_ListBoxXScroll(this, 
					w_needed, 
					view_w, 
					xposition));
			xscrollbar->bound_to = this;
		}
	}
	else
	{
		if(xscrollbar) delete xscrollbar;
		xscrollbar = 0;
	}

	if(need_yscroll)
	{
		if(!yscrollbar)
		{
			destination->add_subwindow(yscrollbar = 
				new BC_ListBoxYScroll(this, 
					h_needed, 
					view_h, 
					yposition));
			yscrollbar->bound_to = this;
		}
	}
	else
	{
		if(yscrollbar) delete yscrollbar;
		yscrollbar = 0;
	}
	
	if(!bg_surface ||
		view_w + 4 != bg_surface->get_w() ||
		view_h + 4 != bg_surface->get_h())
	{
		if(bg_surface) delete bg_surface;
		bg_surface = new BC_Pixmap(gui, view_w + 4, view_h + 4);
	}

	return 0;
}

BC_Pixmap* BC_ListBox::get_bg_surface()
{
	return bg_surface;
}


void BC_ListBox::draw_background()
{
	set_color(WHITE);
	draw_box(0, 0, bg_surface->get_w(), bg_surface->get_h(), bg_surface);
	if(bg_pixmap)
		bg_surface->draw_pixmap(bg_pixmap,
			bg_surface->get_w() - top_level->get_resources()->listbox_bg->get_w(),
			0);
}

void BC_ListBox::clear_listbox(int x, int y, int w, int h)
{
	gui->draw_pixmap(bg_surface, 
		x, 
		y, 
		w, 
		h,
		x,
		y - title_h);
}

int BC_ListBox::draw_items()
{
	if(gui)
	{
		get_scrollbars();
		draw_background();

// Icon display
		if(display_format == LISTBOX_ICONS)
		{
			clear_listbox(2, 2 + title_h, view_w, view_h);

			set_font(MEDIUMFONT);
			for(int i = 0; i < data[0].total; i++)
			{
				if(get_item_x(0, i) >= -get_item_w(0, i) && 
					get_item_x(0, i) < view_w &&
					get_item_y(0, i) >= -get_item_h(0, i) + title_h &&
					get_item_y(0, i) < view_h + title_h)
				{
					int item_color = get_item_highlight(0, i);
					int icon_x, icon_y, icon_w, icon_h;
					int text_x, text_y, text_w, text_h;

// Draw highlights					
					get_icon_mask(0, i, icon_x, icon_y, icon_w, icon_h);
					get_text_mask(0, i, text_x, text_y, text_w, text_h);
					if(item_color != WHITE)
					{
						gui->set_color(BLACK);
						gui->draw_rectangle(icon_x, icon_y, icon_w, icon_h);
						gui->set_color(item_color);
						gui->draw_box(icon_x + 1, icon_y + 1, icon_w - 2, icon_h - 2);
						gui->set_color(BLACK);
						gui->draw_rectangle(text_x, text_y, text_w, text_h);
						gui->set_color(item_color);
						gui->draw_box(text_x + 1, text_y + 1, text_w - 2, text_h - 2);
					
						if(icon_position == ICON_LEFT)
							gui->draw_box(text_x - 1, text_y + 1, 2, text_h - 2);
						else
						if(icon_position == ICON_TOP)
							gui->draw_line(text_x + 1, text_y, text_x + icon_w - 2, text_y);
					}

// Draw icons
					gui->set_color(get_item_color(0, i));
					if(get_item_pixmap(i))
						get_item_pixmap(i)->write_drawable(gui->pixmap, icon_x + ICON_MARGIN, icon_y + ICON_MARGIN);
					gui->draw_text(text_x + ICON_MARGIN, 
						text_y + ICON_MARGIN + get_text_ascent(MEDIUMFONT), 
						data[0].values[i]->text);
				}
			}
		}
		else
// Text display
		if(display_format == LISTBOX_TEXT)
		{
			for(int j = 0; j < columns; j++)
			{
//printf("BC_ListBox::draw_items %d %d %d\n", j, popup_w, get_column_offset(j));
				clear_listbox(2 + get_column_offset(j) - xposition, 
					2 + title_h, 
					get_column_width(j), 
					view_h);

				set_font(MEDIUMFONT);
				for(int i = 0; i < data[j].total; i++)
				{
					if(get_item_y(0, i) >= -get_item_h(0, i) + title_h &&
						get_item_y(0, i) < view_h + title_h)
					{
						int row_color = get_item_highlight(0, i);
						int x, y, w, h;

						get_text_mask(j, i, x, y, w, h);

						if(row_color != WHITE) 
						{
							gui->set_color(row_color);
							gui->draw_box(x, 
								y, 
								get_column_width(j), 
								h);
							gui->set_color(BLACK);
							gui->draw_line(x, y, x + get_column_width(j) - 1, y);
							gui->draw_line(x, y + get_text_height(MEDIUMFONT), x + get_column_width(j) - 1, y + get_text_height(MEDIUMFONT));
						}

						gui->set_color(get_item_color(j, i));

						gui->draw_text(x + 2 + LISTBOX_MARGIN, 
							y + get_text_ascent(MEDIUMFONT), 
							data[j].values[i]->text);
					}
				}
			}
		}

// Draw titles
		if(column_titles && display_format != LISTBOX_ICONS)
		{
			for(int i = 0; i < columns; i++)
			{
				gui->draw_3d_box(get_column_offset(i) - xposition + 2, 
					2, 
					get_column_width(i), 
					title_h, 
					top_level->get_resources()->button_light, 
					top_level->get_resources()->button_up, 
					top_level->get_resources()->button_up, 
					top_level->get_resources()->button_shadow,
					BLACK);

				gui->set_color(BLACK);
				gui->draw_text(-xposition + get_column_offset(i) + LISTBOX_MARGIN + 2, 
					2 + get_text_ascent(MEDIUMFONT), column_titles[i]);
			}
		}

// Clear garbage
		if(xscrollbar && yscrollbar && popup)
		{
			gui->draw_top_background(parent_window, 
				popup_w - SCROLL_SPAN, 
				popup_h - SCROLL_SPAN, 
				SCROLL_SPAN, 
				SCROLL_SPAN);
		}
		draw_border();
	}

	return 0;
}

int BC_ListBox::draw_border()
{
	gui->draw_3d_border(0, 
		0, 
		view_w + 4, 
		view_h + title_h + 4, 
		top_level->get_resources()->button_shadow, 
		highlighted ? RED : BLACK, 
		highlighted ? RED : top_level->get_resources()->button_up, 
		top_level->get_resources()->button_light);
	return 0;
}
