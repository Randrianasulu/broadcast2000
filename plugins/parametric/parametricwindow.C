#include "parametricwindow.h"


ParametricThread::ParametricThread(ParametricMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

ParametricThread::~ParametricThread()
{
}
	
void ParametricThread::run()
{
	window = new ParametricWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






ParametricWindow::ParametricWindow(ParametricMain *client)
 : BC_Window("", MEGREY, client->gui_string, 125 * TOTALEQS + 70, 85, 125 * TOTALEQS + 70, 85, 0, !client->show_initially)
{ this->client = client; }

ParametricWindow::~ParametricWindow()
{
	for(int i = 0; i < TOTALEQS; i++)
	{
		delete units[i];
	}
	delete wetness;
}

int ParametricWindow::create_objects()
{
	for(int i = 0; i < TOTALEQS; i++)
	{
		add_subwindow(units[i] = new EQGuiUnit(client->units[i], 125 * i));
	}
	add_tool(wetness = new EQWetness(client));
	add_tool(new BC_Title(380, 64, "Wetness"));
}

int ParametricWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
}






EQGuiUnit::EQGuiUnit(EQUnit *unit, int x) : BC_SubWindow(x, 0, 125, 85, MEGREY)
{
	this->unit = unit;
}

EQGuiUnit::~EQGuiUnit()
{
	delete level_pot;
	delete freq_pot;
	delete width_pot;
	delete band_toggle;
	delete low_toggle;
	delete high_toggle;
}

int EQGuiUnit::create_objects()
{
	unit->gui_unit = this;
	add_border(LTGREY, MEGREY, DKGREY);
	add_tool(level_pot = new EQLevel(unit));
	add_tool(freq_pot = new EQFreq(unit));
	add_tool(width_pot = new EQWidth(unit));
	add_tool(band_toggle = new EQBandPass(unit, this));
	add_tool(eq_toggle = new EQPass(unit, this));
	add_tool(low_toggle = new EQLowPass(unit, this));
	add_tool(high_toggle = new EQHighPass(unit, this));
	add_tool(new BC_Title(10, 49, "BP", SMALLFONT, MEYELLOW));
	add_tool(new BC_Title(30, 49, "EQ", SMALLFONT, MEYELLOW));
	add_tool(new BC_Title(55, 49, "L", SMALLFONT, MEYELLOW));
	add_tool(new BC_Title(75, 49, "H", SMALLFONT, MEYELLOW));
}

int EQGuiUnit::update()
{
	
	level_pot->update(unit->level);
	freq_pot->update(unit->frequency);
	width_pot->update((int)unit->quality);
	band_toggle->update(unit->bandpass);
	eq_toggle->update(unit->eqpass);
	low_toggle->update(unit->lowpass);
	high_toggle->update(unit->highpass);
}


EQLevel::EQLevel(EQUnit *unit)
 : BC_FPot(10, 5, 35, 35, unit->level, -15, 15, PINK, RED)
{
	this->unit = unit;
}
EQLevel::~EQLevel() {}
int EQLevel::handle_event()
{
	unit->level = get_value();
	unit->client->send_configure_change();
}

EQFreq::EQFreq(EQUnit *unit)
 : BC_QPot(45, 10, 35, 35, unit->frequency, 0, 20000, DKGREY, BLACK)
{
	this->unit = unit;
}
EQFreq::~EQFreq() {}
int EQFreq::handle_event()
{
	unit->frequency = get_value();
	unit->client->send_configure_change();
}

EQWidth::EQWidth(EQUnit *unit)
 : BC_IPot(80, 5, 35, 35, (int)unit->quality, 0, 100, LTGREY, MEGREY)
{
	this->unit = unit;
}
EQWidth::~EQWidth() {}
int EQWidth::handle_event()
{
	unit->quality = get_value();
	unit->client->send_configure_change();
}

EQBandPass::EQBandPass(EQUnit *unit, EQGuiUnit *gui_unit)
 : BC_Radial(10, 64, 17, 17, unit->bandpass)
{
	this->unit = unit;
	this->gui_unit = gui_unit;
}
EQBandPass::~EQBandPass() {}
int EQBandPass::handle_event()
{
	unit->bandpass = get_value();
	if(down)
	{
		unit->eqpass = unit->lowpass = unit->highpass = 0;
		gui_unit->eq_toggle->update(0);
		gui_unit->low_toggle->update(0);
		gui_unit->high_toggle->update(0);
	}
	unit->client->send_configure_change();
}


EQPass::EQPass(EQUnit *unit, EQGuiUnit *gui_unit)
 : BC_Radial(30, 64, 17, 17, unit->eqpass)
{
	this->unit = unit;
	this->gui_unit = gui_unit;
}
EQPass::~EQPass() {}
int EQPass::handle_event()
{
	unit->eqpass = get_value();
	if(down)
	{
		unit->bandpass = unit->lowpass = unit->highpass = 0;
		gui_unit->band_toggle->update(0);
		gui_unit->low_toggle->update(0);
		gui_unit->high_toggle->update(0);
	}
	unit->client->send_configure_change();
}

EQLowPass::EQLowPass(EQUnit *unit, EQGuiUnit *gui_unit)
 : BC_Radial(50, 64, 17, 17, unit->lowpass)
{
	this->unit = unit;
	this->gui_unit = gui_unit;
}
EQLowPass::~EQLowPass() {}
int EQLowPass::handle_event()
{
	unit->lowpass = get_value();
	if(down)
	{
		unit->eqpass = unit->bandpass = unit->highpass = 0;
		gui_unit->eq_toggle->update(0);
		gui_unit->band_toggle->update(0);
		gui_unit->high_toggle->update(0);
	}
	unit->client->send_configure_change();
}

EQHighPass::EQHighPass(EQUnit *unit, EQGuiUnit *gui_unit)
 : BC_Radial(70, 64, 17, 17, unit->highpass)
{
	this->unit = unit;
	this->gui_unit = gui_unit;
}
EQHighPass::~EQHighPass() {}
int EQHighPass::handle_event()
{
	unit->highpass = get_value();
	if(down)
	{
		unit->eqpass = unit->bandpass = unit->lowpass = 0;
		gui_unit->eq_toggle->update(0);
		gui_unit->band_toggle->update(0);
		gui_unit->low_toggle->update(0);
	}
	unit->client->send_configure_change();
}

EQWetness::EQWetness(ParametricMain *client)
 : BC_FPot(395, 15, 35, 35, client->wetness, INFINITYGAIN, 0, PINK, RED)
{
	this->client = client;
}
EQWetness::~EQWetness() {}
int EQWetness::handle_event()
{
	client->wetness = get_value();
	client->send_configure_change();
}
