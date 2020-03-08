#include <string.h>
#include "bcfont.h"
#include "bcmenubar.h"
#include "bcresources.h"
#include "bcwindow.h"


// ============================================== menu popup

BC_MenuPopup::BC_MenuPopup(BC_Menu *menu, 
						BC_MenuBar *menubar, 
						ArrayList<BC_MenuItem*> *menuitems, 
						BC_Window *parent, 
						int color, int x, int y, int w, int h) 
: BC_Popup(parent, color, x, y, w, h) 
{ 
	this->menuitems = menuitems;
	this->menubar = menubar;
	this->menu = menu;
}

BC_MenuPopup::~BC_MenuPopup()
{
}

int BC_MenuPopup::draw()
{
	int i;
	draw_3d_big(0, 0, w, h, 
		top_level->get_resources()->menu_light,
		top_level->get_resources()->menu_up,
		top_level->get_resources()->menu_up,
		top_level->get_resources()->menu_shadow,
		BLACK);

	for(i = 0; i < menuitems->total; i++)
	{
		menuitems->values[i]->draw();
	}
	flash();
return 0;
}

int BC_MenuPopup::cursor_motion()
{
	int i, result;
	
	result = 0;

	for(i = 0; i < menuitems->total; i++)
	{
		result += menuitems->values[i]->cursor_motion(cursor_x, cursor_y);
	}
	if(result) draw();
return 0;
}

int BC_MenuPopup::button_press()
{
	int i, result;
	result = 0;
	for(i = 0; i < menuitems->total && !result; i++)
	{
		result = menuitems->values[i]->button_press();
	}
	flash();
	return result;
return 0;
}

int BC_MenuPopup::button_release()
{
	menu->button_release(cursor_x, cursor_y);
return 0;
}

// ============================================== menu


int BC_Menu::get_width()
{
	int w, test_w, i;
	int text_w, hotkey_w;
	w = 0;
	text_w = 0;
	hotkey_w = 0;
	
	
	for(i = 0; i < menuitems.total; i++)
	{
		if((test_w = menuitems.values[i]->get_text_width()) > text_w) text_w = test_w;
		if((test_w = menuitems.values[i]->get_hotkey_width()) > hotkey_w) hotkey_w = test_w;
	}
	
	w = text_w + hotkey_w;
	hotkey_x = text_w;
	return w;
return 0;
}

int BC_Menu::get_height()
{
	int h, i;
	h = 0;
	
	for(i = 0; i < menuitems.total; i++)
	{
		h += menuitems.values[i]->get_height();
	}
	return h;
return 0;
}

int BC_Menu::activate()
{
	popup_menu = new BC_MenuPopup(this, menubar, &menuitems, top_level, MECYAN, title_x + menubar->x, menubar->h + menubar->y - 2, get_width(), get_height());

	active = 1;
	draw_title();
	popup_menu->draw();
return 0;
}

int BC_Menu::deactivate(int cursor_x, int cursor_y)
{
	int min_title_y;
	min_title_y = -menubar->h;

// deactivate recursively
	if(active && popup_menu)
	{
		deactivate_items();
		active = 0;

		if(cursor_y > min_title_y && cursor_y < 0 && cursor_x > 0 && cursor_x < title_w)
			highlighted = 1;
		else
			highlighted = 0;

		delete popup_menu;
		popup_menu = 0;
		draw_title();
	}
return 0;
}

int BC_Menu::deactivate_items()
{
	if(active && popup_menu)
	{
		popup_menu->draw_3d_big(0, 0, popup_menu->w, popup_menu->h, LTCYAN, MECYAN, DKCYAN);
		for(int i = 0; i < menuitems.total; i++)
		{
			menuitems.values[i]->deactivate();
		}                       // run popup second
 		popup_menu->flash();
 	}
return 0;
}

int BC_Menu::translate_coords(int *x_, int *y_)
{
	*x_ = top_level->cursor_x - subwindow->get_x() - menubar->x; 
	*y_ = top_level->cursor_y - subwindow->get_x() - menubar->y;
return 0;
}


int BC_Menu::get_keypress()
{
	return top_level->get_keypress();
return 0;
}

int BC_Menu::set_keypress(int value)
{
	top_level->set_key_pressed(value);
return 0;
}

int BC_Menu::shift_down()
{
	return top_level->shift_down();
return 0;
}

int BC_Menu::button_down()
{
	return menubar->button_down;
return 0;
}

int BC_Menu::in_submenu()
{
	int i;
	for(i = 0; i < menuitems.total; i++)
	{
		if(menuitems.values[i]->in_submenu()) return 1;
	}
	return 0;
return 0;
}

