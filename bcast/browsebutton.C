#include <string.h>
#include "browsebutton.h"

BrowseButton::BrowseButton(int x, int y, BC_TextBox *textbox, char *init_directory, char *title, char *caption, int want_directory)
 : BC_BigButton(x, y, "Browse..."), Thread()
{ 
	this->want_directory = want_directory;
	this->title = title;
	this->caption = caption;
	this->init_directory = init_directory;
	this->textbox = textbox;
}

BrowseButton::~BrowseButton()  { }

int BrowseButton::handle_event() { start(); return 0;
}

void BrowseButton::run()
{
	disable_window();
	BrowseButtonWindow browsewindow("", textbox->get_text(), title, caption, want_directory);
	browsewindow.create_objects();
	int result2 = browsewindow.run_window();
	if(!result2)
	{
		if(want_directory)
		{
			textbox->update(browsewindow.get_directory());
		}
		else
		{
			textbox->update(browsewindow.get_filename());
		}


		textbox->handle_event();
	}
	enable_window();
}






BrowseButtonWindow::BrowseButtonWindow(char *display, char *init_directory, char *title, char *caption, int want_directory)
 : BC_FileBox(display, init_directory, title, caption, 0, want_directory) {}

BrowseButtonWindow::~BrowseButtonWindow() {}
