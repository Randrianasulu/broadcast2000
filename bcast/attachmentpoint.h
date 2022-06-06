#ifndef ATTACHMENTPOINT_H
#define ATTACHMENTPOINT_H

#include "arraylist.h"
#include "filehtal.inc"
#include "floatauto.inc"
#include "floatautos.inc"
#include "mainwindow.inc"
#include "messages.inc"
#include "pluginserver.inc"
#include "virtualnode.inc"
#include "sharedpluginlocation.h"


class AttachmentPoint
{
public:
	AttachmentPoint(MainWindow *mwindow);
	AttachmentPoint(AttachmentPoint *that);
	virtual ~AttachmentPoint();

	int reset_parameters();

// attach a virtual plugin for realtime playback
// Returns the number of the buffer in the plugin for rendering.
	int attach_virtual_plugin(VirtualNode *virtual_plugin);

// return 0 if ready to render
// check all the virtual plugins for waiting status
// all virtual plugins attached to this must be waiting for a render
	int sort(VirtualNode *virtual_plugin);

// Move new_virtual_plugins to virtual_plugins if duplicate == 0.
	int render_init(int realtime_sched, int duplicate);

// For multichannel plugins store the fragment positions for each plugin
// and render the plugin when the last fragment position is stored
	int render(int double_buffer_in, int double_buffer_out, 
				long fragment_position_in, long fragment_position_out,
				long size, int node_number, 
				long source_position, long source_len, 
				FloatAutos *autos = 0,
				FloatAuto **start_auto = 0,
				FloatAuto **end_auto = 0,
				int reverse = 0);

	int multichannel_shared(int search_new);
	int singlechannel();

// Simply deletes the virtual plugin 
	int render_stop(int duplicate);

// Set up a plugin before attaching
	int update(int plugin_type, int in, int out, char* title, SharedPluginLocation *shared_plugin_location, SharedModuleLocation *shared_module_location);
// Plugin updates its title
	virtual int update_derived() { return 0; };
// Attach a new plugin
	int attach(int is_loading = 0);
// kill the plugin and any threads
	int detach(int update_edits = 1);
// Control the gui for the plugin.
	int show_gui();
	int hide_gui();
// Called by the plugin when it is hidden.
	int set_show(int value);
	virtual int set_show_derived(int value) { return 0; };
// Default title when nothing is attached;
	virtual const char* default_title() { return ""; };


// The track offset sets the first track number when importing a shared module or plugin.
	int load(FileHTAL *file, int track_offset, char *terminator);
	int save(FileHTAL *file, char *block_title);

	AttachmentPoint& operator=(const AttachmentPoint &that);
	int operator==(const AttachmentPoint &that);

// Get the track or module title.
	virtual char* get_module_title();

// Update the widgets after loading.
	virtual int update_display() { return 0; };
// Draw edit after attaching
// Set is_loading during a load operation so overlays aren't drawn
	virtual int update_edit(int is_loading) { return 0; };
	
	int dump();

	int get_configuration_change(char *data);

// location of plugin if shared
	SharedPluginLocation shared_plugin_location;
	SharedModuleLocation shared_module_location;

// thread client for GUI notifications on the plugin
// waits for GUI notifications when not playing
// sends commands to plugin
	PluginServer *plugin_server;  

// For unshared plugins, virtual plugins to send configuration events to.
// For shared plugins, virtual plugins to allocate buffers for.
	ArrayList<VirtualNode*> virtual_plugins;
// List for a new virtual console which is later transferred to virtual_plugins.
	ArrayList<VirtualNode*> new_virtual_plugins;
// For returning to the virtual module and determining if a new plugin must be started.
	int total_input_buffers, new_total_input_buffers;

	int in, out, show, on;
	int plugin_type;       // 0: none  1: executable   2: shared plugin   3: module
	char plugin_data[MESSAGESIZE];     // configuration data for the plugin
	const char plugin_title[1024];    // title of plugin
	MainWindow *mwindow;

// Only used by operator= and copy constructor
	int copy_from(const AttachmentPoint &that);
	int identical(const AttachmentPoint &that);
};

#endif
