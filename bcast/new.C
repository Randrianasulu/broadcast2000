#include <string.h>
#include "defaults.h"
#include "filehtal.h"
#include "levelwindow.h"
#include "mainundo.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "new.h"
#include "videowindow.h"

New::New(MainWindow *mwindow)
 : BC_MenuItem("New...", "n", 'n'), Thread()
{
	this->mwindow = mwindow;
	script = 0;
	already_running = 0;
}

int New::handle_event() { start(); return 0;
}

int New::set_script(FileHTAL *script)
{
	this->script = script;
return 0;
}

int New::run_script(FileHTAL *script)
{
	int script_result = 0, result = 0;

	while(!result && !script_result)
	{
		result = script->read_tag();

		if(!result)
		{
			if(script->tag.title_is("set_atracks"))
			{
				tracks = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_vtracks"))
			{
				vtracks = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_channels"))
			{
				channels = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_samplerate"))
			{
				sample_rate = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_framerate"))
			{
				frame_rate = script->tag.get_property_float(0);
			}
			else
			if(script->tag.title_is("set_trackw"))
			{
				track_w = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_trackh"))
			{
				track_h = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_outputw"))
			{
				output_w = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_outputh"))
			{
				output_h = script->tag.get_property_int(0);
			}
			else
			if(script->tag.title_is("set_aspectw"))
			{
				aspect_w = script->tag.get_property_float(0);
			}
			else
			if(script->tag.title_is("set_aspecth"))
			{
				aspect_h = script->tag.get_property_float(0);
			}
			else
			if(script->tag.title_is("ok"))
			{
				script_result = 1;
			}
			else
			{
				printf("New::run_script: Unrecognized command: %s\n", script->tag.get_title());
			}
		}
	}

	return 0;
return 0;
}


void New::run()
{
	int result = 0;
	if(already_running) return;
	already_running = 1;
	load_defaults();

	if(script)
	{
		result = run_script(script);
	}
	else
	{
		NewWindow newwindow(this, mwindow->display);
		newwindow.create_objects();
		result = newwindow.run_window();
	}

	if(result)
	{
// Aborted
		;
	}
	else
	{
		if(mwindow->gui) mwindow->gui->lock_window();
		mwindow->undo->update_undo_all("New", 0);
		mwindow->delete_project();
		if(mwindow->gui && channels != mwindow->output_channels)
		{
			mwindow->level_window->change_channels(channels);
			mwindow->gui->mainmenu->change_channels(mwindow->output_channels, channels);
		}
		mwindow->sample_rate = sample_rate;
		if(!mwindow->sample_rate) mwindow->sample_rate = 44100;
		mwindow->output_channels = channels;
		if(!mwindow->output_channels) mwindow->output_channels = 2;
		mwindow->frame_rate = frame_rate;
		if(!mwindow->frame_rate) mwindow->frame_rate = 15;
		mwindow->track_w = track_w;
		mwindow->track_h = track_h;
		mwindow->output_w = output_w;
		mwindow->output_h = output_h;
		mwindow->aspect_w = aspect_w;
		mwindow->aspect_h = aspect_h;
		if(!mwindow->track_w) mwindow->track_w = 240;
		mwindow->track_w /= 2;
		mwindow->track_w *= 2;
		if(!mwindow->track_h) mwindow->track_h = 120;
		mwindow->track_h /= 2;
		mwindow->track_h *= 2;
		if(!mwindow->output_w) mwindow->output_w = 240;
		mwindow->output_w /= 2;
		mwindow->output_w *= 2;
		if(!mwindow->output_h) mwindow->output_h = 120;
		mwindow->output_h /= 2;
		mwindow->output_h *= 2;

		for(int i = 0; i < vtracks; i++)
		{
			mwindow->add_video_track();
		}

		for(int i = 0; i < tracks; i++)
		{
			mwindow->add_audio_track();
		}

		mwindow->set_filename("");
		mwindow->undo->update_undo_all();
		mwindow->changes_made = 0;
		save_defaults();

		mwindow->video_window->resize_window();
		if(mwindow->gui) mwindow->gui->unlock_window();
	}
	script = 0;
	already_running = 0;
}

int New::load_defaults()
{
	tracks = mwindow->defaults->get("ATRACKS", 2);
	sample_rate = mwindow->defaults->get("SAMPLERATE", 44100);
	channels = mwindow->defaults->get("OUTCHANNELS", 2);
	
	vtracks = mwindow->defaults->get("VTRACKS", 0);
	frame_rate = mwindow->defaults->get("FRAMERATE", (float)15);
	track_w = mwindow->defaults->get("TRACKW", 640);
	track_h = mwindow->defaults->get("TRACKH", 480);
	output_w = mwindow->defaults->get("OUTPUTW", 640);
	output_h = mwindow->defaults->get("OUTPUTH", 480);
	aspect_w = mwindow->defaults->get("ASPECTW", (float)4);
	aspect_h = mwindow->defaults->get("ASPECTH", (float)3);
	auto_aspect = mwindow->defaults->get("AUTOASPECT", 0);
return 0;
}

int New::save_defaults()
{
	mwindow->defaults->update("ATRACKS", tracks);
	mwindow->defaults->update("SAMPLERATE", sample_rate);
	mwindow->defaults->update("OUTCHANNELS", channels);

	mwindow->defaults->update("VTRACKS", vtracks);
	mwindow->defaults->update("FRAMERATE", frame_rate);
	mwindow->defaults->update("TRACKW", track_w);
	mwindow->defaults->update("TRACKH", track_h);
	mwindow->defaults->update("OUTPUTW", output_w);
	mwindow->defaults->update("OUTPUTH", output_h);
	mwindow->defaults->update("ASPECTW", aspect_w);
	mwindow->defaults->update("ASPECTH", aspect_h);
	mwindow->defaults->update("AUTOASPECT", auto_aspect);
return 0;
}

int New::update_aspect(NewWindow *nwindow)
{
	if(auto_aspect)
	{
		char string[1024];
		mwindow->create_aspect_ratio(aspect_w, aspect_h, output_w, output_h);
		sprintf(string, "%.0f", aspect_w);
		nwindow->aspect_w_text->update(string);
		sprintf(string, "%.0f", aspect_h);
		nwindow->aspect_h_text->update(string);
	}
return 0;
}

NewWindow::NewWindow(New *new_thread, char *display)
 : BC_Window(display, MEGREY, ICONNAME ": New Project", 570, 260, 570, 260)
{
	this->new_thread = new_thread;
}

NewWindow::~NewWindow()
{
}

int NewWindow::create_objects()
{
	add_tool(new BC_Title(5, 5, "Parameters for the new project:"));
	add_tool(new NewOkButton(this));
	add_tool(new NewCancelButton(this));
	char string[1024];
	
	add_tool(new BC_Title(10, 60, "Audio Tracks:"));
	sprintf(string, "%d", new_thread->tracks);
	add_tool(new NewTracks(this, string));
	add_tool(new BC_Title(10, 90, "Channels:"));
	sprintf(string, "%d", new_thread->channels);
	add_tool(new NewChannels(this, string));
	add_tool(new BC_Title(10, 120, "Sample rate:"));
	sprintf(string, "%d", new_thread->sample_rate);
	add_tool(new NewRate(this, string));
	
	add_tool(new BC_Title(230, 60, "Video Tracks:"));
	sprintf(string, "%d", new_thread->vtracks);
	add_tool(new NewVTracks(this, string));
	add_tool(new BC_Title(230, 90, "Frame rate:"));
	sprintf(string, "%.2f", new_thread->frame_rate);
	add_tool(new NewFrameRate(this, string));
	add_tool(new BC_Title(230, 120, "Track Size:"));
	sprintf(string, "%d", new_thread->track_w);
	add_tool(new NewTrackW(this, string));
	add_tool(new BC_Title(405, 120, "x"));
	sprintf(string, "%d", new_thread->track_h);
	add_tool(new NewTrackH(this, string));
	add_tool(new BC_Title(230, 150, "Output Size:"));
	sprintf(string, "%d", new_thread->output_w);
	add_tool(output_w_button = new NewOutputW(this, string));
	add_tool(new BC_Title(405, 150, "x"));
	sprintf(string, "%d", new_thread->output_h);
	add_tool(output_h_button = new NewOutputH(this, string));
	add_tool(new BC_Title(230, 180, "Aspect ratio:"));
	sprintf(string, "%.0f", new_thread->aspect_w);
	add_tool(aspect_w_text = new NewAspectW(this, string));
	add_tool(new BC_Title(385, 180, ":"));
	sprintf(string, "%.0f", new_thread->aspect_h);
	add_tool(aspect_h_text = new NewAspectH(this, string));
	add_tool(new NewAspectAuto(this));
	add_tool(new NewCloneButton(this));
return 0;
}

NewOkButton::NewOkButton(NewWindow *nwindow)
 : BC_BigButton(120, 215, "OK")
{
	this->nwindow = nwindow;
}

NewOkButton::~NewOkButton()
{
}

int NewOkButton::handle_event()
{
	nwindow->set_done(0);
return 0;
}

int NewOkButton::keypress_event()
{
	if(nwindow->get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

NewCancelButton::NewCancelButton(NewWindow *nwindow)
 : BC_BigButton(340, 215, "Cancel")
{
	this->nwindow = nwindow;
}

int NewCancelButton::handle_event()
{
	nwindow->set_done(1);
return 0;
}

int NewCancelButton::keypress_event()
{
	if(nwindow->get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}

NewCloneButton::NewCloneButton(NewWindow *nwindow)
 : BC_BigButton(500, 120, "Clone")
{
	this->nwindow = nwindow;
}

int NewCloneButton::handle_event()
{
	nwindow->output_w_button->update(nwindow->new_thread->track_w);
	nwindow->output_h_button->update(nwindow->new_thread->track_h);
	nwindow->new_thread->output_w = nwindow->new_thread->track_w;
	nwindow->new_thread->output_h = nwindow->new_thread->track_h;
	nwindow->new_thread->update_aspect(nwindow);
return 0;
}

NewTracks::NewTracks(NewWindow *nwindow, char *text)
 : BC_TextBox(120, 60, 50, text)
{
	this->nwindow = nwindow;
}

int NewTracks::handle_event()
{
	nwindow->new_thread->tracks = atol(get_text());
return 0;
}

NewChannels::NewChannels(NewWindow *nwindow, char *text)
 : BC_TextBox(120, 90, 50, text)
{
	this->nwindow = nwindow;
} 

int NewChannels::handle_event()
{
	nwindow->new_thread->channels = atol(get_text());
return 0;
}

NewRate::NewRate(NewWindow *nwindow, char *text)
 : BC_TextBox(120, 120, 90, text)
{
	this->nwindow = nwindow;
}

int NewRate::handle_event()
{
	nwindow->new_thread->sample_rate = atol(get_text());
return 0;
}


NewVTracks::NewVTracks(NewWindow *nwindow, char *text)
 : BC_TextBox(330, 60, 50, text)
{
	this->nwindow = nwindow;
}

int NewVTracks::handle_event()
{
	nwindow->new_thread->vtracks = atol(get_text());
return 0;
}

NewFrameRate::NewFrameRate(NewWindow *nwindow, char *text)
 : BC_TextBox(330, 90, 70, text)
{
	this->nwindow = nwindow;
}

int NewFrameRate::handle_event()
{
	nwindow->new_thread->frame_rate = atof(get_text());
return 0;
}

NewTrackW::NewTrackW(NewWindow *nwindow, char *text)
 : BC_TextBox(330, 120, 70, text)
{
	this->nwindow = nwindow;
}

int NewTrackW::handle_event()
{
	nwindow->new_thread->track_w = atol(get_text());
return 0;
}

NewTrackH::NewTrackH(NewWindow *nwindow, char *text)
 : BC_TextBox(420, 120, 70, text)
{
	this->nwindow = nwindow;
}

int NewTrackH::handle_event()
{
	nwindow->new_thread->track_h = atol(get_text());
return 0;
}

NewOutputW::NewOutputW(NewWindow *nwindow, char *text)
 : BC_TextBox(330, 150, 70, text)
{
	this->nwindow = nwindow;
}

int NewOutputW::handle_event()
{
	nwindow->new_thread->output_w = atol(get_text());
	nwindow->new_thread->update_aspect(nwindow);
return 0;
}

NewOutputH::NewOutputH(NewWindow *nwindow, char *text)
 : BC_TextBox(420, 150, 70, text)
{
	this->nwindow = nwindow;
}

int NewOutputH::handle_event()
{
	nwindow->new_thread->output_h = atol(get_text());
	nwindow->new_thread->update_aspect(nwindow);
return 0;
}

NewAspectW::NewAspectW(NewWindow *nwindow, char *text)
 : BC_TextBox(330, 180, 50, text)
{
	this->nwindow = nwindow;
}

int NewAspectW::handle_event()
{
	nwindow->new_thread->aspect_w = atof(get_text());
return 0;
}

NewAspectH::NewAspectH(NewWindow *nwindow, char *text)
 : BC_TextBox(400, 180, 50, text)
{
	this->nwindow = nwindow;
}

int NewAspectH::handle_event()
{
	nwindow->new_thread->aspect_h = atof(get_text());
return 0;
}

NewAspectAuto::NewAspectAuto(NewWindow *nwindow)
 : BC_CheckBox(460, 185, 16, 16, nwindow->new_thread->auto_aspect, "Auto")
{
	this->nwindow = nwindow;
}
NewAspectAuto::~NewAspectAuto()
{
}
int NewAspectAuto::handle_event()
{
	nwindow->new_thread->auto_aspect = get_value();
	nwindow->new_thread->update_aspect(nwindow);
return 0;
}
