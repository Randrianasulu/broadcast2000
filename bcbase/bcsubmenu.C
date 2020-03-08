#include <string.h>
#include "bcmenubar.h"
#include "bcresources.h"
#include "bcwindow.h"


// ====================================== submenu popup

BC_SubMenuPopup::BC_SubMenuPopup(BC_SubMenu *submenu, ArrayList<BC_SubMenuItem*> *submenuitems, BC_Window *top_level, int color, int x, int y, int w, int h)
 : BC_Popup(top_level, color, x, y, w, h)
{
	this->submenuitems = submenuitems;
	this->submenu = submenu;
}

BC_SubMenuPopup::~BC_SubMenuPopup()
{
}

int BC_SubMenuPopup::draw()
{
	draw_3d_big(0, 0, w, h, 
		top_level->get_resources()->menu_light,
		top_level->get_resources()->menu_up,
		top_level->get_resources()->menu_up,
		top_level->get_resources()->menu_shadow,
		BLACK);

	//draw_3d_big(0, 0, w, h, LTCYAN, MECYAN, DKCYAN);
	for(int i = 0; i < submenuitems->total; i++)
	{
		submenuitems->values[i]->draw();
	}
	flash();
return 0;
}

int BC_SubMenuPopup::cursor_motion()
{
	submenu->cursor_motion(cursor_x, cursor_y);
return 0;
}

int BC_SubMenuPopup::button_press()
{
	int result = 0;
	for(int i = 0; i < submenuitems->total && !result; i++)
	{
		result = submenuitems->values[i]->button_press();
	}
	flash();
	return result;
return 0;
}

int BC_SubMenuPopup::button_release()
{
// deletes the popup
	submenu->button_release(cursor_x, cursor_y);
return 0;
}


// ====================================== submenu object

BC_SubMenu::BC_SubMenu()
{
	active = 0;
	highlighted = 0;
	popup_submenu = 0;
}

BC_SubMenu::~BC_SubMenu()
{
// objects
	for(int i = 0; i < submenuitems.total; i++) delete submenuitems.values[i];
// pointers
	submenuitems.remove_all();
	menuitem->submenu = 0;      // deleted derived submenu
	if(popup_submenu) 
	{
		delete popup_submenu;
		popup_submenu = 0;
	}
}

int BC_SubMenu::create_objects(BC_MenuItem *menuitem, int x, int y)
{
	this->menuitem = menuitem;
	top_level = menuitem->top_level;
	subwindow = menuitem->menu->subwindow;
	menu_x = x;  menu_y = y;
	menubar = menuitem->menu->menubar;
return 0;
}

int BC_SubMenu::activate()
{
	menu_x = menuitem->get_width() - 10;
	
	popup_submenu = new BC_SubMenuPopup(this, &submenuitems, top_level, MECYAN, menuitem->menu->popup_menu->x + menu_x, menuitem->menu->popup_menu->y + menu_y, get_width(), get_height());
	popup_submenu->draw();	
	active = 1;
return 0;
}

int BC_SubMenu::deactivate()
{
	if(active && popup_submenu)
	{
		deactivate_items();
		delete popup_submenu;
		popup_submenu = 0;
	}
	active = 0;
	highlighted = 0;
return 0;
}

int BC_SubMenu::deactivate_items()
{
	if(active)
	{
		for(int i = 0; i < submenuitems.total; i++)
		{
			submenuitems.values[i]->deactivate();
		}
	}
return 0;
}

int BC_SubMenu::add_submenuitem(BC_SubMenuItem* submenuitem)
{
	int y;
	
	if(submenuitems.total == 0)
	{
		y = 0;
	}
	else
	{
		y = submenuitems.values[submenuitems.total - 1]->get_y() + submenuitems.values[submenuitems.total - 1]->get_height();
	}

	submenuitem->create_objects(top_level, this, y);
// pointer
	submenuitems.append(submenuitem);
return 0;
}

int BC_SubMenu::remove_submenuitem(BC_SubMenuItem *item)
{
// derived object must be deleted manually
	//delete submenuitems.values[submenuitems.total - 1];
// pointer only
	submenuitems.remove(item);
return 0;
}

int BC_SubMenu::cursor_left_dispatch()
{
	if(active) popup_submenu->cursor_left_dispatch();
	return 0;
return 0;
}

int BC_SubMenu::motion_event_dispatch()
{
	if(active) popup_submenu->motion_event_dispatch();
return 0;
}

int BC_SubMenu::button_release_dispatch()
{
//printf("BC_SubMenu::button_release_dispatch %d\n", active);
	if(active) popup_submenu->button_release_dispatch();
return 0;
}

int BC_SubMenu::button_press_dispatch()
{
	int result = 0;
	if(active) result = popup_submenu->button_press_dispatch();
	return result;
return 0;
}

// do here since button release deletes the popup
int BC_SubMenu::button_release(int cursor_x, int cursor_y)
{
	if((cursor_x > 0 && cursor_x < popup_submenu->w && cursor_y > 0 && cursor_y < popup_submenu->h)) 
	{             // inside popup
		for(int i = 0; i < submenuitems.total && active; i++)
		{
			submenuitems.values[i]->button_release();
		}
	}
return 0;
}

int BC_SubMenu::cursor_motion(int cursor_x, int cursor_y)
{
	popup_submenu->draw_3d_big(0, 0, popup_submenu->w, popup_submenu->h, 
		top_level->get_resources()->menu_light,
		top_level->get_resources()->menu_up,
		top_level->get_resources()->menu_up,
		top_level->get_resources()->menu_shadow,
		BLACK);

	for(int i = 0; i < submenuitems.total; i++)
	{
		submenuitems.values[i]->motion_event_dispatch(cursor_x, cursor_y);
	}
	if(cursor_x > 0 && cursor_x < popup_submenu->w && cursor_y > 0 && cursor_y < popup_submenu->h) highlighted = 1;
		else highlighted = 0;
	popup_submenu->flash();
return 0;
}

int BC_SubMenu::in_submenu()
{
	if(popup_submenu)
	{
// submenu exists
		if(popup_submenu->cursor_x < 0 || popup_submenu->cursor_x > popup_submenu->w ||
			popup_submenu->cursor_y < 0 || popup_submenu->cursor_y > popup_submenu->h)
		{
// not in the existing submenu
			return 0;
		}
		else
// in the existing submenu
		return 1;
	}
	else 
// no submenu
	return 0;
return 0;
}

int BC_SubMenu::get_width()
{
	int test_w, i, w;
	w = 0;
	
	for(i = 0; i < submenuitems.total; i++)
	{
		if((test_w = submenuitems.values[i]->get_text_width()) > w) w = test_w;
	}
	return w;
return 0;
}

int BC_SubMenu::get_checked()
{
	int i;
	for(i = 0; i < submenuitems.total; i++)
	{
		if(submenuitems.values[i]->checked) return 1;
	}
	return 0;
return 0;
}

int BC_SubMenu::get_height()
{
	int h = 0;
	for(int i = 0; i < submenuitems.total; i++)
	{
		h += submenuitems.values[i]->get_height();
	}
	return h;
return 0;
}

int BC_SubMenu::button_down()
{
	return menuitem->menu->button_down();
return 0;
}
