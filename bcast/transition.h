#ifndef TRANSITION_H
#define TRANSITION_H

class PasteTransition;

#include "attachmentpoint.h"
#include "edit.inc"
#include "filehtal.inc"
#include "mainwindow.inc"
#include "messages.inc"
#include "sharedpluginlocation.h"

class TransitionMenuItem : public BC_MenuItem
{
public:
	TransitionMenuItem(MainWindow *mwindow, int audio, int video);
	~TransitionMenuItem();
	int handle_event();
	PasteTransition *thread;
};

class PasteTransition : public Thread
{
public:
	PasteTransition(MainWindow *mwindow, int audio, int video);
	~PasteTransition();

	void run();

	MainWindow *mwindow;
	int audio, video;
};


// Transition object stored in each edit which is a transition
class Transition : public AttachmentPoint
{
public:
	Transition(MainWindow *mwindow, Edit *edit, int audio, int video);
	Transition(Transition *that, Edit *edit);
	~Transition();

	int reset_parameters();
	int update_derived();
	Transition& operator=(const Transition &that);
	int operator==(const Transition &that);

// Only the show value from the attachment point is used.
	int set_show_derived(int value) { return 0; };

	int popup_transition(int x, int y);
	char* get_module_title();
// Update the widgets after loading
	int update_display();
// Update edit after attaching
	int update_edit(int is_loading);
	const char* default_title();

	int audio, video;
	MainWindow *mwindow;
	Edit *edit;

private:
// Only used by operator= and copy constructor
	int copy_from(const Transition &that);
	int identical(const Transition &that);
};

#endif
