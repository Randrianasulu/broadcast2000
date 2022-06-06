#ifndef POPUPMENU_H
#define POPUPMENU_H

class BC_PopupMenu;
class BC_PopupMenuPopup;

#include "arraylist.h"
#include "bcmenubar.h"
#include "bcpopup.h"
#include "bctool.h"



// Here BC_PopupMenu is the title for a BC_MenuPopup of items.

class BC_PopupMenu : public BC_Tool
{
public:
// Set floating if no title is desired
	BC_PopupMenu(int x, int y, int w, const char *text, int small = 0, int floating = 0);
	virtual ~BC_PopupMenu();

// User commands
	virtual int handle_event() { return 0; };
	int add_item(BC_PopupItem *item);    // user adds an item
	int delete_item(BC_PopupItem *item);   // user deletes an item
	int total_items();
	char* get_text();
	virtual int add_items() { return 0; };    // Called automatically during creation.
	int resize_tool(int x, int y, int w, int h);
	int resize_tool(int x, int y);
	int update(char *text);               // change title string

// Tool commands
	int create_tool_objects();
	int draw_text();  // Just draws the button
	int deactivate_();
	int deactivate_menu();  // Deactivates this menu only.
	int button_release_();
	int button_press_();
	int cursor_motion_();
	int cursor_left_();
	int draw_items();
	int activate_menu();  // Activate the menu
	int activate_menu(int x, int y, BC_Tool *relative_tool);  // Activate a floating menu
	int unhighlight_();

	int highlighted;
	int popup_down; // Whether or not the menu is down.
	int button_releases;
	int small;
	char text[1024];
	int floating;     // Whether or not the menu has a button.
	BC_MenuPopup *menu_popup; // Popup window for the menu
	BC_MenuBar *menubar;     // If owned by a menubar
	BC_MenuItem *menuitem;   // If owned by a menuitem
};

// A backwards compatible item for popup menus to derive.

class BC_PopupItem : public BC_MenuItem
{
public:
	BC_PopupItem(const char *text, int checked = 0);
	virtual ~BC_PopupItem();

// User commands
	BC_PopupMenu* get_menu();    // return the menu button that owns this item
};



// The menu of items.
// This isn't touched by users.

class BC_MenuPopup
{
public:
	BC_MenuPopup();
	BC_MenuPopup(BC_WindowBase *subwindow, BC_PopupMenu *menu_button, BC_Window *top_level);
	BC_MenuPopup(BC_WindowBase *subwindow, BC_Menu *menu_title, BC_Window *top_level);
	BC_MenuPopup(BC_WindowBase *subwindow, BC_MenuItem *menu_item, BC_Window *top_level);
	virtual ~BC_MenuPopup();

	friend BC_PopupItem;
	friend BC_MenuItem;

	int add_item(BC_MenuItem *item);
	int delete_item(BC_MenuItem *item); // This just deletes the pointer to the item.
	int total_items();
	int reset_parameters();

// When activating the menu, need the location of the owning object.
// Coordinates are relative to root window or top_window if top_window_coords is 1.
// Menu is placed horizontally relative to owning object if vertical_justify is 0.
	int activate_menu(int x, int y, int w, int h, int top_window_coords, int vertical_justify);
	int draw_items();        // Draws the contents of this menu and flashes.
	int deactivate_menu();   // Deactivates this menu
// Deactivates all submenus in an downward progression except for the exclude
	int deactivate_menus(BC_MenuPopup *exclude = 0);
// Deactivate all menus
	int deactivate_all();
	int button_press_dispatch();
	int button_release_dispatch();
	int cursor_left_dispatch();
	int motion_event_dispatch();
	int key_press_dispatch();
	int unhighlight();

// Get dimensions of popup relative to top_level and set locations of all items.
	int get_dimensions();
	int x, y, w, h; // Dimensions relative to top_level
	int key_x;      // x of hotkeys
	BC_Popup *popup; // Popup window that only exists when menu is down.

private:
	BC_WindowBase *subwindow;  // for getting relative coords
	BC_PopupMenu *menu_button; // Title that owns this popup if a button.
	BC_Menu *menu_title; // Title that owns this popup if part of a menubar.
	BC_MenuItem *menu_item; // Menu item that owns this popup if a submenu.
	ArrayList<BC_MenuItem *> menu_items;  // Items this popup owns.
	BC_Window *top_level;
	int active;
};




class BC_SubMenu : public BC_MenuPopup
{
public:
	BC_SubMenu();
	virtual ~BC_SubMenu();

	int add_submenuitem(BC_MenuItem *item);
};



#endif
