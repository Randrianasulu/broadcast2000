#include <string.h>
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "setchannels.h"

SetChannels::SetChannels(MainWindow *mwindow)
 : BC_MenuItem("Output channels..."), Thread() 
{ this->mwindow = mwindow; }
 
int SetChannels::handle_event() 
{ 
	old_channels = new_channels = mwindow->output_channels;
	for(int i = 0 ; i < MAXCHANNELS; i++)
	{
		this->channel_positions[i] = mwindow->channel_positions[i];
	}
	mwindow->gui->disable_window();
	start(); 
return 0;
}

void SetChannels::run()
{
	SetChannelsWindow window(this);
	window.create_objects();
	int result = window.run_window();
	if(!result)
	{
		mwindow->undo->update_undo_audio("Output channels", 0);
		for(int i = 0 ; i < MAXCHANNELS; i++)
		{
			mwindow->channel_positions[i] = this->channel_positions[i];
		}
		mwindow->change_channels(old_channels, new_channels);
		//mwindow->defaults->update("OUTCHANNELS", new_channels);
		mwindow->undo->update_undo_audio();
		mwindow->draw();
		mwindow->changes_made = 1;
	}
	mwindow->gui->enable_window();
}


SetChannelsWindow::SetChannelsWindow(SetChannels *setchannels)
 : BC_Window("", MEGREY, ICONNAME ": Output Channels", 340, 240, 340, 270)
{
	this->setchannels = setchannels;
}

SetChannelsWindow::~SetChannelsWindow()
{
	delete ok;
	delete cancel;
	delete text;
	delete canvas;
}

int SetChannelsWindow::create_objects()
{
	add_tool(new BC_Title(5, 5, "Enter the output channels to use:"));
	add_tool(ok = new SetChannelsOkButton(this));
	add_tool(cancel = new SetChannelsCancelButton(this));
	
	add_tool(new BC_Title(5, 60, "Position the channels in space:"));
	add_tool(canvas = new SetChannelsCanvas(setchannels, 10, 80, 150, 150));

	char string[1024];
	sprintf(string, "%d", setchannels->old_channels);
	add_tool(text = new SetChannelsTextBox(setchannels, canvas, string));
	
	canvas->draw();
return 0;
}

SetChannelsOkButton::SetChannelsOkButton(SetChannelsWindow *window)
 : BC_BigButton(5, 240, "OK")
{
	this->window = window;
}

int SetChannelsOkButton::handle_event()
{
	window->set_done(0);
return 0;
}

int SetChannelsOkButton::keypress_event()
{
	if(window->get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}

SetChannelsCancelButton::SetChannelsCancelButton(SetChannelsWindow *window)
 : BC_BigButton(90, 240, "Cancel")
{
	this->window = window;
}

int SetChannelsCancelButton::handle_event()
{
	window->set_done(1);
return 0;
}

int SetChannelsCancelButton::keypress_event()
{
	if(window->get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}

SetChannelsTextBox::SetChannelsTextBox(SetChannels *setchannels, SetChannelsCanvas *canvas, char *text)
 : BC_TextBox(10, 30, 100, text)
{
	this->setchannels = setchannels;
	this->canvas = canvas;
}

int SetChannelsTextBox::handle_event()
{
	int result = atol(get_text());
	if(result > 0 && result < MAXCHANNELS)
	{
		setchannels->new_channels = result;
		canvas->draw();
	}
return 0;
}

SetChannelsCanvas::SetChannelsCanvas(SetChannels *setchannels, int x, int y, int w, int h)
 : BC_Canvas(x, y, w, h, BLACK)
{
	this->setchannels = setchannels;
	active_channel = -1;
	box_r = 10;
}

SetChannelsCanvas::~SetChannelsCanvas()
{
}

int SetChannelsCanvas::create_objects()
{
return 0;
}

int SetChannelsCanvas::draw(int angle)
{
	clear_box(0, 0, w, h);
	set_color(RED);
	int real_w = w - box_r * 2;
	int real_h = h - box_r * 2;
	int real_x = box_r;
	int real_y = box_r;
	draw_circle(real_x, real_y, real_w, real_h);
	
	int x, y, w, h;
	char string[32];
	set_color(MEYELLOW);
	for(int i = 0; i < setchannels->new_channels; i++)
	{
		get_dimensions(setchannels->channel_positions[i], x, y, w, h);
		draw_rectangle(x, y, w, h);
		sprintf(string, "%d", i + 1);
		draw_text(x + 2, y + box_r * 2 - 2, string);
	}
	if(angle > -1)
	{
		sprintf(string, "%d degrees", angle);
		draw_text(this->w / 2 - 40, this->h / 2, string);
	}
	
	flash();
return 0;
}

int SetChannelsCanvas::button_press()
{
// get active channel
	for(int i = 0; i < setchannels->new_channels; i++)
	{
		int x, y, w, h;
		get_dimensions(setchannels->channel_positions[i], x, y, w, h);
		if(cursor_x > x && cursor_y > y && cursor_x < x + w && cursor_y < y + h)
		{
			active_channel = i;
			xytopol(degree_offset, cursor_x - this->w / 2, cursor_y - this->h / 2);
			degree_offset -= setchannels->channel_positions[i];
			draw(setchannels->channel_positions[i]);
//printf("degrees %d degree offset %d\n", setchannels->channel_positions[i], degree_offset);
		}
	}
	return 1;
return 0;
}

int SetChannelsCanvas::button_release()
{
	active_channel = -1;
	draw(-1);
return 0;
}

int SetChannelsCanvas::cursor_motion()
{
	if(active_channel > -1)
	{
// get degrees of new channel
		int new_d;
		xytopol(new_d, cursor_x - this->w / 2, cursor_y - this->h / 2);
		new_d -= degree_offset;
		if(new_d < 0) new_d += 360;
		if(setchannels->channel_positions[active_channel] != new_d)
		{
			setchannels->channel_positions[active_channel] = new_d;
			draw(setchannels->channel_positions[active_channel]);
		}
		return 1;
	}
	return 0;
return 0;
}

int SetChannelsCanvas::get_dimensions(int channel_position, int &x, int &y, int &w, int &h)
{
	int real_w = this->w - box_r * 2;
	int real_h = this->h - box_r * 2;
	poltoxy(x, y, real_w / 2, channel_position);
	x += real_w / 2;
	y += real_h / 2;
	w = box_r * 2;
	h = box_r * 2;
return 0;
}


int SetChannelsCanvas::poltoxy(int &x, int &y, int r, int d)
{
	float radians = (float)(d - 90) / 360 * 2 * M_PI;
	y = (int)(sin(radians) * r);
	x = (int)(cos(radians) * r);
return 0;
}

int SetChannelsCanvas::xytopol(int &d, int x, int y)
{
	float x1, y1;
	float angle;
	y *= -1;
	x *= -1;
	
	if(x < 0 && y > 0){
		x1 = y;
		y1 = x;
	}else{
		x1 = x;
		y1 = y;
	}

	if(!y || !x){
		if(x < 0) angle = .5;
		else
		if(x > 0) angle = 1.5;
		else
		if(y < 0) angle = 1;
		else
		if(y > 0) angle = 0;
	}else{
		angle = atan(y1 / x1);
		angle /= M_PI;
	}

// fix angle

	if(x < 0 && y < 0){
		angle += .5;
	}else
	if(x > 0 && y > 0){
		angle += 1.5;
	}else
	if(x > 0 && y < 0){
		angle += 1.5;
	}

	if(x < 0 && y > 0) angle = -angle;
	angle /= 2;
	angle *= 360;
	d =  (int)angle;
return 0;
}


