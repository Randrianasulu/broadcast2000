#ifndef PARAMETRICWINDOW_H
#define PARAMETRICWINDOW_H

#include "bcbase.h"

class ParametricThread;
class ParametricWindow;

class EQGuiUnit;

#include "filehtal.h"
#include "mutex.h"
#include "parametric.h"

class ParametricThread : public Thread
{
public:
	ParametricThread(ParametricMain *client);
	~ParametricThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	ParametricMain *client;
	ParametricWindow *window;
};

class EQWetness;

class ParametricWindow : public BC_Window
{
public:
	ParametricWindow(ParametricMain *client);
	~ParametricWindow();
	
	int create_objects();
	int close_event();
	
	ParametricMain *client;
	EQGuiUnit *units[TOTALEQS];
	EQWetness *wetness;
};

class EQLevel;
class EQFreq;
class EQWidth;
class EQBandPass;
class EQPass;
class EQLowPass;
class EQHighPass;

class EQGuiUnit : public BC_SubWindow
{
public:
	EQGuiUnit(EQUnit *unit, int x);
	~EQGuiUnit();
	
	int create_objects();
	int update();
	
	EQUnit *unit;
	
	EQLevel *level_pot;
	EQFreq *freq_pot;
	EQWidth *width_pot;
	EQBandPass *band_toggle;
	EQPass *eq_toggle;
	EQLowPass *low_toggle;
	EQHighPass *high_toggle;
};



class EQLevel : public BC_FPot
{
public:
	EQLevel(EQUnit *unit);
	~EQLevel();
	int handle_event();
	EQUnit *unit;
};

class EQFreq : public BC_QPot
{
public:
	EQFreq(EQUnit *unit);
	~EQFreq();
	int handle_event();
	EQUnit *unit;
};

class EQWidth : public BC_IPot
{
public:
	EQWidth(EQUnit *unit);
	~EQWidth();
	int handle_event();
	EQUnit *unit;
};

class EQBandPass : public BC_Radial
{
public:
	EQBandPass(EQUnit *unit, EQGuiUnit *gui_unit);
	~EQBandPass();
	int handle_event();
	EQUnit *unit;
	EQGuiUnit *gui_unit;
};

class EQPass : public BC_Radial
{
public:
	EQPass(EQUnit *unit, EQGuiUnit *gui_unit);
	~EQPass();
	int handle_event();
	EQUnit *unit;
	EQGuiUnit *gui_unit;
};

class EQLowPass : public BC_Radial
{
public:
	EQLowPass(EQUnit *unit, EQGuiUnit *gui_unit);
	~EQLowPass();
	int handle_event();
	EQUnit *unit;
	EQGuiUnit *gui_unit;
};

class EQHighPass : public BC_Radial
{
public:
	EQHighPass(EQUnit *unit, EQGuiUnit *gui_unit);
	~EQHighPass();
	int handle_event();
	EQUnit *unit;
	EQGuiUnit *gui_unit;
};


class EQWetness : public BC_FPot
{
public:
	EQWetness(ParametricMain *client);
	~EQWetness();
	int handle_event();
	ParametricMain *client;
};

#endif
