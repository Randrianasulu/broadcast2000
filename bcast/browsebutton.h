#ifndef BROWSEBUTTON_H
#define BROWSEBUTTON_H

#include "bcbase.h"
#include "thread.h"


class BrowseButton : public BC_BigButton, public Thread
{
public:
	BrowseButton(int x, int y, BC_TextBox *textbox, char *init_directory, char *title, char *caption, int want_directory = 0);
	~BrowseButton();
	
	int handle_event();
	void run();
	int want_directory;
	char result[1024];
	char *title;
	char *caption;
	char *init_directory;
	BC_TextBox *textbox;
};

class BrowseButtonWindow : public BC_FileBox
{
public:
	BrowseButtonWindow(char *display, char *init_directory, char *title, char *caption, int want_directory);
	~BrowseButtonWindow();
};





#endif
