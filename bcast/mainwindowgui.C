#include <string.h>
#include "buttonbar.h"
#include "console.h"
#include "mainmenu.h"
#include "mainsamplescroll.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "patchbay.h"
#include "statusbar.h"
#include "timebar.h"
#include "timebomb.h"
#include "tracks.h"
#include "trackscroll.h"
#include "transitionpopup.h"

// the main window uses its own private colormap for video
MainWindowGUI::MainWindowGUI(char *display, int w, int h)
 : BC_Window(display, BLACK, "Broadcast 2000", w, h, 240, 160, 1)
{
//	TimeBomb timebomb;  // exits here
}


MainWindowGUI::~MainWindowGUI()
{
	delete buttonbar;
	delete statusbar;
	delete mainsamplescroll;
	delete trackscroll;
	delete transition_popup;
}

int MainWindowGUI::create_objects(MainWindow *mwindow)
{
	this->mwindow = mwindow;
	
	add_tool(mainmenu = new MainMenu(this));
	mainmenu->create_objects();

	mwindow->timebar = new TimeBar(mwindow);
	mwindow->timebar->create_objects();

	if(mwindow->tracks_vertical)
	add_subwindow(buttonbar = new ButtonBar(this, 0, menu_h(), BUTTONBARWIDTH, get_h() - menu_h() - 24));
	else
	add_subwindow(buttonbar = new ButtonBar(this, 0, menu_h(), get_w(), BUTTONBARHEIGHT));
	
	add_subwindow(statusbar = new StatusBar(this));
	
	mainsamplescroll = new MainSampleScroll(this);
	mainsamplescroll->create_objects();
	
	trackscroll = new TrackScroll(mwindow);
	trackscroll->create_objects(mwindow->get_top(), mwindow->get_bottom());

	add_tool(transition_popup = new TransitionPopup(mwindow));
return 0;
}

int MainWindowGUI::resize_event(int w, int h)
{
	if(!mwindow->resize_lock)
	{
		buttonbar->resize_event(w, h);
		statusbar->resize_event(w, h);
		mwindow->timebar->resize_event(w, h);
		mainsamplescroll->resize_event(w, h);
		
		int top = mwindow->get_top();
		int bottom = mwindow->get_bottom();
		mwindow->patches->resize_event(top, bottom);
		trackscroll->resize_event(w, h, top, bottom);
		mwindow->tracks->resize_event(w, h, top, bottom);

		mwindow->tracks->resize_event(w, h, top, bottom);
		mwindow->draw();
	}
return 0;
}

int MainWindowGUI::flip_vertical()
{
	statusbar->resize_event(get_w(), get_h());
	buttonbar->flip_vertical(get_w(), get_h());
return 0;
}

int MainWindowGUI::save_defaults(Defaults *defaults)
{
	defaults->update("MWINDOWWIDTH", get_w());
	defaults->update("MWINDOWHEIGHT", get_h());
	mainmenu->save_defaults(defaults);
return 0;
}

int MainWindowGUI::keypress_event()
{
	int result = 0;
	switch(get_keypress())
	{
		case LEFT:
			if(!ctrl_down()) { mwindow->move_left(); result = 1; }
			break;
		case RIGHT:
			if(!ctrl_down()) { mwindow->move_right(); result = 1; }
			break;
		case PGUP:
			if(!ctrl_down()) { mwindow->tracks->move_up(); result = 1; }
			break;
		case PGDN:
			if(!ctrl_down()) { mwindow->tracks->move_down(); result = 1; }
			break;
		case 'd':
			mwindow->stop_playback(1);
			mwindow->console->gui->lock_window();
			mwindow->undo->update_undo_all("Delete tracks", 0);
			mwindow->tracks->delete_track();
			mwindow->undo->update_undo_audio();
			mwindow->console->gui->unlock_window();
			result = 1;
			break;
		default :
			result = buttonbar->keypress_event();
			break;
	}
	return result;
return 0;
}


int MainWindowGUI::close_event() { mainmenu->quit(); return 0;
}

int MainWindowGUI::get_top()
{
	if(mwindow->tracks_vertical) 
	return menu_h() + BUTTONBARWIDTH;    // menubar + buttonbar
	else 
	return menu_h();      // menubar
return 0;
}

int MainWindowGUI::menu_h()
{
	return mainmenu->get_h();
return 0;
}
