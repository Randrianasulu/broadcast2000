#ifndef BCMENUBAR_H
#define BCMENUBAR_H

#include "arraylist.h"
#include "bcpopupmenu.inc"
#include "bctool.h"

class BC_Menu;
class BC_MenuItem;
class BC_SubMenu;

class BC_MenuBar : public BC_Tool
{
public:
	BC_MenuBar(int x, int y, int w);
	virtual ~BC_MenuBar();

// Called by user.
	int add_menu(BC_Menu* menu);
	int enable();
	int disable();

// Called by tool.
	int create_tool_objects();
	int resize_event_(int w, int h);
	int keypress_event_();
	int cursor_motion_();
	int cursor_left_();
	int button_press_();
	int button_release_();
	int expose_event_dispatch();
	int activate_();
	int deactivate_();     // Deactivates all menus propogating downward.
	int unhighlight_();

// Called by menus.
	int activate_menus();

	int draw(); // Just draws the outline for the menubar.

	ArrayList<BC_Menu*> menu_titles;  // Array of menu titles
	int button_down;
	int enabled;
	int button_releases;        // number of button releases since activation
	int active; // When a menu is pulled down.
};

// Here BC_Menu is the title for a BC_MenuPopup of items.
class BC_Menu
{
public:
	BC_Menu(char *text);
	virtual ~BC_Menu();

// Called by user.
	int add_menuitem(BC_MenuItem* menuitem);
	int remove_menuitem(BC_MenuItem *item = 0);
	int total_menuitems();
	int set_done(int return_value);

// Called by menubar.
	int create_objects(BC_MenuBar *menu_bar, BC_Window *top_level, int x, int y, int w, int h);
	int draw_title(); // draw menubar title
	int key_press_dispatch();
	int button_release_dispatch();
	int button_press_dispatch();
	int expose_event_dispatch();
	int cursor_left_dispatch();
	int motion_event_dispatch();
	int unhighlight();
// Deactivate all menus propogating downward.
	int deactivate_menus();
// Deactivate all menus starting from the top.
	int deactivate_all();

	int activate_menu();  // Activate this menu.
	int deactivate_menu();   // Deactivate this menu only.

	BC_MenuPopup *menu_popup;  // Client popup.
	BC_MenuBar *menu_bar;  // Owner menubar.
	int x, y, w, h; // Dimensions relative to menubar.
	int active; // If this menu is pulled down.
	BC_Window *top_level;
	char text[256];
	int highlighted; // If cursor is over title and the menu is not pulled down.
};


// Item shared between pull down menus and popup menus.

class BC_MenuItem
{
public:
	BC_MenuItem(char *text, char *hotkey_text, int hotkey = 0);
	BC_MenuItem(char *text);
	virtual ~BC_MenuItem();

// User functions
	virtual int handle_event() {return 0;};

	int set_done(int return_value);    // Tell the top level window to quit.
	int set_text(char *text);
	int set_checked(int value = 1);
	int set_shift(int value = 1);   // whether or not the hotkey requires shift
	int get_checked();
	int add_submenu(BC_SubMenu *submenu);
	int update_menu();      // Set the menu title to the value of this text
	char* get_text();

// Tool functions
	int reset_parameters();
	int draw();
	int motion_event_dispatch(int cursor_x, int cursor_y);
	int button_press_dispatch(int cursor_x, int cursor_y);
	int button_release_dispatch(int cursor_x, int cursor_y);
	int unhighlight();
// Deactivate menus propogating downward.
	int deactivate_menus(BC_MenuPopup *exclude = 0);
// Deactivate all menus starting from the top.
	int deactivate_all();
	int key_press_dispatch();
	int activate_menu();

	char text[256];               // title
	char hotkey_text[32];         // text of hotkey
	int hotkey;                   // code of hotkey
	int shift_hotkey;
	int checked;                  // check box
	BC_MenuPopup *menu_popup;     // Menu that owns the item.
	BC_SubMenu *submenu;          // Submenu if this item owns one.
	BC_Window *top_level;
	int highlighted;              // whether the cursor is over or not
	int down;                     // whether the cursor is over and the button is down
	int enabled;                  // whether it works or not
	int y;                        // y position of this item set during menu activation
	int h;                        // height of item is set during menu activation
};




class BC_SubMenuItem : public BC_MenuItem
{
public:
	BC_SubMenuItem(char *text);
	virtual ~BC_SubMenuItem();
};

#endif
