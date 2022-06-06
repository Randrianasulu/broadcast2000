#ifndef FORMATTOOLS_H
#define FORMATTOOLS_H

#include "assets.inc"
#include "bcbase.h"
#include "bitspopup.h"
#include "browsebutton.h"
#include "compresspopup.h"
#include "file.inc"
#include "formatpopup.h"

class FormatAParams;
class FormatVParams;
class FormatAThread;
class FormatVThread;
class FormatChannels;
class FormatPathButton;
class FormatPathText;
class FormatFormat;
class FormatAudio;
class FormatVideo;

class FormatTools
{
public:
	FormatTools(BC_WindowBase *window, 
				ArrayList<PluginServer*> *plugindb, 
				Asset *asset,
				int *do_audio,     // Let's not recode the dialog boxes
				int *do_video,
				int *dither);
	~FormatTools();

	int create_objects(int init_x, int init_y, 
						int do_audio,    // Include tools for audio
						int do_video,   // Include tools for video
						int prompt_audio,  // Include checkbox for audio
						int prompt_video,  // Include checkbox for video
						int prompt_audio_channels = 0,
						int prompt_video_compression = 1,
						int recording = 0); // Set to 1 to omit codecs not supported by Linux Quicktime

	int set_audio_options();
	int set_video_options();

	BC_WindowBase *window;
	Asset *asset;

	int *do_audio;
	int *do_video;
	int *dither;
	FormatAParams *aparams_button;
	FormatVParams *vparams_button;
	FormatAThread *aparams_thread;
	FormatVThread *vparams_thread;
	FormatPathButton *path_button;
	FormatPathText *path_textbox;
	FormatFormat *format_button;

	FormatChannels *channels_button;
	FormatAudio *audio_switch;
	FormatVideo *video_switch;
	ArrayList<PluginServer*> *plugindb;
	int recording;
};


class FormatPathButton : public BrowseButton
{
public:
	FormatPathButton(int x, int y, FormatTools *format, BC_TextBox *textbox, const char *text);
	~FormatPathButton();

	FormatTools *format;
};



class FormatPathText : public BC_TextBox
{
public:
	FormatPathText(int x, int y, FormatTools *format, Asset *asset);
	~FormatPathText();
	int handle_event();
	
	Asset *asset;
	FormatTools *format;
};



class FormatFormat : public FormatPopup
{
public:
	FormatFormat(int x, int y, FormatTools *format, Asset *asset, const char* default_);
	~FormatFormat();
	
	int handle_event();
	FormatTools *format;
	Asset *asset;
};

class FormatAParams : public BC_BigButton
{
public:
	FormatAParams(int x, int y, FormatTools *format);
	~FormatAParams();
	int handle_event();
	FormatTools *format;
};

class FormatVParams : public BC_BigButton
{
public:
	FormatVParams(int x, int y, FormatTools *format);
	~FormatVParams();
	int handle_event();
	FormatTools *format;
};


class FormatAThread : public Thread
{
public:
	FormatAThread(FormatTools *format);
	~FormatAThread();
	
	void run();

	FormatTools *format;
	int running;
	File *file;
};

class FormatVThread : public Thread
{
public:
	FormatVThread(FormatTools *format, int recording);
	~FormatVThread();
	
	void run();

	FormatTools *format;
	int running;
	File *file;
	int recording;
};

class FormatAudio : public BC_CheckBox
{
public:
	FormatAudio(int x, int y, FormatTools *format, int default_);
	~FormatAudio();
	int handle_event();
	FormatTools *format;
};

class FormatVideo : public BC_CheckBox
{
public:
	FormatVideo(int x, int y, FormatTools *format, int default_);
	~FormatVideo();
	int handle_event();
	FormatTools *format;
};


class FormatChannels : public BC_TextBox
{
public:
	FormatChannels(int x, int y, Asset *asset);
	~FormatChannels();
	int handle_event();

	Asset *asset;
};


#endif
