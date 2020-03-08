#include "mpeg.h"
#include "mpegwindow.h"

MpegAudioThread::MpegAudioThread(MpegMain *plugin) : Thread()
{
	this->plugin = plugin;
	synchronous = 0;
	running = 0;
	window = 0;
}

MpegAudioThread::~MpegAudioThread()
{
}
	
void MpegAudioThread::run()
{
	window = new MpegAudioWindow(this);
	window->initialize();
	running = 1;
	window->run_window();
	running = 0;
	delete window;
	plugin->save_adefaults();
	plugin->send_completed();
}



MpegAudioWindow::MpegAudioWindow(MpegAudioThread *thread)
 : BC_Window("Bcast: File format", 340, 140, 340, 140, 0, 0)
{
	this->thread = thread;
}

MpegAudioWindow::~MpegAudioWindow()
{
}

int MpegAudioWindow::initialize()
{
	int x = 10, y = 10;

	add_tool(new BC_Title(x, y, "(Audio is encoded in layer 2 only)"));
	y += 20;
	add_tool(new BC_Title(x, y, "Bitrate:"));
	y += 20;
	add_tool(new BitrateText(x, y, &thread->plugin->abitrate));
	add_tool(new BC_Title(x + 200, y + 5, "kbits per second"));

	y += 35;
	x += 100;
	add_tool(new OKButton(x, y, this));
	return 0;
}





MpegVideoThread::MpegVideoThread(MpegMain *plugin) : Thread()
{
	this->plugin = plugin;
	synchronous = 0;
	running = 0;
	window = 0;
}

MpegVideoThread::~MpegVideoThread()
{
}
	
void MpegVideoThread::run()
{
	window = new MpegVideoWindow(this);
	window->initialize();
	running = 1;
	window->run_window();
	running = 0;
	delete window;
	plugin->save_vdefaults();
	plugin->send_completed();
}



MpegVideoWindow::MpegVideoWindow(MpegVideoThread *thread)
 : BC_Window("Bcast: File format", 340, 180, 340, 180, 0, 0)
{
	this->thread = thread;
}

MpegVideoWindow::~MpegVideoWindow()
{
}

int MpegVideoWindow::initialize()
{
	int x = 10, y = 10;

	add_tool(new BC_Title(x, y, "Bitrate:"));
	y += 20;
	add_tool(new BitrateText(x, y, &thread->plugin->vbitrate));
	add_tool(new BC_Title(x + 200, y, "kbits per second"));
	y += 35;
	add_tool(new Interlaced(x, y, thread->plugin));
	y += 35;
	add_tool(mpeg1 = new MPEG1(x, y, this, thread->plugin));
	add_tool(mpeg2 = new MPEG2(x + 100, y, this, thread->plugin));
	x = 100;
	y += 35;
	add_tool(new OKButton(x, y, this));
	return 0;
}

int MpegVideoWindow::set_mpeg(int value)
{
	mpeg1->update(value == 1);
	mpeg2->update(value == 2);
	return 0;
}





BitrateText::BitrateText(int x, int y, int *output)
 : BC_TextBox(x, y, 190, *output)
{
	this->output = output;
}

BitrateText::~BitrateText()
{
}

int BitrateText::handle_event()
{
	*output = atol(get_text());
	return 1;
}

Interlaced::Interlaced(int x, int y, MpegMain *plugin)
 : BC_CheckBox(x, y, 16, 16, plugin->interlaced, "Interlaced")
{
	this->plugin = plugin;
}

Interlaced::~Interlaced()
{
}

int Interlaced::handle_event()
{
	plugin->interlaced = get_value();
	return 1;
}


MPEG1::MPEG1(int x, int y, MpegVideoWindow *window, MpegMain *plugin)
 : BC_Radial(x, y, 16, 16, plugin->video_layer == 1, "MPEG - 1")
{
	this->plugin = plugin;
	this->window = window;
}

MPEG1::~MPEG1()
{
}

int MPEG1::handle_event()
{
	window->set_mpeg(1);
	return 1;
}

MPEG2::MPEG2(int x, int y, MpegVideoWindow *window, MpegMain *plugin)
 : BC_Radial(x, y, 16, 16, plugin->video_layer == 2, "MPEG - 2")
{
	this->plugin = plugin;
	this->window = window;
}

MPEG2::~MPEG2()
{
}

int MPEG2::handle_event()
{
	window->set_mpeg(2);
	return 1;
}







OKButton::OKButton(int x, int y, BC_Window *window)
 : BC_BigButton(x, y, "OK")
{
	this->window = window;
}

int OKButton::handle_event()
{
	window->set_done(0);
	return 1;
}

int OKButton::keypress_event()
{
	if(get_keypress() == 13)
	{
		window->set_done(0);
		return 1;
	}
	return 0;
}

