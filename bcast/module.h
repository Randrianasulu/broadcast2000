#ifndef MODULE_H
#define MODULE_H

#include "bcbase.h"
#include "console.inc"
#include "datatype.h"
#include "filehtal.inc"
#include "mainwindow.inc"
#include "module.inc"
#include "modules.inc"
#include "patch.inc"
#include "plugin.inc"
#include "sharedpluginlocation.inc"
#include "track.inc"

class Module : public ListItem<Module>
{
public:
	Module() { };
	Module(MainWindow *mwindow);
	virtual ~Module();
	
	virtual int create_objects(int pixel) { return 0; };
	
	virtual int save(FileHTAL *htal) { return 0; };
	virtual int load(FileHTAL *htal, int track_offset) { return 0; };

	virtual int set_pixel(int pixel) { return 0; };
	virtual int set_title(char *new_title) { return 0; };
	virtual int flip_plugins(int &x, int &y) { return 0; };
	virtual int change_format() { return 0; };    // change between DB and INT for meter
	virtual int flip_vertical(int pixel) { return 0; };
	virtual int change_x(int new_pixel) { return 0; };
	virtual int change_y(int new_pixel) { return 0; };
	virtual int dump() { return 0; };
	virtual int set_mute(int value) { return 0; };   // for setting all mutes on or off
	int render_init(int realtime_sched, int duplicate);
	int render_stop(int duplicate);

	virtual int change_channels(int new_channels, int *value_positions) { return 0; };
	int shared_plugins(ArrayList<BC_ListBoxItem*> *shared_data, ArrayList<SharedPluginLocation*> *plugin_locations);

// swap the shared module numbers
	int swap_plugins(int number1, int number2);
	int shift_module_pointers(int deleted_track);
	int toggles_selected(int on, int show, int mute);
	int select_all_toggles(int on, int show, int mute);
	int deselect_all_toggles(int on, int show, int mute);

// Queries used for direct frame rendering
// Overlay mode isn't checked.
	int console_routing_used();
	int console_adjusting_used();
	
	Patch* get_patch_of();
	Track* get_track_of();

	Plugin* plugins[PLUGINS];

	float fade;
	int mute;
	int data_type;       // TRACK_AUDIO or TRACK_VIDEO
	int module_width, module_height;          // size of module
	char title[1024];
	MainWindow *mwindow;
	Modules *modules;
	Console *console;
};

#endif
