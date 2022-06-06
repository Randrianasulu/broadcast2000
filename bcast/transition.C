#include <string.h>
#include "console.h"
#include "edit.h"
#include "edits.h"
#include "errorbox.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "patch.h"
#include "plugindialog.h"
#include "track.h"
#include "trackcanvas.h"
#include "tracks.h"
#include "transition.h"
#include "transitionpopup.h"


TransitionMenuItem::TransitionMenuItem(MainWindow *mwindow, int audio, int video)
 : BC_MenuItem("Paste Transition...")
{
	thread = new PasteTransition(mwindow, audio, video);
}

TransitionMenuItem::~TransitionMenuItem()
{
	delete thread;
}

int TransitionMenuItem::handle_event()
{
	thread->start();
return 0;
}


PasteTransition::PasteTransition(MainWindow *mwindow, int audio, int video)
 : Thread()
{
	this->mwindow = mwindow;
	this->audio = audio;
	this->video = video;
}

PasteTransition::~PasteTransition()
{
}

void PasteTransition::run()
{
	Transition transition(mwindow, 0, audio, video);
	int result = 0;

// Set up default transition.
	transition.show = 0;
	transition.on = 1;
	strcpy(transition.plugin_title, "Transition");

	if(mwindow->selectionstart == mwindow->selectionend)
	{
		ErrorBox window("", ICONNAME ": Error");
		window.create_objects("You must select a region to paste a transition.");
		window.run_window();
		return;
	}
	else
	{
		PluginDialog window((char*)(audio ? ICONNAME ": Audio Transition" : ICONNAME ": Video Transition"));
		window.create_objects(mwindow, 0, &transition);
		result = window.run_window();

		if(!result)
		{
			if(!window.plugin_type)
			{
// plugin detached
// make it easy for user to know function of button
				transition.update(0, 0, 0, transition.default_title(), &window.shared_plugin_location, &window.shared_module_location);
			}
			else
			{
				transition.update(window.plugin_type, 
					window.in->get_value(), 
					window.out->get_value(), 
					window.title->get_text(), 
					&window.shared_plugin_location, 
					&window.shared_module_location);
// Hide the GUI while attaching so it just loads the data.
				transition.attach();      // attach the new plugin
			}
		}
	}

	if(!result)
	{
		mwindow->console->gui->lock_window();
		mwindow->undo->update_undo_edits((char*)(audio ? "Audio Transition" : "Video Transition"), 0);

		mwindow->paste_transition(mwindow->selectionstart, 
				mwindow->selectionend, 
				&transition);

		mwindow->undo->update_undo_edits();
		mwindow->console->gui->unlock_window();
	}
}



Transition::Transition(MainWindow *mwindow, Edit *edit, int audio, int video)
 : AttachmentPoint(mwindow)
{
	reset_parameters();
	this->audio = audio;
	this->video = video;
	this->mwindow = mwindow;
	this->edit = edit;
}

Transition::Transition(Transition *that, Edit *edit)
 : AttachmentPoint(that)
{
	reset_parameters();
	copy_from(*that);
	this->edit = edit;
}

Transition::~Transition()
{
}

int Transition::reset_parameters()
{
return 0;
}

Transition& Transition::operator=(const Transition &that)
{
	copy_from(that);
	AttachmentPoint::copy_from(that);
return *this;
}

int Transition::operator==(const Transition &that)
{
	int result = 0;
	if(identical(that) && AttachmentPoint::identical(that)) result = 1;

	return result;
return 0;
}


int Transition::copy_from(const Transition &that)
{
	audio = that.audio;
	video = that.video;
	mwindow = that.mwindow;
	edit = that.edit;
return 0;
}

int Transition::identical(const Transition &that)
{
	int result = 0;

	if(audio == that.audio && video == that.video) 
		result = 1;
	else
		result = 0;

	return result;
return 0;
}

int Transition::popup_transition(int x, int y)
{
	if(mwindow->tracks_vertical)
		mwindow->gui->transition_popup->activate_menu(this, ICONNAME ": Transition", y, x);
	else
		mwindow->gui->transition_popup->activate_menu(this, ICONNAME ": Transition", x, y);
return 0;
}

int Transition::update_derived()
{
// Redraw transition titles
return 0;
}

char* Transition::get_module_title()
{
	return edit->edits->track->get_patch_of()->title;
}

int Transition::update_display()
{
// Don't draw anything during loads.
return 0;
}

const char* Transition::default_title()
{
	return "Transition";
}

int Transition::update_edit(int is_loading)
{
	int w, h;

	w = mwindow->tracks->canvas->get_w();
	h = mwindow->tracks->canvas->get_h();
	if(edit && edit->edits->track->track_visible(0, w, 0, h))
	{
		if(!is_loading) mwindow->tracks->hide_overlays(0);
		edit->draw_transition(0, edit->edits->track->pixel + mwindow->zoom_track / 2, 
			0, w, 0, h, 0);
		if(!is_loading) mwindow->tracks->show_overlays(1);
	}
return 0;
}
