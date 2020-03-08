#ifndef SYNTHMENU_H
#define SYNTHMENU_H

#include "bcbase.h"
#include "headers.h"


class SynthMenu : public BC_MenuBar
{
public:
	SynthMenu(Synth *synth, SynthWindow *window);
	~SynthMenu();
	
	int create_objects(Defaults *defaults);
	int load_defaults(Defaults *defaults);
	int save_defaults(Defaults *defaults);
// most recent loads
	int add_load(char *path);
	SynthLoadPrevThread *prev_load_thread;
	
	int total_loads;
	BC_Menu *filemenu;
	SynthWindow *window;
	Synth *synth;
	SynthLoad *load;
	SynthSave *save;
	SynthLoadPrev *prev_load[TOTAL_LOADS];
};


class SynthLoad : public BC_MenuItem
{
public:
	SynthLoad(Synth *synth, SynthMenu *menu);
	~SynthLoad();
	int handle_event();
	Synth *synth;
	SynthLoadThread *thread;
	SynthMenu *menu;
};

class SynthSave : public BC_MenuItem
{
public:
	SynthSave(Synth *synth, SynthMenu *menu);
	~SynthSave();
	int handle_event();
	Synth *synth;
	SynthSaveThread *thread;
	SynthMenu *menu;
};

class SynthLoadPrev : public BC_MenuItem
{
public:
	SynthLoadPrev(Synth *synth, SynthMenu *menu, char *filename, char *path);
	SynthLoadPrev(Synth *synth, SynthMenu *menu);
	int handle_event();
	int set_path(char *path);
	char path[1024];
	Synth *synth;
	SynthMenu *menu;
};


// ======================= level calculations
class SynthLevelZero : public BC_MenuItem
{
public:
	SynthLevelZero(Synth *synth);
	~SynthLevelZero();
	int handle_event();
	Synth *synth;
};

class SynthLevelMax : public BC_MenuItem
{
public:
	SynthLevelMax(Synth *synth);
	~SynthLevelMax();
	int handle_event();
	Synth *synth;
};

class SynthLevelNormalize : public BC_MenuItem
{
public:
	SynthLevelNormalize(Synth *synth);
	~SynthLevelNormalize();
	int handle_event();
	Synth *synth;
};

class SynthLevelSlope : public BC_MenuItem
{
public:
	SynthLevelSlope(Synth *synth);
	~SynthLevelSlope();
	int handle_event();
	Synth *synth;
};

class SynthLevelRandom : public BC_MenuItem
{
public:
	SynthLevelRandom(Synth *synth);
	~SynthLevelRandom();
	int handle_event();
	Synth *synth;
};

class SynthLevelInvert : public BC_MenuItem
{
public:
	SynthLevelInvert(Synth *synth);
	~SynthLevelInvert();
	int handle_event();
	Synth *synth;
};

class SynthLevelSine : public BC_MenuItem
{
public:
	SynthLevelSine(Synth *synth);
	~SynthLevelSine();
	int handle_event();
	Synth *synth;
};

// ============================ phase calculations

class SynthPhaseInvert : public BC_MenuItem
{
public:
	SynthPhaseInvert(Synth *synth);
	~SynthPhaseInvert();
	int handle_event();
	Synth *synth;
};

class SynthPhaseZero : public BC_MenuItem
{
public:
	SynthPhaseZero(Synth *synth);
	~SynthPhaseZero();
	int handle_event();
	Synth *synth;
};

class SynthPhaseSine : public BC_MenuItem
{
public:
	SynthPhaseSine(Synth *synth);
	~SynthPhaseSine();
	int handle_event();
	Synth *synth;
};

class SynthPhaseRandom : public BC_MenuItem
{
public:
	SynthPhaseRandom(Synth *synth);
	~SynthPhaseRandom();
	int handle_event();
	Synth *synth;
};


// ============================ freq calculations

class SynthFreqRandom : public BC_MenuItem
{
public:
	SynthFreqRandom(Synth *synth);
	~SynthFreqRandom();
	int handle_event();
	Synth *synth;
};

class SynthFreqEnum : public BC_MenuItem
{
public:
	SynthFreqEnum(Synth *synth);
	~SynthFreqEnum();
	int handle_event();
	Synth *synth;
};

class SynthFreqEven : public BC_MenuItem
{
public:
	SynthFreqEven(Synth *synth);
	~SynthFreqEven();
	int handle_event();
	Synth *synth;
};

class SynthFreqOdd : public BC_MenuItem
{
public:
	SynthFreqOdd(Synth *synth);
	~SynthFreqOdd();
	int handle_event();
	Synth *synth;
};

class SynthFreqFibonacci : public BC_MenuItem
{
public:
	SynthFreqFibonacci(Synth *synth);
	~SynthFreqFibonacci();
	int handle_event();
	Synth *synth;
};

class SynthFreqPrime : public BC_MenuItem
{
public:
	SynthFreqPrime(Synth *synth);
	~SynthFreqPrime();
	int handle_event();
	Synth *synth;
private:
	float get_next_prime(float number);
};












class SynthLoadPrevThread : public Thread
{
public:
	SynthLoadPrevThread(Synth *synth, SynthMenu *menu);
	~SynthLoadPrevThread();
	void run();
	int set_path(char *path);
	char path[1024];
	Synth *synth;
	SynthMenu *menu;
};



class SynthSaveThread : public Thread
{
public:
	SynthSaveThread(Synth *synth, SynthMenu *menu);
	~SynthSaveThread();
	void run();
	Synth *synth;
	SynthMenu *menu;
};

class SynthSaveDialog : public BC_FileBox
{
public:
	SynthSaveDialog(Synth *synth);
	~SynthSaveDialog();
	
	int ok_event();
	int cancel_event();
	Synth *synth;
};


class SynthLoadThread : public Thread
{
public:
	SynthLoadThread(Synth *synth, SynthMenu *menu);
	~SynthLoadThread();
	void run();
	Synth *synth;
	SynthMenu *menu;
};

class SynthLoadDialog : public BC_FileBox
{
public:
	SynthLoadDialog(Synth *synth);
	~SynthLoadDialog();
	
	int ok_event();
	int cancel_event();
	Synth *synth;
};









#endif
