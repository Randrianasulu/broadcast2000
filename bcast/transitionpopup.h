#ifndef TRANSITIONPOPUP_H
#define TRANSITIONPOPUP_H

#include "bcbase.h"
#include "transition.inc"
#include "plugindialog.inc"

class TransitionPopupIn;
class TransitionPopupOut;
class TransitionPopupShow;
class TransitionPopupOn;

class TransitionPopup : public BC_PopupMenu
{
public:
	TransitionPopup(MainWindow *mwindow);
	~TransitionPopup();

	int activate_menu(Transition *transition, char *title, int x, int y);
	int update();
	int add_items();  // called by BC_PopupMenu
	int handle_event();

// Set when the user clicks a transition.
	Transition *transition;
	MainWindow *mwindow;
	PluginDialogThread *dialog_thread;

// Needed for loading updates
	TransitionPopupIn *in;
	TransitionPopupOut *out;
	TransitionPopupShow *show;
	TransitionPopupOn *on;
};


class TransitionPopupAttach : public BC_PopupItem
{
public:
	TransitionPopupAttach(TransitionPopup *popup);
	~TransitionPopupAttach();

	int handle_event();
	TransitionPopup *popup;
	TransitionPopup *transition_popup;
};

class TransitionPopupDetach : public BC_PopupItem
{
public:
	TransitionPopupDetach(TransitionPopup *popup);
	~TransitionPopupDetach();

	int handle_event();
	TransitionPopup *popup;
};


class TransitionPopupIn : public BC_PopupItem
{
public:
	TransitionPopupIn(TransitionPopup *popup);
	~TransitionPopupIn();

	int handle_event();
	TransitionPopup *popup;
};

class TransitionPopupOut : public BC_PopupItem
{
public:
	TransitionPopupOut(TransitionPopup *popup);
	~TransitionPopupOut();

	int handle_event();
	TransitionPopup *popup;
};

class TransitionPopupShow : public BC_PopupItem
{
public:
	TransitionPopupShow(TransitionPopup *popup);
	~TransitionPopupShow();

	int handle_event();
	TransitionPopup *popup;
};

class TransitionPopupOn : public BC_PopupItem
{
public:
	TransitionPopupOn(TransitionPopup *popup);
	~TransitionPopupOn();

	int handle_event();
	TransitionPopup *popup;
};

#endif
