#ifndef PLUGINCLIENT_H
#define PLUGINCLIENT_H

// Base class inherited by all the different types of plugins.

#define BCASTDIR "~/.bcast/"

class PluginClient;

#include "arraylist.h"
#include "maxbuffers.h"
#include "messages.h"
#include "pluginbuffer.h"
#include "plugincommands.h"
#include "sema.h"

extern "C"
{
extern PluginClient* new_plugin();
}

class PluginClientAuto
{
public:
	float position;
	float intercept;
	float slope;
};

class PluginClient
{
public:
	PluginClient(int argc, char *argv[]);
	virtual ~PluginClient();

	
	virtual int is_realtime();
	virtual int is_audio();
	virtual int is_video();
	virtual int is_fileio();
	virtual int is_multichannel();
	virtual char* plugin_title();   // return the title of the plugin














// Queries for the plugin server.
// All plugins must provide these.
	virtual int plugin_is_realtime();           // return 1 if this plugin processes data in realtime from the console
	virtual int plugin_is_multi_channel();      // return 1 if this plugin can process data for several tracks at a time
	virtual int plugin_is_fileio();      		// return 1 if this plugin can access a file

// Signal processors can only process one of audio or video.
// File handlers can process audio and video.
	virtual int plugin_is_audio();              // plugin supports audio/default = 0
	virtual int plugin_is_video();              // plugin supports video/default = 0




// Non realtime signal processors define these.
	virtual int get_plugin_samplerate();        // give the samplerate of the output for a non realtime plugin
	virtual float get_plugin_framerate();         // give the framerate of the output for a non realtime plugin
	virtual int init_nonrealtime_parameters();   // Set up pointers for plugin
	virtual int delete_nonrealtime_parameters();
	virtual int start_plugin();         // run a non realtime plugin
	virtual int get_parameters();     // get information from user before non realtime processing
	virtual long get_in_buffers(long recommended_size);  // return desired size for input buffers
	virtual long get_out_buffers(long recommended_size);     // return desired size for output buffers






// Realtime commands for signal processors.
// These must be defined by the plugin itself.
	virtual int set_string();             // Set the string identifying the plugin to modules and patches.
	virtual int start_gui();              // thread out the plugin's GUI
	virtual int stop_gui();               // stop the GUI thread before closing
	virtual int show_gui();               // cause the plugin to show the gui
	virtual int hide_gui();               // cause the plugin to hide the gui
	virtual int start_realtime();         // start the plugin waiting to process realtime data
	virtual int stop_realtime();          // start the plugin waiting to process realtime data
	virtual int save_data(char *text);    // write the plugin settings to text in text format
	virtual int read_data(char *text);    // read the plugin settings from the text
	int send_hide_gui();                                    // should be sent when the GUI recieves a close event from the user
	int send_configure_change();                            // when this plugin is adjusted, propogate parameters to virtual plugins
	int get_configure_change();                             // get propogated configuration change from a send_configure_change
	int plugin_restart_realtime();        // Get new buffers after a rebuilt virtual console
	virtual int process_realtime(long size);  // Process buffers in the client base class.
	virtual int init_realtime_parameters();     // get parameters depending on video or audio
	int get_gui_status();
	int buffers_identical(int channel);        // If the input and output buffer is identical
// Realtime parameters for transient plugins which are called during process_realtime.
	long get_source_len();
	long get_source_position();
// If automation is used
	int automation_used();
// Get the automation value for the position in the current fragment
// The position is relative to the current fragment
	float get_automation_value(long position);




// Operations for file handlers
	virtual int open_file() { return 0; };
	virtual int get_audio_parameters() { return 0; };
	virtual int get_video_parameters() { return 0; };
	virtual int check_header(char *path) { return 0; };
	virtual int open_file(char *path, int wr, int rd) { return 1; };
	virtual int close_file() { return 0; };





// All plugins define these.
	virtual int load_defaults();       // load default settings for the plugin
	virtual int save_defaults();      // save the current settings as defaults




// Non realtime operations for signal processors.
	long get_project_samplerate();            // get samplerate of project data before processing
	float get_project_framerate();         // get framerate of project data before processing
	int get_gui_visible();     // Get the GUI status from the server before processing
	int get_project_framesize(int &frame_w, int &frame_h); // get frame size of project data before processing
	int get_use_float();
	int get_use_alpha();
	int get_use_interpolation();
	int get_project_smp();
	int get_aspect_ratio(float &aspect_w, float &aspect_h);
	int read_frames(long start_position, long total_frames); // returns 1 for failure / waits for input buffers to be filled from start_position in the project
	int write_frames(long total_frames);  // returns 1 for failure / tells the server that all output channel buffers are ready to go
	int read_samples(long start_position, long total_samples); // returns 1 for failure / waits for input buffers to be filled from start_position in the project
	int write_samples(long total_samples);  // returns 1 for failure / tells the server that all output channel buffers are ready to go
	int plugin_get_parameters();
	char* get_defaultdir();     // Directory defaults should be stored in


// Realtime operations.
	int plugin_init(int argc, char *argv[]);
	int plugin_exit();
	int plugin_cleanup();
	int plugin_run();
	virtual int plugin_command_derived(int plugin_command) {}; // Extension of plugin_run for derived plugins
	int plugin_get_range();
	int plugin_negotiate_buffers();
	int plugin_start_plugin();    // Run a non realtime plugin
	int plugin_init_realtime();
	int plugin_stop_realtime();
	int plugin_process_realtime();
	int plugin_create_buffers();
	int plugin_delete_buffers();



// create pointers to buffers of the plugin's type before realtime rendering
	virtual int create_buffer_ptrs();
	virtual int delete_buffer_ptrs();




// communication convenience routines for the base class
	int stop_gui_client();     
	int save_data_client();
	int load_data_client();
	int set_string_client();                // set the string identifying the plugin
	int send_completed();        // send when finished
	int send_cancelled();        // non realtime plugin sends when cancelled

// ================================= Buffers ===============================
// non realtime buffers
	PluginBuffer **data_in;
	PluginBuffer **data_out;
// realtime buffers
	ArrayList<PluginBuffer**> data_in_realtime;   // data coming in from plugin
	ArrayList<PluginBuffer**> data_out_realtime;  // data going out to plugin

// number of double buffers for each channel
	ArrayList<int> double_buffers_in;    
	ArrayList<int> double_buffers_out;
// When arming buffers need to know the offsets in all the buffers and which
// double buffers for each channel before rendering.
	ArrayList<long> offset_in_render;
	ArrayList<long> offset_out_render;
	ArrayList<long> double_buffer_in_render;
	ArrayList<long> double_buffer_out_render;
// total size of each buffer depends on if it's a master or node
	ArrayList<long> realtime_in_size;
	ArrayList<long> realtime_out_size;

// ================================= Automation ===========================

	ArrayList<PluginClientAuto> automation;

// ================================== Messages ===========================
	Messages *messages, *gui_messages;
	Sema *message_lock;             // only used for realtime plugins
	char gui_string[1024];          // string identifying module and plugin
	int master_gui_on;              // Status of the master gui plugin
	int client_gui_on;              // Status of this client's gui

	int show_initially;             // set to show a realtime plugin initially
	long start, end;                // range in project for processing
	int interactive;                // for the progress bar plugin
	int success;
	int total_out_buffers;          // total send buffers allocated by the server
	int total_in_buffers;           // total recieve buffers allocated by the server
	int wr, rd;                     // File permissions for fileio plugins.

// these give the largest fragment the plugin is expected to handle
// currently these give the master size
	long out_buffer_size;  // size of a send buffer to the server
	long in_buffer_size;   // size of a recieve buffer from the server

// Realtime parameters for transient plugins.
	long source_len, source_position;
	int smp;  // Total number of processors available

private:

// File handlers:
//	Asset *asset;     // Point to asset structure in shared memory
};


#endif
