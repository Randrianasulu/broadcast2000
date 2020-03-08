#include "freeverbwin.h"
#include "pot_grey_dn_png.h"
#include "pot_grey_hi_png.h"
#include "pot_grey_up_png.h"
#include "vframe.h"

static VFrame* pot_data[] = 
{
	new VFrame(pot_grey_up_png),
	new VFrame(pot_grey_hi_png),
	new VFrame(pot_grey_dn_png),
};


FreeverbThread::FreeverbThread(Freeverb *freeverb)
 : Thread()
{
	this->freeverb = freeverb;
	set_synchronous(1); // make thread wait for join
	gui_started.lock();
}

FreeverbThread::~FreeverbThread()
{
}
	
void FreeverbThread::run()
{
	window = new FreeverbWindow(freeverb);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






FreeverbWindow::FreeverbWindow(Freeverb *freeverb)
 : BC_Window("Freeverb", 
 	INFINITY, 
	INFINITY, 
	200, 
	170, 
	200, 
	170, 
	0, 
	0,
	!freeverb->show_initially)
{ 
	this->freeverb = freeverb; 
}

FreeverbWindow::~FreeverbWindow()
{
}

int FreeverbWindow::create_objects()
{
	int x = 100, y = 10;
	add_tool(new BC_Title(5, y + 10, "Room size:"));
	add_tool(roomsize = new FreeverbRoomsize(freeverb, x, y)); y += 25;
	add_tool(new BC_Title(5, y + 10, "Damping:"));
	add_tool(damping = new FreeverbDamping(freeverb, x + 35, y)); y += 25;
	add_tool(new BC_Title(5, y + 10, "Wetness:"));
	add_tool(wetness = new FreeverbWetness(freeverb, x, y)); y += 25;
	add_tool(new BC_Title(5, y + 10, "Dryness:"));
	add_tool(dryness = new FreeverbDryness(freeverb, x + 35, y)); y += 25;
	add_tool(new BC_Title(5, y + 10, "Gain:"));
	add_tool(gain = new FreeverbGain(freeverb, x, y)); y += 25;
	return 0;
}

int FreeverbWindow::close_event()
{
	hide_window();
	freeverb->send_hide_gui();
	return 1;
}





FreeverbRoomsize::FreeverbRoomsize(Freeverb *freeverb, int x, int y)
 : BC_PercentagePot(x, y, freeverb->roomsize, 0, 100, pot_data)
{
	this->freeverb = freeverb;
}
int FreeverbRoomsize::handle_event()
{
	freeverb->roomsize = get_value();
	freeverb->send_configure_change();
	return 1;
}

FreeverbDamping::FreeverbDamping(Freeverb *freeverb, int x, int y)
 : BC_FPot(x, y, freeverb->damping, INFINITYGAIN, 0, pot_data)
{
	this->freeverb = freeverb;
}
int FreeverbDamping::handle_event()
{
	freeverb->damping = get_value();
	freeverb->send_configure_change();
	return 1;
}

FreeverbWetness::FreeverbWetness(Freeverb *freeverb, int x, int y)
 : BC_FPot(x, y, freeverb->wetness, INFINITYGAIN, 0, pot_data)
{
	this->freeverb = freeverb;
}
int FreeverbWetness::handle_event()
{
	freeverb->wetness = get_value();
	freeverb->send_configure_change();
	return 1;
}


FreeverbDryness::FreeverbDryness(Freeverb *freeverb, int x, int y)
 : BC_FPot(x, y, freeverb->dryness, INFINITYGAIN, 0, pot_data)
{
	this->freeverb = freeverb;
}
int FreeverbDryness::handle_event()
{
	freeverb->dryness = get_value();
	freeverb->send_configure_change();
	return 1;
}

FreeverbGain::FreeverbGain(Freeverb *freeverb, int x, int y)
 : BC_FPot(x, y, freeverb->gain, INFINITYGAIN, 6, pot_data)
{
	this->freeverb = freeverb;
}
int FreeverbGain::handle_event()
{
	freeverb->gain = get_value();
	freeverb->send_configure_change();
	return 1;
}
