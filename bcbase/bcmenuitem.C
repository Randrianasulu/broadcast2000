#include <string.h>
#include "bcfont.h"
#include "bcmenubar.h"
#include "bcresources.h"
#include "bcwindow.h"

// ======================================= menu item

BC_MenuItem::BC_MenuItem(char *text, char *hotkey_text, int hotkey)
{
	checked = 0;
	this->hotkey = hotkey;
	if(this->hotkey == 0) this->hotkey = 65535;
	active = 0;
	highlighted = 0;
	submenu = 0;
	shift_set = 0;
	strcpy(this->text, text);
	strcpy(this->hotkey_text, hotkey_text);
}

BC_MenuItem::BC_MenuItem(char *text)
{
	checked = hotkey = 0;
	active = 0;
	highlighted = 0;
	submenu = 0;
	shift_set = 0;
	strcpy(this->text, text);
	this->hotkey_text[0] = 0;
}

int BC_MenuItem::create_objects(BC_Window* top_level, BC_Menu* menu, int y)
{
	this->menu = menu;    this->y = y;
	this->top_level = top_level;
	if(!strcmp(text, "-")) h = 5; else h = 22; 
return 0;
}

BC_MenuItem::~BC_MenuItem()
{
// derived submenu must be deleted manually
// delete underived submenu
	if(submenu) delete submenu;
	menu->remove_menuitem(this); // remove this pointer
}

int BC_MenuItem::set_done(int return_value)
{
	menu->set_done(return_value);
return 0;
}

int BC_MenuItem::set_text(char *text)
{
	strcpy(this->text, text);
return 0;
}

int BC_MenuItem::get_height()
{
	return h;
return 0;
}

int BC_MenuItem::get_y()
{
	return y;
return 0;
}

int BC_MenuItem::in_submenu()
{
	if(submenu) return submenu->in_submenu();
	else return 0;
return 0;
}

int BC_MenuItem::get_text_width()
{
	int w;
	w = menu->menubar->get_text_width(MEDIUMFONT, text) + 20;
	if(get_checked()) { w += 20; }
	return w;
return 0;
}

char* BC_MenuItem::get_text()
{
	return text;
}

int BC_MenuItem::get_checked()
{
	if(submenu) return submenu->get_checked();
	else
	return checked;
return 0;
}

int BC_MenuItem::get_hotkey_width()
{
	int w;
	w = menu->menubar->get_text_width(MEDIUMFONT, hotkey_text) + 10;
	return w;
return 0;
}

int BC_MenuItem::get_width()
{
	return menu->get_width();
return 0;
}

int BC_MenuItem::set_shift()
{
	shift_set = 1;
return 0;
}

int BC_MenuItem::set_checked(int checked)
{
	this->checked = checked;
return 0;
}

int BC_MenuItem::add_submenu(BC_SubMenu* submenu)
{
	this->submenu = submenu;
	submenu->create_objects(this, menu->get_width() - 10, y - 5);
return 0;
}

int BC_MenuItem::draw()
{
	if(!strcmp(text, "-"))
	{
		menu->popup_menu->set_color(DKGREY);
		menu->popup_menu->draw_line(5, y + h / 2, menu->popup_menu->w - 5, y + h / 2);
		menu->popup_menu->set_color(LTGREY);
		menu->popup_menu->draw_line(5, y + h / 2 + 1, menu->popup_menu->w - 5, y + h / 2 + 1);
	}
	else
	{
		if(highlighted)
		{
			int y = this->y, w = menu->popup_menu->w - 4, h = this->h;
			if(y == 0)
			{ y += 2; h -= 4; }
			else
			{ h -= 2; }

			if(menu->button_down())
			{
				menu->popup_menu->draw_3d_big(2, y, w, h, 
					top_level->get_resources()->menu_shadow,
					BLACK,
					top_level->get_resources()->menu_down,
					top_level->get_resources()->menu_down,
					top_level->get_resources()->menu_light);

// 				menu->popup_menu->draw_3d_big(2, y + 2, menu->popup_menu->w - 4, h - 4, DKCYAN, MDCYAN, LTCYAN);
// 				else
// 				menu->popup_menu->draw_3d_big(2, y, menu->popup_menu->w - 4, h - 2, DKCYAN, MDCYAN, LTCYAN);
			}
			else
			{
				menu->popup_menu->set_color(top_level->get_resources()->menu_highlighted);

				menu->popup_menu->draw_box(2, y, w, h);
			}
		}

		menu->popup_menu->set_color(BLACK);
		if(get_checked())
		{
			menu->popup_menu->draw_check(10, y + 2, 15, 15);
			menu->popup_menu->draw_text(30, y + h - 7, text);
			menu->popup_menu->draw_text(menu->hotkey_x, y + h - 7, hotkey_text);
		}
		else
		{
			menu->popup_menu->draw_text(10, y + h - 7, text);
			menu->popup_menu->draw_text(menu->hotkey_x, y + h - 7, hotkey_text);
		}
	}
return 0;
}

int BC_MenuItem::deactivate()
{
	highlighted = 0;
	if(submenu) submenu->deactivate();
	return 1;
return 0;
}

int BC_MenuItem::activate()
{
	highlighted = 1;
	if(submenu) submenu->activate();
	return 1;
return 0;
}

int BC_MenuItem::cursor_left_dispatch()
{
	int result;
	result = 0;

	if(submenu && submenu->active) result += submenu->cursor_left_dispatch();
	else
	if(highlighted)
	{
		deactivate();
		result = 1;
	}
	return result;
return 0;
}

int BC_MenuItem::key_press()
{
	int result;
	result = 0;

	if((menu->get_keypress() == hotkey && !menu->shift_down() && !shift_set) || 
		 (menu->get_keypress() == hotkey && menu->shift_down() && shift_set))
	{
		handle_event();
// trap keypress
		menu->set_keypress(0);
		result = 1;
	}
	return result;
return 0;
}

int BC_MenuItem::cursor_motion(int cursor_x, int cursor_y)
{
	int result;
	
	result = 0;
// test only submenu if submenu active
	if(submenu && submenu->active) submenu->motion_event_dispatch();

// test this item
	if(highlighted)
	{
// cursor just moved out
		if(cursor_x < 0 || cursor_x > menu->popup_menu->w ||
		   cursor_y < y || cursor_y > y + h)
		{
			if(!submenu || !submenu->in_submenu()) result = deactivate();
		}
	}
	else
	{
// cursor just moved in
		if(cursor_x > 0 && cursor_x < menu->popup_menu->w &&
		   cursor_y > y && cursor_y < y + h)
		{
			//menu->deactivate_items();
			if(!menu->in_submenu()) result = activate();
		}
	}
	
	return result;
return 0;
}

int BC_MenuItem::button_press()
{
	int result = 0;
	if(submenu) result = submenu->button_press_dispatch();
	if(highlighted) draw();
	return result;
return 0;
}

int BC_MenuItem::button_release_dispatch(int cursor_x, int cursor_y)
{
	if(strcmp(text, "-"))
	{
// test submenu
		if(submenu) submenu->button_release_dispatch();

// didn't find in submenu
		if(menu->active)
		{
			if(cursor_x > 0 && cursor_x < menu->popup_menu->w &&
				 cursor_y > y && cursor_y < y + h)
			{
				menu->menubar->deactivate();
				handle_event();
			}
		}
	}
return 0;
}
