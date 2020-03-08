#ifndef FEATHEREDITS_H
#define FEATHEREDITS_H

#include "bcbase.h"
#include "mainwindow.inc"

class FeatherEdits : public BC_MenuItem, Thread
{
public:
	FeatherEdits(MainWindow *mwindow, int audio, int video);

	int handle_event();
	
	void run();

	MainWindow *mwindow;
	int audio;
	int video;
};



class FeatherEditsOkButton;
class FeatherEditsCancelButton;
class FeatherEditsTextBox;

class FeatherEditsWindow : public BC_Window
{
public:
	FeatherEditsWindow(long feather_samples);
	~FeatherEditsWindow();
	
	int create_objects(int audio, int video);

	long feather_samples;
	FeatherEditsOkButton  *ok;
	FeatherEditsCancelButton  *cancel;
	FeatherEditsTextBox  *text;
	int audio;
	int video;
};

class FeatherEditsOkButton : public BC_BigButton
{
public:
	FeatherEditsOkButton(FeatherEditsWindow *window);
	
	int handle_event();
	int keypress_event();
	
	FeatherEditsWindow *window;
};

class FeatherEditsCancelButton : public BC_BigButton
{
public:
	FeatherEditsCancelButton(FeatherEditsWindow *window);
	
	int handle_event();
	int keypress_event();
	
	FeatherEditsWindow *window;
};

class FeatherEditsTextBox : public BC_TextBox
{
public:
	FeatherEditsTextBox(FeatherEditsWindow *window, char *text);
	
	int handle_event();
	
	FeatherEditsWindow *window;
};


#endif
