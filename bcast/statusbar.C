#include <string.h>
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "preferences.h"
#include "statusbar.h"
#include "tracks.h"

StatusBar::StatusBar(MainWindowGUI *gui)
 : BC_SubWindow(0, gui->get_h() - STATUSBAR_PIXELS, gui->get_w(), STATUSBAR_PIXELS, MEGREY) 
{
	this->gui = gui;
	mwindow = gui->mwindow;
	old_position = 0;
}

StatusBar::~StatusBar()
{
	delete from_value;
	delete length_value;
	delete to_value;
	delete zoom_value;
	//printf("~StatusBar\n");
}

int StatusBar::create_objects()
{
	add_border();
//	add_tool(new BC_Title(5, 4, "Fr:", MEDIUMFONT, MEYELLOW));
	add_tool(new BC_Title(105, 4, "...", MEDIUMFONT, MEYELLOW));
	add_tool(new BC_Title(220, 4, "...", MEDIUMFONT, MEYELLOW));
//	add_tool(new BC_Title(340, 4, ":", MEDIUMFONT, MEYELLOW));
//	add_tool(new BC_Title(450, 4, "Zm:", MEDIUMFONT, MEYELLOW));
	
	add_tool(from_value = new FromTextBox(mwindow, this, 10, 4));
	add_tool(length_value = new LengthTextBox(mwindow, this, 120, 4));
	add_tool(to_value = new ToTextBox(mwindow, this, 240, 4));
	add_tool(playback_value = new BC_Title(340, 4, "--", MEDIUMFONT, RED));
	
	add_tool(zoom_value = new BC_Title(440, 4, "--", MEDIUMFONT, BLACK));
return 0;
}

int StatusBar::draw()
{
	update();
return 0;
}

int StatusBar::update()
{
	from_value->update_position(mwindow->selectionstart);
	
	long length = mwindow->selectionend - mwindow->selectionstart;
	totext(string, 
		length, 
		mwindow->sample_rate, 
		mwindow->preferences->time_format, 
		mwindow->frame_rate,
		mwindow->preferences->frames_per_foot);
	length_value->update(string);

	to_value->update_position(mwindow->selectionend);
	
	totext(string2, 
		mwindow->tracks->view_pixels() * mwindow->zoom_sample, 
		mwindow->sample_rate, 
		mwindow->preferences->time_format, 
		mwindow->frame_rate,
		mwindow->preferences->frames_per_foot);
	sprintf(string, "%s x %.2f", string2, (float)mwindow->zoom_y / mwindow->zoom_track);
	zoom_value->update(string);
return 0;
}

int StatusBar::update_playback(long new_position)
{
	if(new_position != old_position)
	{
		totext(string, 
				new_position, 
				mwindow->sample_rate, 
				mwindow->preferences->time_format, 
				mwindow->frame_rate,
				mwindow->preferences->frames_per_foot);
		playback_value->update(string);
		old_position = new_position;
	}
return 0;
}

int StatusBar::resize_event(int w, int h)
{
// don't change anything but y and width
	resize_window(0, h - this->get_h(), w, this->get_h());
return 0;
}

int StatusBar::set_selection(int which_one)
{
	long start_position = mwindow->selectionstart;
	long end_position = mwindow->selectionend;
	long length = end_position - start_position;

	switch(which_one)
	{
		case 2:
			start_position = fromtext(from_value->get_text(), 
				mwindow->sample_rate, 
				mwindow->preferences->time_format, 
				mwindow->frame_rate,
				mwindow->preferences->frames_per_foot);
			length = fromtext(length_value->get_text(), 
				mwindow->sample_rate, 
				mwindow->preferences->time_format, 
				mwindow->frame_rate,
				mwindow->preferences->frames_per_foot);
			end_position = start_position + length;
			break;

		default:
			start_position = fromtext(from_value->get_text(), 
				mwindow->sample_rate, 
				mwindow->preferences->time_format, 
				mwindow->frame_rate,
				mwindow->preferences->frames_per_foot);
			end_position = fromtext(to_value->get_text(), 
				mwindow->sample_rate, 
				mwindow->preferences->time_format, 
				mwindow->frame_rate,
				mwindow->preferences->frames_per_foot);
			break;
	}
	mwindow->set_selection(start_position, end_position);
return 0;
}









FromTextBox::FromTextBox(MainWindow *mwindow, StatusBar *statusbar, int x, int y)
 : BC_TextBox(x, y, 90, "--", 0)
{
	this->mwindow = mwindow;
	this->statusbar = statusbar;
}

int FromTextBox::handle_event()
{
	if(get_keypress() == 13)
		statusbar->set_selection(1);
return 0;
}

int FromTextBox::update_position(long new_position)
{
	totext(string, 
		new_position, 
		mwindow->sample_rate, 
		mwindow->preferences->time_format, 
		mwindow->frame_rate,
		mwindow->preferences->frames_per_foot);
	update(string);
return 0;
}






LengthTextBox::LengthTextBox(MainWindow *mwindow, StatusBar *statusbar, int x, int y)
 : BC_TextBox(x, y, 90, "--", 0)
{
	this->mwindow = mwindow;
	this->statusbar = statusbar;
}

int LengthTextBox::handle_event()
{
	if(get_keypress() == 13)
		statusbar->set_selection(2);
return 0;
}

int LengthTextBox::update_position(long new_position)
{
	totext(string, 
		new_position, 
		mwindow->sample_rate, 
		mwindow->preferences->time_format, 
		mwindow->frame_rate,
		mwindow->preferences->frames_per_foot);
	update(string);
return 0;
}





ToTextBox::ToTextBox(MainWindow *mwindow, StatusBar *statusbar, int x, int y)
 : BC_TextBox(x, y, 90, "--", 0)
{
	this->mwindow = mwindow;
	this->statusbar = statusbar;
}

int ToTextBox::handle_event()
{
	if(get_keypress() == 13)
		statusbar->set_selection(3);
return 0;
}

int ToTextBox::update_position(long new_position)
{
	totext(string, 
		new_position, 
		mwindow->sample_rate, 
		mwindow->preferences->time_format, 
		mwindow->frame_rate,
		mwindow->preferences->frames_per_foot);
	update(string);
return 0;
}
