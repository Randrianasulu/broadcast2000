#ifndef DENOISEWINDOW_H
#define DENOISEWINDOW_H

class DenoiseWindow;
class DenoiseThread;

#include "bcbase.h"
#include "denoise.h"
#include "mutex.h"

class DenoiseThread : public Thread
{
public:
	DenoiseThread(Denoise *plugin);
	~DenoiseThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	Denoise *plugin;
	DenoiseWindow *window;
};

class DenoiseNoiseLevel;
class DenoiseOutputLevel;
class DenoiseWindowSize;
class DenoiseLevels;
class DenoiseReset;
class DenoiseIterations;


class DenoiseWindow : public BC_Window
{
public:
	DenoiseWindow(Denoise *plugin);
	~DenoiseWindow();
	
	create_objects();
	close_event();
	update_gui();

	Denoise *plugin;
	DenoiseNoiseLevel *noiselevel;
	DenoiseOutputLevel *outputlevel;
	DenoiseWindowSize *windowsize;
	DenoiseLevels *levels;
	DenoiseIterations *iterations;
	DB db;
};

class DenoiseNoiseLevel : public BC_IPot
{
public:
	DenoiseNoiseLevel(Denoise *plugin, int x, int y);
	~DenoiseNoiseLevel();
	handle_event();
	Denoise *plugin;
};

class DenoiseOutputLevel : public BC_FPot
{
public:
	DenoiseOutputLevel(Denoise *plugin, DenoiseWindow *window, float value, int x, int y);
	~DenoiseOutputLevel();
	handle_event();
	Denoise *plugin;
	DenoiseWindow *window;
};

class DenoiseWindowSize : public BC_TextBox
{
public:
	DenoiseWindowSize(Denoise *plugin, int x, int y);
	~DenoiseWindowSize();
	handle_event();
	Denoise *plugin;
};

class DenoiseLevels : public BC_IPot
{
public:
	DenoiseLevels(Denoise *plugin, int x, int y);
	~DenoiseLevels();
	handle_event();
	Denoise *plugin;
};

class DenoiseReset : public BC_BigButton
{
public:
	DenoiseReset(Denoise *plugin, int x, int y);
	~DenoiseReset();
	handle_event();
	Denoise *plugin;
};

class DenoiseIterations : public BC_IPot
{
public:
	DenoiseIterations(Denoise *plugin, int x, int y);
	~DenoiseIterations();
	handle_event();
	Denoise *plugin;
};




#endif
