#ifndef MODULES_H
#define MODULES_H

#include "bcbase.h"
#include "console.inc"
#include "filehtal.inc"         // for load_console
#include "mainwindow.inc"
#include "module.h"           // for module list item
#include "sharedpluginlocation.inc"       // for plugin location


class Modules : public List<Module>
{
public:
	Modules(MainWindow *mwindow, Console *console);
	virtual ~Modules();

	int load_console(FileHTAL *htal, Module* module, int track_offset);        // load for undo
	int add_audio_track();
	int add_video_track();

	int flip_vertical(int &pixel, int pixel_start);

	int delete_track();
	int delete_track(Module *module);
	int delete_all();
	int pixelmovement(int distance);
	int reset_pixels();        // reset all the module positions after a move or delete
	int redo_pixels(int &pixel);
	int change_channels(int old_channels, int new_channels);
	int init_meters(int total_peaks);      // set up meters for playback
	int stop_meters();
	int reset_meters();
	int arender_init(int realtime_sched, int duplicate);      // set up buffers and plugin servers for playback
	int arender_stop(int duplicate);
	int vrender_init(int duplicate);
	int vrender_stop(int duplicate);
	int toggles_selected(int on, int show, int mute);
	int select_all_toggles(int on, int show, int mute);
	int deselect_all_toggles(int on, int show, int mute);

	int total_pixels();
	int change_format();
	int update_meters(int peak_number, int last_peak, int total_peaks);

	int shared_aplugins(ArrayList<BC_ListBoxItem*> *shared_data, ArrayList<SharedPluginLocation*> *plugin_locations, int exclude_module);
	int shared_amodules(ArrayList<BC_ListBoxItem*> *module_data, ArrayList<SharedModuleLocation*> *module_locations, int exclude_module);
	int playable_amodules(ArrayList<Module *> *playable_modules);
	int shared_vplugins(ArrayList<BC_ListBoxItem*> *shared_data, ArrayList<SharedPluginLocation*> *plugin_locations, int exclude_module);
	int shared_vmodules(ArrayList<BC_ListBoxItem*> *module_data, ArrayList<SharedModuleLocation*> *module_locations, int exclude_module);
	int swap_plugins(Module* module1, Module* module2);

	int number_of(Module *module);        // module number of pointer
	int number_of_audio(Module *module);        // number of audio modules before pointer for pans
	Module* module_number(int number);       // pointer to module number
	BC_TextBox* module_title_number(int number);
	MainWindow *mwindow;
	Console *console;
};

#endif
