#include "synthesizer.h"
#include "synthmenu.h"
#include "synthwindow.h"
#include <time.h>
#include <math.h>


SynthMenu::SynthMenu(Synth *synth, SynthWindow *window)
 : BC_MenuBar(0, 0, window->get_w())
{
	this->window = window;
	this->synth = synth;
}

SynthMenu::~SynthMenu()
{
	delete load;
	delete save;
	//delete set_default;
	for(int i = 0; i < total_loads; i++)
	{
		delete prev_load[i];
	}
	delete prev_load_thread;
}

int SynthMenu::create_objects(Defaults *defaults)
{
	add_menu(filemenu = new BC_Menu("File"));
	filemenu->add_menuitem(load = new SynthLoad(synth, this));
	filemenu->add_menuitem(save = new SynthSave(synth, this));
	//filemenu->add_menuitem(set_default = new SynthSetDefault);
	load_defaults(defaults);
	prev_load_thread = new SynthLoadPrevThread(synth, this);
	
	
	BC_Menu *levelmenu, *phasemenu, *harmonicmenu;
	add_menu(levelmenu = new BC_Menu("Level"));
	add_menu(phasemenu = new BC_Menu("Phase"));
	add_menu(harmonicmenu = new BC_Menu("Harmonic"));
	
	levelmenu->add_menuitem(new SynthLevelInvert(synth));
	levelmenu->add_menuitem(new SynthLevelMax(synth));
	//levelmenu->add_menuitem(new SynthLevelNormalize(synth));
	levelmenu->add_menuitem(new SynthLevelRandom(synth));
	levelmenu->add_menuitem(new SynthLevelSine(synth));
	levelmenu->add_menuitem(new SynthLevelSlope(synth));
	levelmenu->add_menuitem(new SynthLevelZero(synth));

	phasemenu->add_menuitem(new SynthPhaseInvert(synth));
	phasemenu->add_menuitem(new SynthPhaseRandom(synth));
	phasemenu->add_menuitem(new SynthPhaseSine(synth));
	phasemenu->add_menuitem(new SynthPhaseZero(synth));

	harmonicmenu->add_menuitem(new SynthFreqEnum(synth));
	harmonicmenu->add_menuitem(new SynthFreqEven(synth));
	harmonicmenu->add_menuitem(new SynthFreqFibonacci(synth));
	harmonicmenu->add_menuitem(new SynthFreqOdd(synth));
	harmonicmenu->add_menuitem(new SynthFreqPrime(synth));
	//harmonicmenu->add_menuitem(new SynthFreqRandom(synth));
return 0;
}

int SynthMenu::load_defaults(Defaults *defaults)
{
	FileSystem fs;
	total_loads = defaults->get("TOTAL_LOADS", 0);
	if(total_loads > 0)
	{
		filemenu->add_menuitem(new BC_MenuItem("-"));
		char string[1024], path[1024], filename[1024];
	
		for(int i = 0; i < total_loads; i++)
		{
			sprintf(string, "LOADPREVIOUS%d", i);
			defaults->get(string, path);
			fs.extract_name(filename, path);
//printf("SynthMenu::load_defaults %s\n", path);
			filemenu->add_menuitem(prev_load[i] = new SynthLoadPrev(synth, this, filename, path));
		}
	}
return 0;
}

int SynthMenu::save_defaults(Defaults *defaults)
{
//printf("SynthMenu::save_defaults %d\n", total_loads);
	if(total_loads > 0)
	{
		defaults->update("TOTAL_LOADS",  total_loads);
		char string[1024];
		
		for(int i = 0; i < total_loads; i++)
		{
			sprintf(string, "LOADPREVIOUS%d", i);
			defaults->update(string, prev_load[i]->path);
		}
	}
return 0;
}

int SynthMenu::add_load(char *path)
{
	if(total_loads == 0)
	{
		filemenu->add_menuitem(new BC_MenuItem("-"));
	}
	
// test for existing copy
	FileSystem fs;
	char text[1024], new_path[1024];      // get text and path
	fs.extract_name(text, path);
	strcpy(new_path, path);
	
	for(int i = 0; i < total_loads; i++)
	{
		if(!strcmp(prev_load[i]->text, text))     // already exists
		{                                // swap for top load
			for(int j = i; j > 0; j--)   // move preceeding loads down
			{
				prev_load[j]->set_text(prev_load[j - 1]->text);
				prev_load[j]->set_path(prev_load[j - 1]->path);
			}
			prev_load[0]->set_text(text);
			prev_load[0]->set_path(new_path);
			return 1;
		}
	}
	
// add another load
	if(total_loads < TOTAL_LOADS)
	{
		filemenu->add_menuitem(prev_load[total_loads] = new SynthLoadPrev(synth, this));
		total_loads++;
	}
	
// cycle loads down
	for(int i = total_loads - 1; i > 0; i--)
	{         
	// set menu item text
		prev_load[i]->set_text(prev_load[i - 1]->text);
	// set filename
		prev_load[i]->set_path(prev_load[i - 1]->path);
	}

// set up the new load
	prev_load[0]->set_text(text);
	prev_load[0]->set_path(new_path);
	return 0;
}

SynthLoad::SynthLoad(Synth *synth, SynthMenu *menu)
 : BC_MenuItem("Load...")
{
	this->synth = synth;
	this->menu = menu;
	thread = new SynthLoadThread(synth, menu);
}
SynthLoad::~SynthLoad()
{
	delete thread;
}
int SynthLoad::handle_event()
{
	thread->start();
return 0;
}

SynthSave::SynthSave(Synth *synth, SynthMenu *menu)
 : BC_MenuItem("Save...")
{
	this->synth = synth;
	this->menu = menu;
	thread = new SynthSaveThread(synth, menu);
}
SynthSave::~SynthSave()
{
	delete thread;
}
int SynthSave::handle_event()
{
	thread->start();
return 0;
}

SynthLoadPrev::SynthLoadPrev(Synth *synth, SynthMenu *menu, char *filename, char *path)
 : BC_MenuItem(filename)
{
	this->synth = synth;
	this->menu = menu;
	strcpy(this->path, path);
}
SynthLoadPrev::SynthLoadPrev(Synth *synth, SynthMenu *menu)
 : BC_MenuItem("")
{
	this->synth = synth;
	this->menu = menu;
}
int SynthLoadPrev::handle_event()
{
	menu->prev_load_thread->set_path(path);
	menu->prev_load_thread->start();
return 0;
}
int SynthLoadPrev::set_path(char *path)
{
	strcpy(this->path, path);
return 0;
}


SynthSaveThread::SynthSaveThread(Synth *synth, SynthMenu *menu)
 : Thread()
{
	this->synth = synth;
	this->menu = menu;
}
SynthSaveThread::~SynthSaveThread()
{
}
void SynthSaveThread::run()
{
	int result = 0;
	{
		SynthSaveDialog dialog(synth);
		dialog.create_objects();
		result = dialog.run_window();
		if(!result) strcpy(synth->config_directory, dialog.get_filename());
	}
	if(!result) 
	{
		result = synth->save_to_file(synth->config_directory);
		menu->add_load(synth->config_directory);
	}
}

SynthSaveDialog::SynthSaveDialog(Synth *synth)
 : BC_FileBox("", 
 			synth->config_directory, 
 			"Save synth", 
 			"Select the synth file to save as", 0, 0)
{
	this->synth = synth;
}
SynthSaveDialog::~SynthSaveDialog()
{
}
int SynthSaveDialog::ok_event()
{
	set_done(0);
return 0;
}
int SynthSaveDialog::cancel_event()
{
	set_done(1);
return 0;
}



SynthLoadThread::SynthLoadThread(Synth *synth, SynthMenu *menu)
 : Thread()
{
	this->synth = synth;
	this->menu = menu;
}
SynthLoadThread::~SynthLoadThread()
{
}
void SynthLoadThread::run()
{
	int result = 0;
	{
		SynthLoadDialog dialog(synth);
		dialog.create_objects();
		result = dialog.run_window();
		if(!result) strcpy(synth->config_directory, dialog.get_filename());
	}
	if(!result) 
	{
		result = synth->load_from_file(synth->config_directory);
		if(!result)
		{
			menu->add_load(synth->config_directory);
			synth->send_configure_change();
		}
	}
}

SynthLoadPrevThread::SynthLoadPrevThread(Synth *synth, SynthMenu *menu) : Thread()
{
	this->synth = synth;
	this->menu = menu;
}
SynthLoadPrevThread::~SynthLoadPrevThread()
{
}
void SynthLoadPrevThread::run()
{
	int result = 0;
	strcpy(synth->config_directory, path);
	result = synth->load_from_file(path);
	if(!result)
	{
		menu->add_load(path);
		synth->send_configure_change();
	}
}
int SynthLoadPrevThread::set_path(char *path)
{
	strcpy(this->path, path);
return 0;
}





SynthLoadDialog::SynthLoadDialog(Synth *synth)
 : BC_FileBox("", 
 			synth->config_directory, 
 			"Load synth", 
 			"Select the synth file to load from", 0, 0)
{
	this->synth = synth;
}
SynthLoadDialog::~SynthLoadDialog()
{
}
int SynthLoadDialog::ok_event()
{
	set_done(0);
return 0;
}
int SynthLoadDialog::cancel_event()
{
	set_done(1);
return 0;
}



// ======================= level calculations
SynthLevelZero::SynthLevelZero(Synth *synth)
 : BC_MenuItem("Zero")
{ this->synth = synth; }
SynthLevelZero::~SynthLevelZero() {}
int SynthLevelZero::handle_event()
{
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->level = INFINITYGAIN;
	}
	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthLevelMax::SynthLevelMax(Synth *synth)
 : BC_MenuItem("Maximum")
{ this->synth = synth; }
SynthLevelMax::~SynthLevelMax()
{
}

int SynthLevelMax::handle_event()
{
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->level = 0;
	}
	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthLevelNormalize::SynthLevelNormalize(Synth *synth)
 : BC_MenuItem("Normalize")
{ this->synth = synth; }
SynthLevelNormalize::~SynthLevelNormalize()
{
}

int SynthLevelNormalize::handle_event()
{
// get total power
	float total = 0;
	
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		total += synth->db.fromdb(synth->oscillators[i]->level);
	}

	float scale = 1 / total;
	float new_value;

	for(int i = 0; i < synth->total_oscillators; i++)
	{
		new_value = synth->db.fromdb(synth->oscillators[i]->level);
		new_value *= scale;
		new_value = synth->db.todb(new_value);
		
		synth->oscillators[i]->level = new_value;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthLevelSlope::SynthLevelSlope(Synth *synth)
 : BC_MenuItem("Slope")
{ this->synth = synth; }
SynthLevelSlope::~SynthLevelSlope()
{
}

int SynthLevelSlope::handle_event()
{
	float slope = (float)INFINITYGAIN / synth->total_oscillators;
	
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->level = i * slope;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthLevelRandom::SynthLevelRandom(Synth *synth)
 : BC_MenuItem("Random")
{ this->synth = synth; }
SynthLevelRandom::~SynthLevelRandom()
{
}

int SynthLevelRandom::handle_event()
{
	srand(time(0));
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->level = -(rand() % -INFINITYGAIN);
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthLevelInvert::SynthLevelInvert(Synth *synth)
 : BC_MenuItem("Invert")
{ this->synth = synth; }
SynthLevelInvert::~SynthLevelInvert()
{
}

int SynthLevelInvert::handle_event()
{
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->level = INFINITYGAIN - synth->oscillators[i]->level;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthLevelSine::SynthLevelSine(Synth *synth)
 : BC_MenuItem("Sine")
{ this->synth = synth; }
SynthLevelSine::~SynthLevelSine()
{
}

int SynthLevelSine::handle_event()
{
	float new_value;
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		new_value = (float)i / synth->total_oscillators * 2 * M_PI;
		new_value = sin(new_value) * INFINITYGAIN / 2 + INFINITYGAIN / 2;
		synth->oscillators[i]->level = new_value;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

// ============================ phase calculations

SynthPhaseInvert::SynthPhaseInvert(Synth *synth)
 : BC_MenuItem("Invert")
{ this->synth = synth; }
SynthPhaseInvert::~SynthPhaseInvert()
{
}

int SynthPhaseInvert::handle_event()
{
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->phase = 1 - synth->oscillators[i]->phase;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthPhaseZero::SynthPhaseZero(Synth *synth)
 : BC_MenuItem("Zero")
{ this->synth = synth; }
SynthPhaseZero::~SynthPhaseZero()
{
}

int SynthPhaseZero::handle_event()
{
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->phase = 0;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthPhaseSine::SynthPhaseSine(Synth *synth)
 : BC_MenuItem("Sine")
{ this->synth = synth; }
SynthPhaseSine::~SynthPhaseSine()
{
}

int SynthPhaseSine::handle_event()
{
	float new_value;
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		new_value = (float)i / synth->total_oscillators * 2 * M_PI;
		new_value = sin(new_value) / 2 + .5;
		synth->oscillators[i]->phase = new_value;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthPhaseRandom::SynthPhaseRandom(Synth *synth)
 : BC_MenuItem("Random")
{ this->synth = synth; }
SynthPhaseRandom::~SynthPhaseRandom()
{
}

int SynthPhaseRandom::handle_event()
{
	srand(time(0));
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->phase = (float)(rand() % 360) / 360;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}


// ============================ freq calculations

SynthFreqRandom::SynthFreqRandom(Synth *synth)
 : BC_MenuItem("Random")
{ this->synth = synth; }
SynthFreqRandom::~SynthFreqRandom()
{
}

int SynthFreqRandom::handle_event()
{
	srand(time(0));
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->freq_factor = rand() % 100;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthFreqEnum::SynthFreqEnum(Synth *synth)
 : BC_MenuItem("Enumerate")
{ this->synth = synth; }
SynthFreqEnum::~SynthFreqEnum()
{
}

int SynthFreqEnum::handle_event()
{
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->freq_factor = (float)i + 1;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthFreqEven::SynthFreqEven(Synth *synth)
 : BC_MenuItem("Even")
{ this->synth = synth; }
SynthFreqEven::~SynthFreqEven()
{
}

int SynthFreqEven::handle_event()
{
	if(synth->total_oscillators)
		synth->oscillators[0]->freq_factor = (float)1;

	for(int i = 1; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->freq_factor = (float)i * 2;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthFreqOdd::SynthFreqOdd(Synth *synth)
 : BC_MenuItem("Odd")
{ this->synth = synth; }
SynthFreqOdd::~SynthFreqOdd()
{
}

int SynthFreqOdd::handle_event()
{
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->freq_factor = (float)1 + i * 2;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthFreqFibonacci::SynthFreqFibonacci(Synth *synth)
 : BC_MenuItem("Fibonnacci")
{ this->synth = synth; }
SynthFreqFibonacci::~SynthFreqFibonacci()
{
}

int SynthFreqFibonacci::handle_event()
{
	float last_value1 = 0, last_value2 = 1;
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->freq_factor = last_value1 + last_value2;
		if(synth->oscillators[i]->freq_factor > 100) synth->oscillators[i]->freq_factor = 100;
		last_value1 = last_value2;
		last_value2 = synth->oscillators[i]->freq_factor;
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

SynthFreqPrime::SynthFreqPrime(Synth *synth)
 : BC_MenuItem("Prime")
{ this->synth = synth; }
SynthFreqPrime::~SynthFreqPrime()
{
}

int SynthFreqPrime::handle_event()
{
	float number = 1;
	for(int i = 0; i < synth->total_oscillators; i++)
	{
		synth->oscillators[i]->freq_factor = number;
		number = get_next_prime(number);
	}

	synth->update_gui();
	synth->send_configure_change();
return 0;
}

float SynthFreqPrime::get_next_prime(float number)
{
	int result = 1;
	
	while(result)
	{
		result = 0;
		number++;
		
		for(float i = number - 1; i > 1 && !result; i--)
		{
			if((number / i) - (int)(number / i) == 0) result = 1;
		}
	}
	
	return number;
}
