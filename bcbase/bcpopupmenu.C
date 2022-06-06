#include <string.h>
#include "bccolors.h"
#include "bcfont.h"
#include "bcpopupmenu.h"
#include "bcresources.h"

BC_PopupMenu::BC_PopupMenu(int x, int y, int w, const char *text, int small, int floating)
 : BC_Tool(x, y, w, 25)
{
	highlighted = popup_down = 0;
	strcpy(this->text, text);
	menu_popup = 0;
	this->small = small;
	this->floating = floating;
}

BC_PopupMenu::~BC_PopupMenu()
{
	if(menu_popup) delete menu_popup;
	menu_popup = 0;
}

char* BC_PopupMenu::get_text()
{
	return text;
}

int BC_PopupMenu::create_tool_objects()
{
	if(!floating)
	{
		if(small)
			h = get_text_height(MEDIUMFONT) + 4;
		else
			h = get_text_height(MEDIUMFONT) + 8;
	}

	menu_popup = new BC_MenuPopup(subwindow, this, top_level);

	if(!floating)
	{
		create_window(x, y, w, h, top_level->get_resources()->menu_up);
		draw_text();
	}

	add_items();  // Virtual function declared by user.
return 0;
}

int BC_PopupMenu::add_item(BC_PopupItem *item)
{
	menu_popup->add_item(item);
return 0;
}

int BC_PopupMenu::delete_item(BC_PopupItem *item)
{
	menu_popup->delete_item(item);
return 0;
}

int BC_PopupMenu::total_items()
{
	return menu_popup->total_items();
return 0;
}

int BC_PopupMenu::resize_tool(int x, int y, int w, int h)
{
	if(!floating)
	{
		resize_window(x, y, w, h);
		draw_text();
	}
return 0;
}

int BC_PopupMenu::resize_tool(int x, int y)
{
	if(!floating)
	{
		resize_window(x, y, w, h);
		draw_text();
	}
return 0;
}

int BC_PopupMenu::update(const char *text)
{
	if(!floating)
	{
		strcpy(this->text, text);
		draw_text();
	}
return 0;
}

int BC_PopupMenu::draw_text()
{
	if(floating) return 0;

	if(!popup_down)
	{
		if(highlighted) 
			draw_3d_big(0, 0, get_w(), get_h(), 
				top_level->get_resources()->menu_light,
				top_level->get_resources()->menu_up,
				top_level->get_resources()->menu_highlighted,
				top_level->get_resources()->menu_shadow,
				BLACK);
		else
			draw_3d_big(0, 0, get_w(), get_h(), 
				top_level->get_resources()->menu_light,
				top_level->get_resources()->menu_up,
				top_level->get_resources()->menu_up,
				top_level->get_resources()->menu_shadow,
				BLACK);
	}
	else
	{
			draw_3d_big(0, 0, get_w(), get_h(), 
				top_level->get_resources()->menu_shadow,
				BLACK,
				top_level->get_resources()->menu_down,
				top_level->get_resources()->menu_down,
				top_level->get_resources()->menu_light);
	}

	set_color(BLACK);
	set_font(MEDIUMFONT);
	if(small)       // a.k.a. plugin menu
	{
		BC_Tool::draw_text(3, get_h() - get_text_descent(MEDIUMFONT) - 2, text);
	}
	else
	{
		draw_center_text(get_w() / 2, get_h() - get_text_descent(MEDIUMFONT) - 4, text, MEDIUMFONT);
	}
	flash();
	return 0;
return 0;
}

int BC_PopupMenu::deactivate_()
{
	if(popup_down)
	{
		set_active_popupmenu(0);
		popup_down = 0;
		menu_popup->deactivate_menus();
		menu_popup->deactivate_menu();

		if(!floating) draw_text();    // draw the title
	}
return 0;
}

int BC_PopupMenu::button_press_()
{
	int result = 0;

	if(popup_down)
	{
// Menu is down so dispatch to popup.
		result = menu_popup->button_press_dispatch();
	}

	if(!result && !floating)
	{
// Either menu isn't down or menu didn't get it so try title.
		if(get_cursor_x() > 0 && get_cursor_x() < get_w() &&
			 get_cursor_y() > 0 && get_cursor_y() < get_h())
		{
			if(!popup_down)
			{
// Title activated
				button_releases = 0;
				activate_menu();
			}
			result = 1;
		}
	}
	return result;
return 0;
}

int BC_PopupMenu::activate_menu()
{
	button_releases = 0;
	menu_popup->activate_menu(x, y, w, h, 1, 1);
	popup_down = 1;
	set_active_popupmenu(this);
	if(!floating) draw_text();
return 0;
}

int BC_PopupMenu::activate_menu(int x, int y, BC_Tool *relative_tool)
{
	Window tempwin;
	int new_x, new_y;
	button_releases = 0;
	XTranslateCoordinates(top_level->display, relative_tool->win, top_level->rootwin, x, y, &new_x, &new_y, &tempwin);

	menu_popup->activate_menu(new_x, new_y, 0, 0, 0, 1);
	popup_down = 1;
	set_active_popupmenu(this);
return 0;
}

int BC_PopupMenu::deactivate_menu()
{
	deactivate_();
return 0;
}

int BC_PopupMenu::button_release_()
{
// try the title
	int result;
	result = 0;

	button_releases++;
	if(!floating && get_cursor_x() > 0 && get_cursor_x() < get_w() &&
		get_cursor_y() > 0 && get_cursor_y() < get_h())
	{
// Inside title
		if(button_releases >= 2)
		{
			highlighted = 1;
			deactivate_();
		}
		result = 1;
	}
	else
	if(popup_down && (result = menu_popup->button_release_dispatch()))
	{
// Inside the popup
		;		
	}
	else
	if(floating && button_releases < 2)
	{
// First release outside a floating menu
		result = 1;
	}

	if(!result && popup_down)
	{
// Button was released outside any menu.
		deactivate_();
		result = 1;
	}
	return result;
return 0;
}

int BC_PopupMenu::cursor_left_()
{
	if(highlighted && !floating)
	{
		if(get_cursor_x() < 0 || get_cursor_x() > get_w() ||
			 get_cursor_y() < 0 || get_cursor_y() > get_h())
		{
			highlighted = 0;
			draw_text();
		}
	}

// dispatch to popup
	if(popup_down)
	{
		menu_popup->cursor_left_dispatch();
	}
	return 0;
return 0;
}

int BC_PopupMenu::cursor_motion_()
{
	int result;
	result = 0;

// This menu is down.
	if(popup_down)
	{
		result = menu_popup->motion_event_dispatch();
	}

	if(!result && !floating)
	{
		if(highlighted)
		{
			if(get_cursor_x() < 0 || get_cursor_x() > get_w() ||
				 get_cursor_y() < 0 || get_cursor_y() > get_h())
			{
				highlighted = 0;
				draw_text();
			}
		}
		else
		{
			if(get_cursor_x() > 0 && get_cursor_x() < get_w() &&
				get_cursor_y() > 0 && get_cursor_y() < get_h())
			{
				top_level->unhighlight();
				highlighted = 1;
				draw_text();
				result = 1;
			}
		}
	}
	return result;
return 0;
}

int BC_PopupMenu::unhighlight_()
{
	int result;
	if(highlighted && !floating)
	{
		highlighted = 0;
		draw_text();
		result = 1;
	}

	if(popup_down)
	{
		result += menu_popup->unhighlight();
	}
	return result;
return 0;
}




// ================================== popup item

BC_PopupItem::BC_PopupItem(const char *text, int checked)
 : BC_MenuItem(text)
{
	set_checked(checked);
}

BC_PopupItem::~BC_PopupItem()
{
}

BC_PopupMenu* BC_PopupItem::get_menu()
{
	return menu_popup->menu_button;
}







// ================================== window

BC_MenuPopup::BC_MenuPopup()
{
}

BC_MenuPopup::BC_MenuPopup(BC_WindowBase *subwindow, BC_PopupMenu *menu_button, BC_Window *top_level)
{
	reset_parameters();
	this->subwindow = subwindow;
	this->menu_button = menu_button;
	this->top_level = top_level;
}

BC_MenuPopup::BC_MenuPopup(BC_WindowBase *subwindow, BC_Menu *menu_title, BC_Window *top_level)
{
	reset_parameters();
	this->subwindow = subwindow;
	this->menu_title = menu_title;
	this->top_level = top_level;
}

BC_MenuPopup::BC_MenuPopup(BC_WindowBase *subwindow, BC_MenuItem *menu_item, BC_Window *top_level)
{
	reset_parameters();
	this->subwindow = subwindow;
	this->menu_item = menu_item;
	this->top_level = top_level;
}

BC_MenuPopup::~BC_MenuPopup()
{
	while(menu_items.total)
	{
// Each menuitem recursively removes itself from the arraylist
		delete menu_items.values[0];
	}
}

int BC_MenuPopup::reset_parameters()
{
	menu_button = 0;
	menu_title = 0;
	menu_item = 0;
	popup = 0;
	active = 0;
return 0;
}

int BC_MenuPopup::add_item(BC_MenuItem *item)
{
	menu_items.append(item);
	item->top_level = top_level;
	item->menu_popup = this;
return 0;
}

int BC_MenuPopup::delete_item(BC_MenuItem *item)
{
	if(item)
		menu_items.remove(item);
	else
		menu_items.remove();
return 0;
}

int BC_MenuPopup::total_items()
{
	return menu_items.total;
return 0;
}

int BC_MenuPopup::activate_menu(int x, int y, int w, int h, int top_window_coords, int vertical_justify)
{
	Window tempwin;
	int new_x, new_y, top_w, top_h;
	top_w = top_level->get_top_w();
	top_h = top_level->get_top_h();

	get_dimensions();

	if(top_window_coords)
		XTranslateCoordinates(top_level->display, subwindow->win, top_level->rootwin, x, y, &new_x, &new_y, &tempwin);
	else
		{ new_x = x; new_y = y; }

// All coords are now relative to root window.
	if(vertical_justify)
	{
		this->x = new_x;
		this->y = new_y + h;
		if(this->x + this->w > top_w) this->x -= this->x + this->w - top_w; // Right justify
		if(this->y + this->h > top_h) this->y -= this->h + h; // Bottom justify
	}
	else
	{
		this->x = new_x + w;
		this->y = new_y;
		if(this->x + this->w > top_w) this->x = new_x - this->w;
		if(this->y + this->h > top_h) this->y = new_y + h - this->h;
	}

	active = 1;
	popup = new BC_Popup(top_level, top_level->get_resources()->menu_up, this->x, this->y, this->w, this->h, 0);
	draw_items();
return 0;
}

int BC_MenuPopup::deactivate_menu()
{
	if(popup) delete popup;
	popup = 0;
	active = 0;
	return 0;
return 0;
}

int BC_MenuPopup::deactivate_all()
{
	if(menu_button) menu_button->deactivate_();
	else
	if(menu_title) menu_title->deactivate_all();
	else
	if(menu_item) menu_item->deactivate_all();
	return 0;
return 0;
}

int BC_MenuPopup::deactivate_menus(BC_MenuPopup *exclude)
{
	for(int i = 0; i < menu_items.total; i++)
	{
		menu_items.values[i]->deactivate_menus(exclude);
	}
return 0;
}

int BC_MenuPopup::get_dimensions()
{
	int widest_text = 10, widest_key = 10;
	int text_w, key_w;
	int i = 0;

	h = 2;  // pad for border
// Set up parameters in each item and get total h. 
	for(i = 0; i < menu_items.total; i++)
	{
		text_w = 10 + XTextWidth(top_level->get_font(MEDIUMFONT), menu_items.values[i]->text, strlen(menu_items.values[i]->text));
		if(menu_items.values[i]->checked) text_w += 20;
		key_w = 10 + XTextWidth(top_level->get_font(MEDIUMFONT), menu_items.values[i]->hotkey_text, strlen(menu_items.values[i]->hotkey_text));
		if(text_w > widest_text) widest_text = text_w;
		if(key_w > widest_key) widest_key = key_w;

		if(!strcmp(menu_items.values[i]->text, "-")) 
			menu_items.values[i]->h = 5;
		else
			menu_items.values[i]->h = top_level->get_font(MEDIUMFONT)->descent + top_level->get_font(MEDIUMFONT)->ascent + 4;

		menu_items.values[i]->y = h;
		menu_items.values[i]->highlighted = 0;
		menu_items.values[i]->down = 0;
		h += menu_items.values[i]->h;
	}
	w = widest_text + widest_key + 10;
	key_x = widest_text + 5; // pad for division
	h += 2;  // pad for border
return 0;
}

int BC_MenuPopup::draw_items()
{
	int i;
	popup->draw_3d_big(0, 0, w, h, 
		top_level->get_resources()->menu_light,
		top_level->get_resources()->menu_up,
		top_level->get_resources()->menu_up,
		top_level->get_resources()->menu_shadow,
		BLACK);

	for(i = 0; i < menu_items.total; i++)
	{
		menu_items.values[i]->draw();
	}
	popup->flash();
return 0;
}

int BC_MenuPopup::motion_event_dispatch()
{
	int cursor_x, cursor_y, i, result;
	Window tempwin;

	result = 0;
	if(popup)
	{
		XTranslateCoordinates(top_level->display, top_level->win, popup->win, 
			top_level->get_cursor_x(), top_level->get_cursor_y(), &cursor_x, &cursor_y, &tempwin);

		result = 0;
		for(i = 0; i < menu_items.total && !result; i++)
		{
			result = menu_items.values[i]->motion_event_dispatch(cursor_x, cursor_y);
//printf("BC_MenuPopup::motion_event_dispatch %d %d\n", i, result);
		}

		if(result) draw_items();
	}
	return result;
return 0;
}

int BC_MenuPopup::unhighlight()
{
//printf("BC_MenuPopup::unhighlight\n");
	for(int i = 0; i < menu_items.total; i++)
	{
		menu_items.values[i]->unhighlight();
	}
	if(popup) draw_items();
	return 0;
return 0;
}

int BC_MenuPopup::button_press_dispatch()
{
	int cursor_x, cursor_y, i, result;
	Window tempwin;

	result = 0;
	if(popup)
	{
		XTranslateCoordinates(top_level->display, top_level->win, popup->win, 
			top_level->get_cursor_x(), top_level->get_cursor_y(), &cursor_x, &cursor_y, &tempwin);

//printf("BC_MenuPopup::button_press_dispatch %d %d\n", cursor_x, cursor_y);
		result = 0;
		for(i = 0; i < menu_items.total && !result; i++)
		{
			result = menu_items.values[i]->button_press_dispatch(cursor_x, cursor_y);
		}

		if(result) draw_items();
	}
	return result;
return 0;
}

int BC_MenuPopup::button_release_dispatch()
{
	int cursor_x, cursor_y, i, result;
	Window tempwin;

	result = 0;
	if(popup)
	{
		XTranslateCoordinates(top_level->display, top_level->win, popup->win, 
			top_level->get_cursor_x(), top_level->get_cursor_y(), &cursor_x, &cursor_y, &tempwin);

		result = 0;
		for(i = 0; i < menu_items.total && !result; i++)
		{
			result = menu_items.values[i]->button_release_dispatch(cursor_x, cursor_y);
		}
	}
	return result;
return 0;
}

int BC_MenuPopup::cursor_left_dispatch()
{
	return 0;
return 0;
}

int BC_MenuPopup::key_press_dispatch()
{
	int result, i;
	result = 0;
	for(i = 0; i < menu_items.total && !result; i++)
	{
		result = menu_items.values[i]->key_press_dispatch();
	}
	return result;
return 0;
}


BC_SubMenu::BC_SubMenu()
 : BC_MenuPopup()
{
}

BC_SubMenu::~BC_SubMenu()
{
}


int BC_SubMenu::add_submenuitem(BC_MenuItem *item)
{
	add_item(item);
return 0;
}

