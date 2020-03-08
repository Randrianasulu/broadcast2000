#ifndef CHANNELPICKER_H
#define CHANNELPICKER_H

#include "bcbase.h"
#include "channel.inc"
#include "channeledit.inc"
#include "recordengine.inc"

class ChannelButton;
class UpChannel;
class DnChannel;



class ChannelPicker
{
public:
	ChannelPicker(RecordEngine *engine, 
		ArrayList <Channel*> *channeldb, 
		BC_WindowBase *window);
	~ChannelPicker();

	int close_threads();
	int create_objects(int x, int y, int h);
	char *get_source_name(Channel *channel);  // Get the name of the source for a channel
	char *current_channel_name();    // Get the name of the current channel of the device
	int channel_up();
	int channel_down();
	int set_brightness(int value);
	int set_hue(int value);
	int set_color(int value);
	int set_contrast(int value);
	int set_whiteness(int value);

	Channel *current_channel;
	RecordEngine *engine;
	BC_WindowBase *window;
	ChannelButton *channel_button;
	UpChannel *up_channel;
	DnChannel *dn_channel;
	ArrayList <Channel*> *channeldb;
};


class ChannelButton : public BC_SmallButton
{
public:
	ChannelButton(int x, int y, ChannelPicker *channel_picker);
	~ChannelButton();
	
	int handle_event();
	ChannelPicker *channel_picker;
	ChannelEditThread *thread;
	
};

class UpChannel : public BC_UpTriangleButton
{
public:
	UpChannel(int x, int y, ChannelPicker *channel_picker);
	~UpChannel();
	int handle_event();
	int keypress_event();
	ChannelPicker *channel_picker;
};

class DnChannel : public BC_DownTriangleButton
{
public:
	DnChannel(int x, int y, ChannelPicker *channel_picker);
	~DnChannel();
	int handle_event();
	int keypress_event();
	ChannelPicker *channel_picker;
};

#endif
