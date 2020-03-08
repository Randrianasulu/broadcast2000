#include <string.h>
#include "buttonbar.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "playbackengine.h"
#include "preferences.h"
#include "record.h"
#include "tracks.h"

ButtonBar::ButtonBar(MainWindowGUI *gui, int x, int y, int w, int h)
 : BC_SubWindow(x, y, w, h, MEGREY) 
{ 
	this->gui = gui;
	this->mwindow = gui->mwindow; 
}

ButtonBar::~ButtonBar()
{
	delete record_button;

	delete rewind_button;
	delete fast_reverse;
	delete reverse_play;
	delete stop_button;
	delete forward_play;
	delete fast_play;
	delete end_button;

	delete x_title;
	delete expand_x_button;
	delete zoom_x_button;

	delete y_title;
	delete expand_y_button;
	delete zoom_y_button;

	delete t_title;
	delete expand_t_button;
	delete zoom_t_button;

	// delete v_title;
	//delete expand_v_button;
	//delete zoom_v_button;

	delete fit_button;
	delete label_button;
	delete cut_button;
	delete copy_button;
	delete paste_button;
}

int ButtonBar::create_objects()
{
	add_border();
	if(mwindow->tracks_vertical)
	{
		int x = 3, y = 3;
		add_tool(record_button = new Record(mwindow->defaults, mwindow, x, y)); y += 30;

// configure the transport buttons
		add_tool(rewind_button = new RewindButton(x, y, mwindow)); y += 25;
		add_tool(fast_reverse = new FastReverseButton(mwindow, this, x , y)); y += 25;
		add_tool(reverse_play = new ReverseButton(mwindow, this, x, y)); y += 25;
		add_tool(frame_reverse_play = new FrameReverseButton(mwindow, this, x, y)); y += 25;
		add_tool(stop_button = new StopButton(x, y, mwindow)); y += 25;
		add_tool(frame_forward_play = new FramePlayButton(mwindow, this, x, y)); y += 25;
		add_tool(forward_play = new PlayButton(mwindow, this, x, y)); y += 25;
		add_tool(fast_play = new FastPlayButton(mwindow, this, x, y)); y += 25;
		add_tool(end_button = new EndButton(x, y, mwindow)); y += 25;

		add_tool(x_title = new BC_Title(x + 3, y, "X:")); y += 15;
		add_tool(expand_x_button = new ExpandX(x, y, mwindow)); //y += 20;
		add_tool(zoom_x_button = new ZoomX(x + 20, y, mwindow)); y += 20;

		add_tool(y_title = new BC_Title(x + 3, y, "Y:")); y += 15;
		add_tool(expand_y_button = new ExpandY(x, y, mwindow)); //y += 20;
		add_tool(zoom_y_button = new ZoomY(x + 20, y, mwindow)); y += 20;

		add_tool(t_title = new BC_Title(x + 3, y, "T:")); y += 15;
		add_tool(expand_t_button = new ExpandTrack(x, y, mwindow)); //y += 20;
		add_tool(zoom_t_button = new ZoomTrack(x + 20, y, mwindow)); y += 25;

		//add_tool(v_title = new BC_Title(x + 3, y, "V:")); y += 45;
		//add_tool(expand_v_button = new ExpandVideo(x, y, mwindow)); y += 20;
		//add_tool(zoom_v_button = new ZoomVideo(x, y mwindow)); y += 25;

		add_tool(fit_button = new Fit(x, y, mwindow)); y += fit_button->h + 5;
		add_tool(label_button = new LabelButton(x, y, mwindow)); y += label_button->h;
		add_tool(cut_button = new Cut(x, y, mwindow)); y += cut_button->h;
		add_tool(copy_button = new Copy(x, y, mwindow)); y += copy_button->h;
		add_tool(paste_button = new Paste(x, y, mwindow)); y += paste_button->h;
	}
	else
	{
		int x = 3, y = 5;
		add_tool(record_button = new Record(mwindow->defaults, mwindow, x, y - 2)); x += 30;

// configure the transport buttons
		add_tool(rewind_button = new RewindButton(x, y - 2, mwindow)); x += 25;
		add_tool(fast_reverse = new FastReverseButton(mwindow, this, x, y - 2)); x += 25;
		add_tool(reverse_play = new ReverseButton(mwindow, this, x, y - 2)); x += 25;
		add_tool(frame_reverse_play = new FrameReverseButton(mwindow, this, x, y - 2)); x += 25;
		add_tool(stop_button = new StopButton(x, y - 2, mwindow)); x += 25;
		add_tool(frame_forward_play = new FramePlayButton(mwindow, this, x, y - 2)); x += 25;
		add_tool(forward_play = new PlayButton(mwindow, this, x, y - 2)); x += 25;
		add_tool(fast_play = new FastPlayButton(mwindow, this, x, y - 2)); x += 25;

		add_tool(end_button = new EndButton(x, y - 2, mwindow)); x += 25;

		add_tool(x_title = new BC_Title(x, y + 1, "X:")); x += 15;
		add_tool(expand_x_button = new ExpandX(x, y, mwindow)); x += 20;
		add_tool(zoom_x_button = new ZoomX(x, y, mwindow)); x += 20;

		add_tool(y_title = new BC_Title(x, y + 1, "Y:")); x += 15;
		add_tool(expand_y_button = new ExpandY(x, y, mwindow)); x += 20;
		add_tool(zoom_y_button = new ZoomY(x, y, mwindow)); x += 20;

		add_tool(t_title = new BC_Title(x, y + 1, "T:")); x += 15;
		add_tool(expand_t_button = new ExpandTrack(x, y, mwindow)); x += 20;
		add_tool(zoom_t_button = new ZoomTrack(x, y, mwindow)); x += 25;

		//add_tool(v_title = new BC_Title(x, y + 1, "Video:")); x += 45;
		//add_tool(expand_v_button = new ExpandVideo(x, y, mwindow)); x += 20;
		//add_tool(zoom_v_button = new ZoomVideo(x, y mwindow)); x += 25;

		add_tool(fit_button = new Fit(x, y, mwindow)); x += fit_button->w + 5;
		add_tool(label_button = new LabelButton(x, y, mwindow)); x += label_button->w;
		add_tool(cut_button = new Cut(x, y, mwindow)); x += cut_button->w;
		add_tool(copy_button = new Copy(x, y, mwindow)); x += copy_button->w;
		add_tool(paste_button = new Paste(x, y, mwindow)); x += paste_button->w;
	}
	
	active_button = 0;
	reverse = 0;
	speed = 0;
return 0;
}

int ButtonBar::resize_event(int w, int h)
{
	if(mwindow->tracks_vertical)
	resize_window(get_x(), get_y(), this->get_w(), h - mwindow->gui->menu_h() - 24);
	else
	resize_window(get_x(), get_y(), w, this->get_h());
return 0;
}

int ButtonBar::flip_vertical(int w, int h)
{
	if(mwindow->tracks_vertical)
	{
		int x = 3, y = 3;
		resize_window(0, mwindow->gui->menu_h(), BUTTONBARWIDTH, h - mwindow->gui->menu_h() - 24);
		record_button->resize_tool(x, y); y += 30;

// configure the transport buttons
		rewind_button->resize_tool(x, y); y += 25;
		fast_reverse->resize_tool(x, y); y += 25;
		reverse_play->resize_tool(x, y); y += 25;
		frame_reverse_play->resize_tool(x, y); y += 25;
		stop_button->resize_tool(x, y); y += 25;
		frame_forward_play->resize_tool(x, y); y += 25;
		forward_play->resize_tool(x, y); y += 25;
		fast_play->resize_tool(x, y); y += 25;

		end_button->resize_tool(x, y); y += 25;

		x_title->resize_tool(x + 3, y); y += 15;
		expand_x_button->resize_tool(x, y); //y += 20;
		zoom_x_button->resize_tool(x + 20, y); y += 20;

		y_title->resize_tool(x + 3, y); y += 15;
		expand_y_button->resize_tool(x, y); //y += 20;
		zoom_y_button->resize_tool(x + 20, y); y += 20;

		t_title->resize_tool(x + 3, y); y += 15;
		expand_t_button->resize_tool(x, y); //y += 20;
		zoom_t_button->resize_tool(x + 20, y); y += 25;

		//v_title->resize_tool(x + 3, y); y += 45;
		//expand_v_button->resize_tool(x, y); y += 20;
		//zoom_v_button->resize_tool(x, y); y += 25;

		fit_button->resize_tool(x, y); y += fit_button->h + 5;
		label_button->resize_tool(x, y); y += label_button->h;
		cut_button->resize_tool(x, y); y += cut_button->h;
		copy_button->resize_tool(x, y); y += copy_button->h;
		paste_button->resize_tool(x, y); y += paste_button->h;
	}
	else
	{
		int x = 3, y = 5;
		resize_window(0, mwindow->gui->menu_h(), w, BUTTONBARHEIGHT);
		record_button->resize_tool(x, y - 2); x += 30;

// configure the transport buttons
		rewind_button->resize_tool(x, y - 2); x += 25;
		fast_reverse->resize_tool(x, y - 2); x += 25;
		reverse_play->resize_tool(x, y - 2); x += 25;
		frame_reverse_play->resize_tool(x, y - 2); x += 25;
		stop_button->resize_tool(x, y - 2); x += 25;
		frame_forward_play->resize_tool(x, y - 2); x += 25;
		forward_play->resize_tool(x, y - 2); x += 25;
		fast_play->resize_tool(x, y - 2); x += 25;

		end_button->resize_tool(x, y - 2); x += 25;

		x_title->resize_tool(x, y + 1); x += 15;
		expand_x_button->resize_tool(x, y); x += 20;
		zoom_x_button->resize_tool(x, y); x += 20;

		y_title->resize_tool(x, y + 1); x += 15;
		expand_y_button->resize_tool(x, y); x += 20;
		zoom_y_button->resize_tool(x, y); x += 20;

		t_title->resize_tool(x, y + 1); x += 15;
		expand_t_button->resize_tool(x, y); x += 20;
		zoom_t_button->resize_tool(x, y); x += 25;

		//v_title->resize_tool(x, y + 1); x += 45;
		//expand_v_button->resize_tool(x, y); x += 20;
		//zoom_v_button->resize_tool(x, y); x += 25;

		fit_button->resize_tool(x, y); x += fit_button->w + 5;
		label_button->resize_tool(x, y); x += label_button->w;
		cut_button->resize_tool(x, y); x += cut_button->w;
		copy_button->resize_tool(x, y); x += copy_button->w;
		paste_button->resize_tool(x, y); x += paste_button->w;
	}
return 0;
}

int ButtonBar::keypress_event()
{
	if(gui->shift_down()) return  0;
	return transport_keys(get_keypress());
return 0;
}

int ButtonBar::transport_keys(int key)
{
	int is_playing_back = mwindow->playback_engine->is_playing_back;
	int result = 0;

	switch(key)
	{
		case KPPLUS:        handle_transport(fast_reverse, 1, mwindow->preferences->scrub_speed);         result = 1; break;
		case KP6:           handle_transport(reverse_play, 1, 1);         result = 1; break;
		case KP5:           handle_transport(fast_reverse, 1, .5);        result = 1; break;
		case KP4:           handle_transport(frame_reverse_play, 1, FRAME_SPEED); result = 1; break;
		case KP1:           handle_transport(frame_forward_play, 0, FRAME_SPEED); result = 1; break;
		case KP2:           handle_transport(fast_play, 0, .5);           result = 1; break;
		case KP3:           handle_transport(forward_play, 0, 1);         result = 1; break;
		case KPENTER:       handle_transport(fast_play, 0, mwindow->preferences->scrub_speed);            result = 1; break;
		case KPINS:         mwindow->stop_playback(1);                    result = 1; break;
		case ' ':           handle_transport(0, 0, 1);                    result = 1; break;
		case 'k':           mwindow->stop_playback(1);					  result = 1; break;
	}

	return result;
return 0;
}


int ButtonBar::start_playback(BC_PlayButton *button, int reverse, float speed)
{
// start from wherever cursor is
	long playback_position = mwindow->get_playback_position(); 

// stop any current playback and update its button
	mwindow->stop_playback(1);             
// change button to a pause button
	if(button) button->update(2);
	active_button = button;
// set speed, direction, and position
	mwindow->set_playback_range(playback_position, reverse, speed);
// want to update this button if finished
	mwindow->arm_playback(1, 1, 0, 0);
// start playback
	mwindow->start_playback();         
return 0;
}

int ButtonBar::handle_transport(BC_PlayButton *button, int reverse, float speed)
{
	switch(mwindow->playback_engine->is_playing_back)
	{
		case 0:
// start playback
// space pressed so assume forward play
			if(!button) button = forward_play;
			start_playback(button, reverse, speed);
			break;

		case 1:
// resume
// another button is paused so change direction
			if(button && (button != active_button || reverse != this->reverse || speed != this->speed))
			{
// start in this direction
				start_playback(button, reverse, speed);
			}
			else
			if(!button || button == active_button)
			{
// this button is paused or a space so resume
// pause is in same direction
				if(active_button) active_button->update(2);
				mwindow->start_playback();
			}
			break;

		case 2:
			if(button && (button != active_button || reverse != this->reverse || speed != this->speed))
			{
// different button pressed so change direction
				start_playback(button, reverse, speed);
			}
			else
			if(!button || button == active_button)
			{
// same button pressed or space so pause
				mwindow->stop_playback();
				mwindow->arm_playback(1, 1, 0, 0);       // keeps the previous range
				if(active_button) active_button->update(0);
			}
			break;
	}
	this->reverse = reverse;
	this->speed = speed;
return 0;
}

int ButtonBar::pause_transport()
{
	if(active_button) active_button->update(0);
return 0;
}


int ButtonBar::reset_transport()
{
	fast_reverse->reset_button();
	reverse_play->reset_button();
	forward_play->reset_button();
	frame_reverse_play->reset_button();
	frame_forward_play->reset_button();
	fast_play->reset_button();
return 0;
}



RecButton::RecButton(int x, int y, MainWindow *mwindow)
 : BC_RecButton(x, y, 24, 24) 
{ this->mwindow = mwindow; }
int RecButton::handle_event()
{
//printf("Rec pressed\n");
return 0;
}
int RecButton::keypress_event()
{
	if(get_keypress() == 18) { handle_event(); return 1; }
	return 0;
return 0;
}

RewindButton::RewindButton(int x, int y, MainWindow *mwindow)
 : BC_RewindButton(x, y, 24, 24) 
{ this->mwindow = mwindow; }
int RewindButton::handle_event()
{	
	mwindow->stop_playback(1);
	mwindow->goto_start();
return 0;
}

FastReverseButton::FastReverseButton(MainWindow *mwindow, ButtonBar *bar, int x, int y)
 : BC_FastReverseButton(x, y, 24, 24) 
{ this->mwindow = mwindow; this->bar = bar; }
int FastReverseButton::handle_event() 
{
	bar->handle_transport(bar->fast_reverse, 1, mwindow->preferences->scrub_speed);	
return 0;
}
int FastReverseButton::button_press()
{
	return 0;
return 0;
}
int FastReverseButton::button_release()
{
	return 0;
return 0;
}

// Reverse playback normal speed

ReverseButton::ReverseButton(MainWindow *mwindow, ButtonBar *bar, int x, int y)
 : BC_ReverseButton(x, y, 24, 24) 
{ this->mwindow = mwindow; this->bar = bar; }
int ReverseButton::handle_event()
{
	bar->handle_transport(bar->reverse_play, 1, 1);	
return 0;
}

// Reverse playback one frame

FrameReverseButton::FrameReverseButton(MainWindow *mwindow, ButtonBar *bar, int x, int y)
 : BC_FrameReverseButton(x, y, 24, 24)
{ this->mwindow = mwindow; this->bar = bar; }
int FrameReverseButton::handle_event()
{
	bar->handle_transport(bar->frame_reverse_play, 1, FRAME_SPEED);
return 0;
}

// forward playback normal speed

PlayButton::PlayButton(MainWindow *mwindow, ButtonBar *bar, int x, int y)
 : BC_ForwardButton(x, y, 24, 24) 
{ this->mwindow = mwindow; this->bar = bar; }
int PlayButton::handle_event()
{
	bar->handle_transport(bar->forward_play, 0, 1);	
return 0;
}
int PlayButton::keypress_event()
{ 
	if(get_keypress() == 'p') { handle_event(); return 1; }
	return 0;
return 0;
}



// forward playback one frame

FramePlayButton::FramePlayButton(MainWindow *mwindow, ButtonBar *bar, int x, int y)
 : BC_FrameForwardButton(x, y, 24, 24) 
{ this->mwindow = mwindow; this->bar = bar; }
int FramePlayButton::handle_event()
{
	bar->handle_transport(bar->frame_forward_play, 0, FRAME_SPEED);
return 0;
}



FastPlayButton::FastPlayButton(MainWindow *mwindow, ButtonBar *bar, int x, int y)
 : BC_FastForwardButton(x, y, 24, 24) 
{ this->mwindow = mwindow; this->bar = bar; }
int FastPlayButton::handle_event() 
{ 
	bar->handle_transport(bar->fast_play, 0, mwindow->preferences->scrub_speed);
return 0;
}
int FastPlayButton::button_press()
{
	return 0;
return 0;
}
int FastPlayButton::button_release()
{
return 0;
}

EndButton::EndButton(int x, int y, MainWindow *mwindow)
 : BC_EndButton(x, y, 24, 24) 
{ this->mwindow = mwindow; }
int EndButton::handle_event()
{	
	mwindow->stop_playback(1);
	mwindow->goto_end();
return 0;
}

StopButton::StopButton(int x, int y, MainWindow *mwindow)
 : BC_StopButton(x, y, 24, 24) 
{ this->mwindow = mwindow; }
int StopButton::handle_event()
{
	mwindow->stop_playback(1);
return 0;
}
int StopButton::keypress_event()
{ 
	if(get_keypress() == 'k') { handle_event(); return 1; }
	return 0;
return 0;
}



ExpandX::ExpandX(int x, int y, MainWindow *mwindow)
 : BC_UpTriangleButton(x, y, 20, 20) 
{ this->mwindow = mwindow; }
int ExpandX::handle_event()
{	
	mwindow->expand_sample(); 
return 0;
}
int ExpandX::keypress_event()
{ 
	if(get_keypress() == UP && !ctrl_down()) { handle_event(); return 1; }
	return 0;
return 0;
}


ZoomX::ZoomX(int x, int y, MainWindow *mwindow)
 : BC_DownTriangleButton(x, y, 20, 20) 
{ this->mwindow = mwindow; }
int ZoomX::handle_event()
{		
	mwindow->zoom_in_sample(); 
return 0;
}
int ZoomX::keypress_event()
{ 
	if(get_keypress() == DOWN && !ctrl_down()) { handle_event(); return 1; }
	return 0;
return 0;
}


ExpandY::ExpandY(int x, int y, MainWindow *mwindow)
 : BC_UpTriangleButton(x, y, 20, 20) 
{ this->mwindow = mwindow; }
int ExpandY::handle_event()
{	
	mwindow->expand_y();
return 0;
}
int ExpandY::keypress_event()
{ 
	if(get_keypress() == UP && ctrl_down()) { handle_event(); return 1; }
	return 0;
return 0;
}


ZoomY::ZoomY(int x, int y, MainWindow *mwindow)
 : BC_DownTriangleButton(x, y, 20, 20) 
{ this->mwindow = mwindow; }
int ZoomY::handle_event()
{
	mwindow->zoom_in_y();
return 0;
}
int ZoomY::keypress_event()
{ 
	if(get_keypress() == DOWN && ctrl_down()) { handle_event(); return 1; }
	return 0;
return 0;
}


ExpandTrack::ExpandTrack(int x, int y, MainWindow *mwindow)
 : BC_UpTriangleButton(x, y, 20, 20) 
{ this->mwindow = mwindow; }
int ExpandTrack::handle_event()
{	
	mwindow->expand_t();
return 0;
}
int ExpandTrack::keypress_event()
{ 
	if(get_keypress() == PGUP && ctrl_down()) { handle_event(); return 1; }
	return 0;
return 0;
}


ZoomTrack::ZoomTrack(int x, int y, MainWindow *mwindow)
 : BC_DownTriangleButton(x, y, 20, 20) 
{ this->mwindow = mwindow; }
int ZoomTrack::handle_event()
{
	mwindow->zoom_in_t();
return 0;
}
int ZoomTrack::keypress_event()
{ 
	if(get_keypress() == PGDN && ctrl_down()) { handle_event(); return 1; }
	return 0;
return 0;
}


ExpandVideo::ExpandVideo(int x, int y, MainWindow *mwindow)
 : BC_UpTriangleButton(x, y, 20, 20) 
{ this->mwindow = mwindow; }
int ExpandVideo::handle_event()
{
	printf("ExpandVideo\n");
return 0;
}

ZoomVideo::ZoomVideo(int x, int y, MainWindow *mwindow)
 : BC_DownTriangleButton(x, y, 20, 20) 
{ this->mwindow = mwindow; }
int ZoomVideo::handle_event()
{
	printf("ZoomVideo\n");
return 0;
}

Fit::Fit(int x, int y, MainWindow *mwindow)
 : BC_SmallButton(x, y, 40, "Fit") 
{ this->mwindow = mwindow; }
int Fit::handle_event()
{
	mwindow->fit_sample();
return 0;
}
int Fit::keypress_event()
{ 
	if(get_keypress() == 'f') { handle_event(); return 1; }
	return 0;
return 0;
}


LabelButton::LabelButton(int x, int y, MainWindow *mwindow)
 : BC_SmallButton(x, y, 45, "Label") 
{ this->mwindow = mwindow; }
int LabelButton::handle_event()
{
	mwindow->toggle_label();
return 0;
}
int LabelButton::keypress_event()
{ 
	if(get_keypress() == 'l') { handle_event(); return 1; }
	return 0;
return 0;
}


Cut::Cut(int x, int y, MainWindow *mwindow)
 : BC_SmallButton(x, y, 40, "Cut") 
{ this->mwindow = mwindow; }
int Cut::handle_event()
{
	mwindow->stop_playback(1);
// save the before undo
	mwindow->undo->update_undo_edits("Cut", 0);
	mwindow->cut(mwindow->selectionstart, mwindow->selectionend);
// save the after undo
	mwindow->undo->update_undo_edits();
return 0;
}
int Cut::keypress_event()
{ 
	if(get_keypress() == 'x') { handle_event(); return 1; }
	return 0;
return 0;
}


Copy::Copy(int x, int y, MainWindow *mwindow)
 : BC_SmallButton(x, y, 45, "Copy") 
{ this->mwindow = mwindow; }
int Copy::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->tracks->copy(mwindow->selectionstart, mwindow->selectionend);
return 0;
}
int Copy::keypress_event()
{ 
	if(get_keypress() == 'c') { handle_event(); return 1; }
	return 0;
return 0;
}


Paste::Paste(int x, int y, MainWindow *mwindow)
 : BC_SmallButton(x, y, 45, "Paste") 
{ this->mwindow = mwindow; }
int Paste::handle_event()
{
	mwindow->stop_playback(1);
// save the before undo
	mwindow->undo->update_undo_edits("Paste", 0);
	mwindow->tracks->paste(mwindow->selectionstart, mwindow->selectionend);
// save the after undo
	mwindow->undo->update_undo_edits();
return 0;
}
int Paste::keypress_event()
{ 
	if(get_keypress() == 'v') { handle_event(); return 1; }
	return 0;
return 0;
}

