#include <string.h>
#include "bcbase.h"


class TestItem : public BC_MenuItem
{
public:
	TestItem() : BC_MenuItem("Quit") {};
	~TestItem() {};

	handle_event() { printf("Quit\n"); set_done(0); };
};


class TestTextBox : public BC_TextBox
{
public:
	TestTextBox(int x, int y) : BC_TextBox(x, y, 200, "TextBox", 1) {};
	handle_event() { printf("%s\n", get_text()); return 0; };
};

int main()
{
	BC_Window window("Test", 320, 320, 320, 320);
	BC_MenuBar *menubar;
	BC_Menu *menu1, *menu2, *menu3;
	BC_MenuItem *item1, *item2, *item3, *item4;
	BC_SubMenu *submenu;
	BC_PopupMenu *popupmenu;
	
	window.add_tool(menubar = new BC_MenuBar(0, 0, window.get_w()));
	menubar->add_menu(menu1 = new BC_Menu("File"));
	menubar->add_menu(menu2 = new BC_Menu("Edit"));
	menubar->add_menu(menu3 = new BC_Menu("Audio"));
	menu1->add_menuitem(item1 = new TestItem);
	menu1->add_menuitem(item2 = new BC_MenuItem("Load"));
	menu1->add_menuitem(item3 = new BC_MenuItem("Save"));
	menu2->add_menuitem(new BC_MenuItem("Undo"));
	menu2->add_menuitem(new BC_MenuItem("Redo"));
	menu3->add_menuitem(new BC_MenuItem("Add Track"));
	menu3->add_menuitem(item4 = new BC_MenuItem("Pans"));
	menu3->add_menuitem(new BC_MenuItem("Mute"));
	menu3->add_menuitem(new BC_MenuItem("Fade"));
	item4->add_submenu(submenu = new BC_SubMenu);
	submenu->add_submenuitem(new BC_SubMenuItem("Pan 1"));
	submenu->add_submenuitem(new BC_SubMenuItem("Pan 2"));
	submenu->add_submenuitem(new BC_SubMenuItem("Pan 3"));

	window.add_tool(new TestTextBox(5, menubar->get_h() + 10));
	window.add_tool(new TestTextBox(5, menubar->get_h() + 40));

	window.add_tool(popupmenu = new BC_PopupMenu(100, 150, 100, "Test", 0));
	popupmenu->add_item(new BC_PopupItem("Item 1"));
	popupmenu->add_item(new BC_PopupItem("Item 2"));
	popupmenu->add_item(new BC_PopupItem("Item 3"));
	window.run_window();
	
	return 0;
}
