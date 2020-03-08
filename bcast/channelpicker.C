#include <string.h>
#include "channel.h"
#include "channeledit.h"
#include "channelpicker.h"
#include "chantables.h"
#include "record.h"
#include "recordengine.h"


ChannelPicker::ChannelPicker(RecordEngine *engine, 
	ArrayList <Channel*> *channeldb, 
	BC_WindowBase *window)
{
	this->channeldb = channeldb;
	this->engine = engine;
	this->window = window;
	current_channel = new Channel;
}

ChannelPicker::~ChannelPicker()
{
	delete current_channel;
}

int ChannelPicker::create_objects(int x, int y, int h)
{
	window->add_tool(channel_button = new ChannelButton(x, y + 3, this));
	x += channel_button->get_w() + 5;
	window->add_tool(up_channel = new UpChannel(x, y + 2, this));
	x += up_channel->get_w();
	window->add_tool(dn_channel = new DnChannel(x, y + 2, this));
	x += dn_channel->get_w();
return 0;
}

int ChannelPicker::close_threads()
{
	channel_button->thread->close_threads();
return 0;
}

char* ChannelPicker::get_source_name(Channel *channel)
{
	if(channel->entry < chanlists[channel->freqtable].count)
		return chanlists[channel->freqtable].list[channel->entry].name;
	else
		return chanlists[channel->freqtable].list[0].name;
}

char* ChannelPicker::current_channel_name()
{
	if(engine->record->current_channel < channeldb->total &&
		engine->record->current_channel > -1)
		return channeldb->values[engine->record->current_channel]->title;
	else
		return "Channel";
}

int ChannelPicker::channel_down()
{
	engine->record->current_channel--;
	if(engine->record->current_channel < 0)
		engine->record->current_channel = channeldb->total - 1;
	
	if(engine->record->current_channel < channeldb->total)
	{
		channel_button->update(current_channel_name());
		if(engine->record->current_channel > -1)
		{
			engine->change_channel(channeldb->values[engine->record->current_channel]);
		}
	}
return 0;
}

int ChannelPicker::channel_up()
{
	engine->record->current_channel++;
	if(engine->record->current_channel > channeldb->total - 1)
		engine->record->current_channel = 0;
	
	if(engine->record->current_channel < channeldb->total)
	{
		channel_button->update(current_channel_name());
		if(engine->record->current_channel > -1)
		{
			engine->change_channel(channeldb->values[engine->record->current_channel]);
		}
	}
return 0;
}

int ChannelPicker::set_brightness(int value)
{
	engine->record->video_brightness = value;
	engine->set_video_picture();
return 0;
}

int ChannelPicker::set_hue(int value)
{
	engine->record->video_hue = value;
	engine->set_video_picture();
return 0;
}

int ChannelPicker::set_color(int value)
{
	engine->record->video_color = value;
	engine->set_video_picture();
return 0;
}

int ChannelPicker::set_contrast(int value)
{
	engine->record->video_contrast = value;
	engine->set_video_picture();
return 0;
}

int ChannelPicker::set_whiteness(int value)
{
	engine->record->video_whiteness = value;
	engine->set_video_picture();
return 0;
}



ChannelButton::ChannelButton(int x, int y, ChannelPicker *channel_picker)
 : BC_SmallButton(x, y, 80, channel_picker->current_channel_name())
{
	this->channel_picker = channel_picker;
	thread = new ChannelEditThread(channel_picker);
}

ChannelButton::~ChannelButton()
{
	delete thread;
}

int ChannelButton::handle_event()
{
	thread->start();
return 0;
}

UpChannel::UpChannel(int x, int y, ChannelPicker *channel_picker)
 : BC_UpTriangleButton(x, y, 20, 20)
{
	this->channel_picker = channel_picker;
}
UpChannel::~UpChannel()
{
}
int UpChannel::handle_event()
{
	channel_picker->channel_up();
return 0;
}
int UpChannel::keypress_event()
{
	if(get_keypress() == PGUP)
	{
		handle_event();
		trap_keypress();
	}
	else
	return 0;
return 0;
}

DnChannel::DnChannel(int x, int y, ChannelPicker *channel_picker)
 : BC_DownTriangleButton(x, y, 20, 20)
{
	this->channel_picker = channel_picker;
}
DnChannel::~DnChannel()
{
}
int DnChannel::handle_event()
{
	channel_picker->channel_down();
return 0;
}

int DnChannel::keypress_event()
{
	if(get_keypress() == PGDN)
	{
		handle_event();
		trap_keypress();
	}
	else
	return 0;
return 0;
}
