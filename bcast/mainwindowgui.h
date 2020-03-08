#ifndef MAINWINDOWGUI_H
#define MAINWINDOWGUI_H

#include "bcbase.h"
#include "buttonbar.inc"
#include "mainmenu.inc"
#include "mainwindow.inc"
#include "statusbar.inc"
#include "mainsamplescroll.inc"
#include "trackscroll.inc"
#include "transitionpopup.inc"


class MainWindowGUI : public BC_Window
{
public:
	MainWindowGUI(char *display, int w, int h);
	~MainWindowGUI();

	int create_objects(MainWindow *mwindow);

// ======================== event handlers

	int resize_event(int w, int h);          // handle a resize event
	int flip_vertical();
	int keypress_event();
	int close_event();
	int quit();
	int save_defaults(Defaults *defaults);
	int get_top();
	int menu_h();

	MainWindow *mwindow;

	ButtonBar *buttonbar;
	MainMenu *mainmenu;
	StatusBar *statusbar;
	MainSampleScroll *mainsamplescroll;
	TrackScroll *trackscroll;
// Floating popup for transitions
	TransitionPopup *transition_popup;
};



#endif
