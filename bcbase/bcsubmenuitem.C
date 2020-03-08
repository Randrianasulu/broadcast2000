#include <string.h>
#include "bcfont.h"
#include "bcmenubar.h"
#include "bcresources.h"
#include "bcwindow.h"

BC_SubMenuItem::BC_SubMenuItem(char *text)
{
	strcpy(this->text, text);
	highlighted = 0;
	checked = 0;
}

BC_SubMenuItem::~BC_SubMenuItem()
{
	submenu->remove_submenuitem(this);
}

int BC_SubMenuItem::create_objects(BC_Window *top_level, BC_SubMenu *submenu, int y)
{
	this->submenu = submenu;	this->y = y;    h = 22;
	this->top_level = top_level;
return 0;
}

int BC_SubMenuItem::get_text_width()
{
	int w;
	w = submenu->menuitem->menu->menubar->get_text_width(MEDIUMFONT, text) + 20;
	if(checked) w += 20;
	return w;
return 0;
}

int BC_SubMenuItem::get_height()
{
	return h;
return 0;
}

int BC_SubMenuItem::get_y()
{
	return y;
return 0;
}

int BC_SubMenuItem::set_checked(int checked)
{
	this->checked = checked;
return 0;
}

int BC_SubMenuItem::get_checked()
{
	return checked;
return 0;
}

int BC_SubMenuItem::activate()
{
	highlighted = 1;
return 0;
}

int BC_SubMenuItem::deactivate()
{
	highlighted = 0;
return 0;
}

int BC_SubMenuItem::draw()
{
	if(highlighted)
	{
		int y = this->y, w = submenu->popup_submenu->w - 4, h = this->h;
		if(y == 0)
		{ y += 2; h -= 4; }
		else
		{ h -= 2; }

		if(submenu->button_down())
		{
				submenu->popup_submenu->draw_3d_big(2, y, w, h, 
					top_level->get_resources()->menu_shadow,
					BLACK,
					top_level->get_resources()->menu_down,
					top_level->get_resources()->menu_down,
					top_level->get_resources()->menu_light);
		}
		else
		{
			submenu->popup_submenu->set_color(top_level->get_resources()->menu_highlighted);
			submenu->popup_submenu->draw_box(2, y, w, h);
		}
	}
	submenu->popup_submenu->set_color(BLACK);
	if(checked)
	{
		submenu->popup_submenu->draw_check(10, y + 2, 15, 15);
		submenu->popup_submenu->draw_text(30, y + h - 7, text);
	}
	else
	{
		submenu->popup_submenu->draw_text(10, y + h - 7, text);
	}
return 0;
}

int BC_SubMenuItem::motion_event_dispatch(int cursor_x, int cursor_y)
{
	if(highlighted)
	{
		if(cursor_x < 0 || cursor_x > submenu->popup_submenu->w ||
		   cursor_y < y || cursor_y > y + h)
		{
			deactivate();
		}
	}
	else
	{
		if(cursor_x > 0 && cursor_x < submenu->popup_submenu->w &&
		   cursor_y > y && cursor_y < y + h)
		{
			//submenu->deactivate_items();
			activate();
		}
	}
	draw();
return 0;
}

int BC_SubMenuItem::button_press()
{
	if(highlighted) draw();
	return 0;
return 0;
}

int BC_SubMenuItem::button_release()
{
	if(submenu->popup_submenu->cursor_x > 0 && submenu->popup_submenu->cursor_x < submenu->popup_submenu->w &&
		 submenu->popup_submenu->cursor_y > y && submenu->popup_submenu->cursor_y < y + h)
	{
		submenu->menubar->deactivate();
		handle_event();
	}
return 0;
}
