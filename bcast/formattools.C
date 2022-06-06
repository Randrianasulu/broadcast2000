#include <string.h>
#include "assets.h"
#include "bcbase.h"
#include "file.h"
#include "formattools.h"

FormatTools::FormatTools(BC_WindowBase *window, 
				ArrayList<PluginServer*> *plugindb,
				Asset *asset,
				int *do_audio,     // Let's not recode the dialog boxes
				int *do_video,
				int *dither)
{
	this->window = window;
	this->asset = asset;
	this->do_audio = do_audio;
	this->do_video = do_video;
	this->dither = dither;
	this->plugindb = plugindb;

	aparams_button = 0;
	vparams_button = 0;
	aparams_thread = 0;
	vparams_thread = 0;
}

FormatTools::~FormatTools()
{
	delete path_button;
	delete path_textbox;
	delete format_button;

	if(aparams_button) delete aparams_button;
	if(vparams_button) delete vparams_button;
	if(aparams_thread) delete aparams_thread;
	if(vparams_thread) delete vparams_thread;
}

int FormatTools::create_objects(int init_x, int init_y, 
						int do_audio,    // Include support for audio
						int do_video,   // Include support for video
						int prompt_audio,  // Include checkbox for audio
						int prompt_video,
						int prompt_audio_channels,
						int prompt_video_compression,
						int recording)
{
	int x = init_x;
	int y = init_y;
	
	this->recording = recording;

	window->add_tool(path_textbox = new FormatPathText(x, y, this, asset));
	x += 305;
	window->add_tool(path_button = new FormatPathButton(x, y, this, path_textbox, asset->path));
	x -= 305;
	y += 35;

	window->add_tool(new BC_Title(x, y, "File Format:"));
	x += 90;
	File file;
	window->add_tool(format_button = new FormatFormat(x, y, this, asset, file.formattostr(plugindb, asset->format)));

	x = init_x;
	y += 35;
	if(do_audio)
	{
		window->add_tool(new BC_Title(x, y, "Audio:", LARGEFONT, RED));
		x += 100;
		if(prompt_audio) window->add_tool(audio_switch = new FormatAudio(x, y + 5, this, *(this->do_audio)));
		x = init_x;
		y += 35;
		window->add_tool(aparams_button = new FormatAParams(x, y, this));
		y += 30;

// Audio channels only used for recording.
		if(prompt_audio_channels)
		{
			window->add_tool(new BC_Title(x, y, "Number of audio channels to record:"));
			window->add_tool(channels_button = new FormatChannels(x + 260, y, asset));
			y += 30;
		}
		
		aparams_thread = new FormatAThread(this);
	}

	if(do_video)
	{
		window->add_tool(new BC_Title(x, y, "Video:", LARGEFONT, RED));
		x += 100;
		if(prompt_video) window->add_tool(video_switch = new FormatVideo(x, y + 5, this, *(this->do_video)));
		y += 35;
		if(prompt_video_compression)
		{
			x = init_x;
			window->add_tool(vparams_button = new FormatVParams(x, y, this));
			x = 5;
			y += 50;
		}

		vparams_thread = new FormatVThread(this, recording);
	}
return 0;
}

int FormatTools::set_audio_options()
{
	if(aparams_thread->running)
	{
		// Skip
	}
	else
	{
		aparams_thread->running = 1;
		aparams_thread->start();
	}
return 0;
}

int FormatTools::set_video_options()
{
	if(vparams_thread->running)
	{
		// Skip
	}
	else
	{
		vparams_thread->running = 1;
		vparams_thread->start();
	}
return 0;
}





FormatAParams::FormatAParams(int x, int y, FormatTools *format)
 : BC_BigButton(x, y, "Options...")
{ this->format = format; }
FormatAParams::~FormatAParams() {}
int FormatAParams::handle_event() { format->set_audio_options(); return 0;
}

FormatVParams::FormatVParams(int x, int y, FormatTools *format)
 : BC_BigButton(x, y, "Options...")
{ this->format = format; }
FormatVParams::~FormatVParams() {}
int FormatVParams::handle_event() { format->set_video_options(); return 0;
}


FormatAThread::FormatAThread(FormatTools *format)
 : Thread()
{ 
	this->format = format; 
	file = new File;
	running = 0; 
}
FormatAThread::~FormatAThread() 
{
	delete file;
}
void FormatAThread::run()
{
	file->get_audio_options(format->plugindb, format->asset, format->dither);
	running = 0;
}




FormatVThread::FormatVThread(FormatTools *format, int recording)
 : Thread()
{
	this->recording = recording;
	this->format = format; 
	file = new File;
	running = 0; 
}
FormatVThread::~FormatVThread() 
{
	delete file;
}
void FormatVThread::run()
{
	file->get_video_options(format->plugindb, format->asset, recording);
	running = 0;
}

FormatPathText::FormatPathText(int x, int y, FormatTools *format, Asset *asset)
 : BC_TextBox(x, y, 300, asset->path) { this->format = format; this->asset = asset; }
FormatPathText::~FormatPathText() {}
int FormatPathText::handle_event() 
{
	strcpy(asset->path, get_text());
return 0;
}

FormatPathButton::FormatPathButton(int x, int y, FormatTools *format, BC_TextBox *textbox, const char *text)
 : BrowseButton(x, y, textbox, text, "Output to file", "Select a file to write to:", 0) 
{ this->format = format; }
FormatPathButton::~FormatPathButton() {}




FormatAudio::FormatAudio(int x, int y, FormatTools *format, int default_)
 : BC_CheckBox(x, y, 17, 17, default_, (char*)(format->recording ? "Record audio tracks" : "Render audio tracks"))
 { this->format = format; }
FormatAudio::~FormatAudio() {}
int FormatAudio::handle_event()
{
	*(format->do_audio) = get_value();
return 0;
}


FormatVideo::FormatVideo(int x, int y, FormatTools *format, int default_)
 : BC_CheckBox(x, y, 17, 17, default_, (char*)(format->recording ? "Record video tracks" : "Render video tracks"))
 { this->format = format; }
FormatVideo::~FormatVideo() {}
int FormatVideo::handle_event()
{
	*(format->do_video) = get_value();
return 0;
}




FormatFormat::FormatFormat(int x, int y, FormatTools *format, Asset *asset, const char* default_)
 : FormatPopup(format->plugindb, x, y, default_)
{ this->format = format; this->asset = asset; }
FormatFormat::~FormatFormat() {}
int FormatFormat::handle_event()
{
	File file;
	asset->format = file.strtoformat(format->plugindb, get_text());
return 0;
}



FormatChannels::FormatChannels(int x, int y, Asset *asset)
 : BC_TextBox(x, y, 100, asset->channels) { this->asset = asset; }
FormatChannels::~FormatChannels() {}
int FormatChannels::handle_event() 
{
	asset->channels = atol(get_text());
return 0;
}
