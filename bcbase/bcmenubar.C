#include <string.h>
#include "bccolors.h"
#include "bcfont.h"
#include "bcmenubar.h"
#include "bcpopupmenu.h"
#include "bcresources.h"
#include "bcwindow.h"

// ================================== Menu bar.

BC_MenuBar::BC_MenuBar(int x, int y, int w)
 : BC_Tool(x, y, w, 25)
{
// Height is really determined by the font in create_tool_objects.
	button_down = 0;
	enabled = 1;
	active = 0;
}

BC_MenuBar::~BC_MenuBar()
{
// Delete all titles.
	for(int i = 0; i < menu_titles.total; i++) delete menu_titles.values[i];
	menu_titles.remove_all();
}

int BC_MenuBar::create_tool_objects()
{
// Get the height + 4  for menubar bevel + 4 for menu title bevel.
	create_window(x, y, w, get_text_height(MEDIUMFONT) + 8, top_level->get_resources()->menu_up);
	draw();
return 0;
}

int BC_MenuBar::add_menu(BC_Menu* menu)
{
	int x, w;
	if(menu_titles.total == 0)
	{
		x = 2;
	}
	else
	{
		x = menu_titles.values[menu_titles.total - 1]->x + 
			menu_titles.values[menu_titles.total - 1]->w;
	}
	
	w = get_text_width(MEDIUMFONT, menu->text) + 20;

// add pointer	
	menu_titles.append(menu);
	menu->create_objects(this, top_level, x, 2, w, get_h() - 4); // initialize and draw
return 0;
}

int BC_MenuBar::button_press_()
{
	int result;
	result = 0;
	for(int i = 0; i < menu_titles.total && enabled && !result; i++)
	{
		result = menu_titles.values[i]->button_press_dispatch();
	}
	return result;
return 0;
}

int BC_MenuBar::button_release_()
{
	int result;
	result = 0;
	button_down = 0;
	button_releases++;
	for(int i = 0; i < menu_titles.total && enabled; i++)
	{
		result += menu_titles.values[i]->button_release_dispatch();
	}

	if(!result && active)
	{
// Button was released outside any menu.
		deactivate_();
		result = 1;
	}
	return result;
return 0;
}

int BC_MenuBar::unhighlight_()
{
//printf("BC_MenuBar::unhighlight_\n");
	for(int i = 0; i < menu_titles.total; i++)
	{
		menu_titles.values[i]->unhighlight();
	}
	return 0;
return 0;
}

int BC_MenuBar::activate_menus()
{
	active = 1;
	set_active_menubar(this);
return 0;
}

int BC_MenuBar::resize_event_(int w, int h)
{
	resize_window(get_x(), get_y(), w, get_h());
	draw();
	for(int i = 0; i < menu_titles.total; i++)
	{
		menu_titles.values[i]->draw_title();
	}
return 0;
}

int BC_MenuBar::draw()
{
	int lx,ly,ux,uy;
	int h, w;
	h = get_h();
	w = get_w();
	h--; w--;

	lx = 1;  ly = 1;
	ux = w-1;  uy = h-1;

	set_color(top_level->get_resources()->menu_light);
	draw_line(0, 0, 0, uy);
	draw_line(0, 0, ux, 0);

	set_color(top_level->get_resources()->menu_shadow);
	draw_line(ux, ly, ux, uy);
	draw_line(lx, uy, ux, uy);
	set_color(BLACK);
	draw_line(w, 0, w, h);
	draw_line(0, h, w, h);

	flash();
return 0;
}

int BC_MenuBar::deactivate_()
{
	for(int i = 0; i < menu_titles.total; i++)
	{
		menu_titles.values[i]->deactivate_menus();
	}
	set_active_menubar(0);
	active = 0;
return 0;
}

int BC_MenuBar::activate_()
{
	set_active_menubar(this);
	active = 1;
return 0;
}

int BC_MenuBar::enable()
{
	enabled = 1;
return 0;
}

int BC_MenuBar::disable()
{
	enabled = 0;
return 0;
}

int BC_MenuBar::keypress_event_()
{
	int result;
	result = 0;
	if(enabled && (!get_active_tool() || !get_active_tool()->uses_text()))
	{
		for(int i = 0; i < menu_titles.total && top_level->get_key_pressed() && !result; i++)
		{
			result = menu_titles.values[i]->key_press_dispatch();
		}
	}
	return result;
return 0;
}

int BC_MenuBar::expose_event_dispatch()
{
	if(get_event_win() == win) flash();

// dispatch to menus
	for(int i = 0; i < menu_titles.total; i++)
	{
		menu_titles.values[i]->expose_event_dispatch();
	}
return 0;
}

int BC_MenuBar::cursor_left_()
{
	for(int i = 0; i < menu_titles.total && enabled; i++)
	{
		menu_titles.values[i]->cursor_left_dispatch();
	}
return 0;
}

int BC_MenuBar::cursor_motion_()
{
	int result;
	result = 0;
	for(int i = 0; i < menu_titles.total && enabled && !result; i++)
	{
		result = menu_titles.values[i]->motion_event_dispatch();
	}
	return result;
return 0;
}

// ======================================== Menu

BC_Menu::BC_Menu(const char *text)
{
	active = 0;
	strcpy(this->text, text);
	highlighted = 0;
	menu_bar = 0;
}

BC_Menu::~BC_Menu()
{
	delete menu_popup;
}

int BC_Menu::create_objects(BC_MenuBar *menu_bar, BC_Window *top_level, int x, int y, int w, int h)
{
	this->x = x; this->y = y; this->w = w; this->h = h;
	this->menu_bar = menu_bar;
	this->top_level = top_level;
	menu_popup = new BC_MenuPopup(menu_bar->subwindow, this, top_level);
	draw_title();
return 0;
}

int BC_Menu::button_press_dispatch()
{
	int result;
	result = 0;
	if(active)
	{
// Menu is down so dispatch to popup.
		result = menu_popup->button_press_dispatch();
	}

	if(!result)
	{
// Either menu isn't down or menu didn't get it so try title.
		if(menu_bar->get_cursor_x() > x && menu_bar->get_cursor_x() < x + w &&
			 menu_bar->get_cursor_y() > y && menu_bar->get_cursor_y() < y + h)
		{
			if(!active)
			{
// Title activated
				menu_bar->deactivate_();
				menu_bar->unhighlight();
				menu_bar->button_releases = 0;
				menu_bar->activate_menus();
				activate_menu();
			}
			result = 1;
		}
	}
	return result;
return 0;
}

int BC_Menu::activate_menu()
{
	menu_popup->activate_menu(x, y, w, h, 1, 1);
	active = 1;
	draw_title();
return 0;
}

int BC_Menu::draw_title()
{
	if(active && menu_popup)
	{
// Menu is pulled down and title is recessed.
		menu_bar->draw_3d_big(x, y, w, h, 
			top_level->get_resources()->menu_shadow, 
			BLACK, 
			top_level->get_resources()->menu_down,
			top_level->get_resources()->menu_down,
			top_level->get_resources()->menu_light);
	}
	else
	{
// Menu is not pulled down.
		if(highlighted)
		{
			menu_bar->set_color(top_level->get_resources()->menu_highlighted);
		}
		else
		{
			menu_bar->set_color(top_level->get_resources()->menu_up);
		}
	
		menu_bar->draw_box(x, y, w, h);
	}
	menu_bar->set_color(BLACK);
	menu_bar->set_font(MEDIUMFONT);
	menu_bar->draw_text(x + 10, h - menu_bar->get_text_descent(MEDIUMFONT), text);
	menu_bar->flash();
return 0;
}

int BC_Menu::set_done(int return_value)
{
	top_level->set_done(return_value);
return 0;
}

int BC_Menu::add_menuitem(BC_MenuItem* menuitem)
{
	menu_popup->add_item(menuitem);
return 0;
}

int BC_Menu::total_menuitems()
{
	return menu_popup->total_items();
return 0;
}

int BC_Menu::remove_menuitem(BC_MenuItem *item)
{
	menu_popup->delete_item(item);
return 0;
}

int BC_Menu::expose_event_dispatch()
{
return 0;
}

int BC_Menu::deactivate_menu()
{
	if(active)
	{
		menu_popup->deactivate_menus();
		menu_popup->deactivate_menu();
		active = 0;
		draw_title();
	}
return 0;
}

int BC_Menu::deactivate_menus()
{
	deactivate_menu();
return 0;
}

// Deactivate all menus starting from the top.
int BC_Menu::deactivate_all()
{
	menu_bar->deactivate_();
return 0;
}

int BC_Menu::cursor_left_dispatch()
{
	if(highlighted)
	{
		if(menu_bar->get_cursor_x() < x || menu_bar->get_cursor_x() > x + w ||
			 menu_bar->get_cursor_y() < y || menu_bar->get_cursor_y() > h)
		{
			highlighted = 0;
			draw_title();
		}
	}

// dispatch to popup
	if(active)
	{
		menu_popup->cursor_left_dispatch();
	}
return 0;
}

int BC_Menu::motion_event_dispatch()
{
	int result;
	result = 0;

// This menu is down.
	if(active && menu_popup)
	{
		result = menu_popup->motion_event_dispatch();
	}

	if(!result)
	{
// Menu is down somewhere
		if(menu_bar->active)
		{
// cursor has just moved into this title
			if(menu_bar->get_cursor_x() > x && menu_bar->get_cursor_x() < x + w &&
				 menu_bar->get_cursor_y() > y && menu_bar->get_cursor_y() < y + h && 
				 !active)
			{
				menu_bar->deactivate_();
				menu_bar->activate_menus();
				activate_menu();
				result = 1;
			}
		}
// No menu down.
		else
		{
			if(highlighted)
			{
				if(menu_bar->get_cursor_x() < x || menu_bar->get_cursor_x() > x + w ||
					 menu_bar->get_cursor_y() < y || menu_bar->get_cursor_y() > y + h)
				{
					highlighted = 0;
					draw_title();
				}
			}
			else
			{
				if(menu_bar->get_cursor_x() > x && menu_bar->get_cursor_x() < x + w &&
					menu_bar->get_cursor_y() > y && menu_bar->get_cursor_y() < y + h)
				{
					top_level->unhighlight();
					highlighted = 1;
					draw_title();
					result = 1;
				}
			}
		}
	}
	return result;
return 0;
}

int BC_Menu::unhighlight()
{
	if(highlighted)
	{
		highlighted = 0;
		draw_title();
	}
	if(menu_popup->popup)
	{
		menu_popup->unhighlight();
	}
	return 0;
return 0;
}

int BC_Menu::button_release_dispatch()
{
// try the title
	if(menu_bar->get_cursor_x() > x && menu_bar->get_cursor_x() < x + w &&
		menu_bar->get_cursor_y() > y && menu_bar->get_cursor_y() < y + h)
	{
		if(menu_bar->button_releases >= 2)
		{
			highlighted = 1;
			menu_bar->deactivate_();
		}
		return 1;
	}
	else
// try the popup
		return menu_popup->button_release_dispatch();
return 0;
}

int BC_Menu::key_press_dispatch()
{
	return menu_popup->key_press_dispatch();
return 0;
}



// ========================================== menu item

BC_MenuItem::BC_MenuItem(const char *text, char *hotkey_text, int hotkey)
{
	strcpy(this->text, text);
	strcpy(this->hotkey_text, hotkey_text);
	this->hotkey = hotkey;
	reset_parameters();
}

BC_MenuItem::~BC_MenuItem()
{
	if(submenu) delete submenu;          // remove any submenu
	menu_popup->menu_items.remove(this); // remove from arraylist
}

BC_MenuItem::BC_MenuItem(const char *text)
{
	strcpy(this->text, text);
	this->hotkey_text[0] = 0;
	hotkey = 0;
	reset_parameters();
}

int BC_MenuItem::reset_parameters()
{
	checked = 0;
	highlighted = 0;
	shift_hotkey = 0;
	down = 0;
	submenu = 0;
return 0;
}

int BC_MenuItem::draw()
{
	int text_descent;
	text_descent = top_level->get_font(MEDIUMFONT)->descent;
	if(!strcmp(text, "-"))
	{
		menu_popup->popup->set_color(DKGREY);
		menu_popup->popup->draw_line(5, y + h / 2, menu_popup->w - 5, y + h / 2);
		menu_popup->popup->set_color(LTGREY);
		menu_popup->popup->draw_line(5, y + h / 2 + 1, menu_popup->w - 5, y + h / 2 + 1);
	}
	else
	{
// Draw the bounding box.
		if(highlighted)
		{
			int y = this->y;
			int w = menu_popup->w - 4;
			int h = this->h;

			if(top_level->get_button_down() && !submenu)
			{
				menu_popup->popup->draw_3d_big(2, y, menu_popup->w - 4, h, 
					top_level->get_resources()->menu_shadow,
					BLACK,
					top_level->get_resources()->menu_down,
					top_level->get_resources()->menu_down,
					top_level->get_resources()->menu_light);
			}
			else
			{
				menu_popup->popup->set_color(top_level->get_resources()->menu_highlighted);
				menu_popup->popup->draw_box(2, y, menu_popup->w - 4, h);
			}
		}

		menu_popup->popup->set_color(BLACK);
		if(checked)
		{
			menu_popup->popup->draw_check(10, y + 2, 15, 15);
			menu_popup->popup->draw_text(30, y + h - text_descent - 2, text);
			menu_popup->popup->draw_text(menu_popup->key_x, y + h - text_descent - 2, hotkey_text);
		}
		else
		{
			menu_popup->popup->draw_text(10, y + h - text_descent - 2, text);
			menu_popup->popup->draw_text(menu_popup->key_x, y + h - text_descent - 2, hotkey_text);
		}
	}
return 0;
}

int BC_MenuItem::motion_event_dispatch(int cursor_x, int cursor_y)
{
	int result;
	result = 0;

	if(submenu)
	{
		result = submenu->motion_event_dispatch();
	}

	if(!result)
	{
		if(cursor_x > 0 && cursor_x < menu_popup->w && 
			cursor_y > y && cursor_y <= y + h)
		{
// Cursor in item
			if(!highlighted)
			{
				top_level->unhighlight();
// Deactivate submenus in the parent menu excluding this one.
				menu_popup->deactivate_menus(submenu);
				highlighted = 1;
				if(submenu) activate_menu();
			}
			result = 1;
		}
		else
		if(highlighted)
		{
			highlighted = 0;
			//if(submenu) submenu->deactivate_menu();
			menu_popup->draw_items();
		}
	}
//printf("%d %d %d %d %d\n", cursor_x, cursor_y, menu_popup->w, y, result);

	return result;
return 0;
}

int BC_MenuItem::button_press_dispatch(int cursor_x, int cursor_y)
{
	int result;
	result = 0;

	if(submenu)
	{
		result = submenu->button_press_dispatch();
	}

	if(!result)
	{
		if(cursor_x > 0 && cursor_x < menu_popup->w && 
			cursor_y > y && cursor_y <= y + h)
		{
			if(!highlighted)
			{
				top_level->unhighlight();
				highlighted = 1;
			}
			result = 1;
		}
		else
		if(highlighted)
		{
			highlighted = 0;
			menu_popup->draw_items();
		}
	}
	return result;
return 0;
}

int BC_MenuItem::button_release_dispatch(int cursor_x, int cursor_y)
{
	int result;
	result = 0;

	if(submenu) result = submenu->button_release_dispatch();

	if(cursor_x > 0 && cursor_x < menu_popup->w && 
		cursor_y > y && cursor_y <= y + h && !result)
	{
		menu_popup->deactivate_all();
		handle_event();
		result = 1;
	}

	return result;
return 0;
}

int BC_MenuItem::key_press_dispatch()
{
	int result;
	result = 0;

	if(top_level->get_keypress() == hotkey)
	{
		if((shift_hotkey && top_level->shift_down()) ||
			(!shift_hotkey && !top_level->shift_down()))
		{
			menu_popup->deactivate_all();
			handle_event();
			result =  1;
		}
	}

	if(submenu && !result)
	{	
		return submenu->key_press_dispatch();
	}
	return 0;
return 0;
}

int BC_MenuItem::unhighlight()
{
//printf("BC_MenuItem::unhighlight %s\n", text);
	if(submenu) submenu->unhighlight();

	if(highlighted)
	{
		highlighted = 0;
// Cursor may be in submenu now.

		if(menu_popup->popup)
			menu_popup->draw_items();
	}
	return 0;
return 0;
}

int BC_MenuItem::deactivate_menus(BC_MenuPopup *exclude)
{
	if(submenu && submenu != exclude)
	{
		submenu->deactivate_menus(exclude);
		submenu->deactivate_menu();
		highlighted = 0;
	}
return 0;
}

int BC_MenuItem::deactivate_all()
{
	menu_popup->deactivate_all();
return 0;
}

int BC_MenuItem::activate_menu()
{
	int new_x, new_y;
	if(menu_popup->popup && submenu && !submenu->popup)
	{
		Window tempwin;
		XTranslateCoordinates(top_level->display, menu_popup->popup->win, top_level->rootwin, 0, y, &new_x, &new_y, &tempwin);
		submenu->activate_menu(new_x + 5, new_y, menu_popup->w - 10, h, 0, 0);
		highlighted = 1;
	}
return 0;
}

int BC_MenuItem::set_done(int return_value)
{
	top_level->set_done(return_value);
	return 0;
return 0;
}

int BC_MenuItem::set_text(const char *text)
{
	strcpy(this->text, text);
return 0;
}

int BC_MenuItem::update_menu()
{
	if(menu_popup->menu_button)
		menu_popup->menu_button->update(text);
return 0;
}

int BC_MenuItem::set_checked(int value)
{
	this->checked = value;
return 0;
}

int BC_MenuItem::get_checked()
{
	return checked;
return 0;
}

const char* BC_MenuItem::get_text()
{
	return text;
}

int BC_MenuItem::set_shift(int value)
{
	shift_hotkey = value;
return 0;
}

int BC_MenuItem::add_submenu(BC_SubMenu *submenu)
{
	this->submenu = submenu;
	submenu->reset_parameters();
	submenu->top_level = top_level;
	submenu->menu_item = this;
	submenu->subwindow = top_level;
return 0;
}






BC_SubMenuItem::BC_SubMenuItem(char *text) : BC_MenuItem(text)
{
}

BC_SubMenuItem::~BC_SubMenuItem()
{
}
