#include <string.h>
#include "console.h"
#include "mainwindow.h"
#include "transition.h"
#include "plugindialog.h"
#include "tracks.h"
#include "transition.h"
#include "transitionpopup.h"


TransitionPopup::TransitionPopup(MainWindow *mwindow)
 : BC_PopupMenu(0, 0, 100, "", 1, 1)
{
	this->mwindow = mwindow;
	dialog_thread = new PluginDialogThread(mwindow, 0, 0, ICONNAME ": Transition");
}

TransitionPopup::~TransitionPopup()
{
	delete dialog_thread;
}

int TransitionPopup::activate_menu(Transition *transition, char *title, int x, int y)
{
	this->transition = transition;

	dialog_thread->set_dialog(transition, title);
	update();

// Pull down menu here.
	BC_PopupMenu::activate_menu(x, y, (BC_Tool*)(mwindow->tracks->canvas));
return 0;
}

int TransitionPopup::update()
{
	this->in->set_checked(transition->in);
	this->out->set_checked(transition->out);
	this->show->set_checked(transition->show);
	this->on->set_checked(transition->on);
return 0;
}


int TransitionPopup::add_items()
{
	add_item(new TransitionPopupAttach(this));
	add_item(in = new TransitionPopupIn(this));
	add_item(out = new TransitionPopupOut(this));
	add_item(show = new TransitionPopupShow(this));
	add_item(on = new TransitionPopupOn(this));
	add_item(new TransitionPopupDetach(this));
return 0;
}

int TransitionPopup::handle_event()
{
return 0;
}


TransitionPopupAttach::TransitionPopupAttach(TransitionPopup *popup)
 : BC_PopupItem("Attach...")
{
	this->popup = popup;
}

TransitionPopupAttach::~TransitionPopupAttach()
{
}

int TransitionPopupAttach::handle_event()
{
	popup->dialog_thread->start();
return 0;
}







TransitionPopupDetach::TransitionPopupDetach(TransitionPopup *popup)
 : BC_PopupItem("Detach")
{
	this->popup = popup;
}

TransitionPopupDetach::~TransitionPopupDetach()
{
}

int TransitionPopupDetach::handle_event()
{
	if(popup->transition->plugin_type)
	{
// a plugin already is attached
		popup->transition->mwindow->start_reconfigure(1);
// detach it
		popup->transition->detach();			
// make it easy for user to know function of button
		popup->transition->update(0, 0, 0, "Transition", 0, 0);
		popup->transition->mwindow->stop_reconfigure(1);
	}
return 0;
}







TransitionPopupIn::TransitionPopupIn(TransitionPopup *popup)
 : BC_PopupItem("Send", 0)
{
	this->popup = popup;
}

TransitionPopupIn::~TransitionPopupIn()
{
}

int TransitionPopupIn::handle_event()
{
	popup->transition->mwindow->start_reconfigure(1);
	popup->transition->in ^= 1;
	set_checked(popup->transition->in);
	popup->transition->mwindow->stop_reconfigure(1);
return 0;
}





TransitionPopupOut::TransitionPopupOut(TransitionPopup *popup)
 : BC_PopupItem("Receive", 0)
{
	this->popup = popup;
}

TransitionPopupOut::~TransitionPopupOut()
{
}

int TransitionPopupOut::handle_event()
{
	popup->transition->mwindow->start_reconfigure(1);
	popup->transition->out ^= 1;
	set_checked(popup->transition->out);
	popup->transition->mwindow->stop_reconfigure(1);
return 0;
}






TransitionPopupOn::TransitionPopupOn(TransitionPopup *popup)
 : BC_PopupItem("On", 0)
{
	this->popup = popup;
}

TransitionPopupOn::~TransitionPopupOn()
{
}

int TransitionPopupOn::handle_event()
{
	popup->transition->mwindow->start_reconfigure(1);
	popup->transition->on ^= 1;
	set_checked(popup->transition->on);
	popup->transition->mwindow->stop_reconfigure(1);
return 0;
}






TransitionPopupShow::TransitionPopupShow(TransitionPopup *popup)
 : BC_PopupItem("Show", 0)
{
	this->popup = popup;
}

TransitionPopupShow::~TransitionPopupShow()
{
}

int TransitionPopupShow::handle_event()
{
	popup->transition->show ^= 1;
	set_checked(popup->transition->show);
	if(popup->transition->show) 
		popup->transition->show_gui();
	else
		popup->transition->hide_gui();
return 0;
}

