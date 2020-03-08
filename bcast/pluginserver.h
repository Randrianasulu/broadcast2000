#ifndef PLUGINSERVER_H
#define PLUGINSERVER_H

// inherited by plugins


#include "arraylist.h"
#include "attachmentpoint.inc"
#include "floatauto.inc"
#include "floatautos.inc"
#include "mainwindow.inc"
#include "maxbuffers.h"
#include "menueffects.inc"
#include "messages.inc"
#include "mutex.h"
#include "plugin.inc"
#include "pluginbuffer.inc"
#include "pluginserver.inc"
#include "sema.inc"
#include "thread.h"
#include "track.inc"
#include "vframe.inc"

#include <stdio.h>
#include <string.h>
#include <unistd.h>




// handle realtime GUI requests in a different thread


class PluginGUIServer : public Thread
{
public:
	PluginGUIServer();
	~PluginGUIServer();

	int start_gui_server(PluginServer *plugin_server, char *string);
	void run();

	Messages *messages;       // GUI from plugin to server only messages
	PluginServer *plugin_server;
	Mutex completion_lock;
	char string[1024];      // Title given by the module
};


// use this to encapsulate the fork command and prevent zombies
class PluginForkThread : public Thread
{
public:
	PluginForkThread();
	~PluginForkThread();

	int fork_plugin(char *path, char **args);      // sets plugin_pid to the pid of the plugin
	int wait_completion();
	void run();

	char *path, **args;
	Mutex fork_lock, completion_lock;
	int plugin_pid;
};



class PluginServer
{
public:
	PluginServer();
	PluginServer(char *path);
	PluginServer(PluginServer &);
	virtual ~PluginServer();


// Get information about the plugin and close it.
	int query_plugin();
// open a plugin and wait for commands
	int open_plugin();    
// open a plugin and show the GUI
	int open_plugin(AttachmentPoint *attachment, MenuEffectPrompt *prompt, char *string);
// close the plugin
	int close_plugin();    



// queries
	int get_realtime();
	int get_multichannel();
	int get_audio();     // Query the plugin for audio support
	int get_video();     // Query the plugin for video support
	int get_fileio();     // Query the plugin for fileio support
	int get_title(char *title);




// =============================== for realtime plugins
// save configuration of plugin
	char* save_data();          
// notify plugin to load data in a separate routine so same buffer can have data written to it directly
	int notify_load_data();   
// load configuration of plugin after message buffer has been armed with data by get_message_buffer
	int load_data();     
// update configuration to reflect the master plugin
	int get_configuration_change(char *data); 
// return pointer to the actual message buffer
	char* get_message_buffer();   	
// set for realtime processor usage
	int set_realtime_sched();    

// Send GUI status to the DSP plugins
	int send_gui_status(int visible);

//	int show_gui();          // cause the plugin to show the GUI
//	int hide_gui();           // cause the plugin to hide the GUI

	int set_string(char *string);      // set the string that appears on the plugin title
	int init_realtime(int realtime_sched = 0);   // give the buffers and sizes and prepare processing realtime data
	int realtime_stop();        // deallocate buffers and stop realtime processing without closing the plugin
// process the data in the realtime buffers
	int process_realtime(long source_len, long source_position, long fragment_len);
// process data in the realtime buffers for parallelization
	int process_realtime_start(long source_len, long source_position, long fragment_len);
	int process_realtime_end();
// Send new buffer information after the virtual console is rebuilt.
	int restart_realtime();
// Send the boundary autos of the next fragment
	int set_automation(FloatAutos *autos, FloatAuto **start_auto, FloatAuto **end_auto, int reverse);
// Send the automation to the plugin
	int send_automation(long source_len, long source_position, long buffer_len);



// set the fragment position of a buffer before rendering
	int arm_buffer(int buffer_number, 
				long in_fragment_position, 
				long out_fragment_position,
				int double_buffer_in,
				int double_buffer_out);
// Attach a shared buffer from a pluginarray.
	int attach_input_buffer(PluginBuffer *input, long size);
	int attach_output_buffer(PluginBuffer *output, long size);

// Attach a shared buffer from a virtual module.
// Returns the buffer number for arming.
	int attach_input_buffer(PluginBuffer **input, long double_buffers, long buffer_size, long fragment_size);
	int attach_output_buffer(PluginBuffer **output, long double_buffers, long buffer_size, long fragment_size);
// Detach all the shared buffers.
	int detach_buffers();

	int send_buffer_info();







// ============================ for non realtime plugins
	int start_plugin();   // start processing data in plugin
	int handle_plugin_command();     // handle one plugin command and return 1 if plugin completed
	int load_defaults();             // loads defaults from disk file
	int save_defaults();
// For non realtime, prompt user for parameters, waits for plugin to finish and returns a result
	int get_parameters();
	int get_samplerate();      // get samplerate produced by plugin
	float get_framerate();     // get framerate produced by plugin
	int set_path(char *path);    // required first
	int set_mainwindow(MainWindow *mwindow);     // required before opening
	int set_interactive();             // make this the master plugin for progress bars
	int set_range(long start, long end);
	int set_track(Track *track);      // add track to the list of affected tracks for a non realtime plugin
	int set_error();         // flag to send plugin an error on next request

	int send_cancel();            // send a cancel command to plugin
	int negotiate_buffers(long recommended_size);          // get buffer size and allocate two buffers for each track
	int read_samples();  // load all output buffers with track data
	int write_samples();   // write all input buffers
	int long get_written_samples();   // after samples are written, get the number written
	int read_frames();  // load all output buffers with track data
	int write_frames();   // write all input buffers
	int long get_written_frames();   // after frames are written, get the number written
	int send_write_result(int result);  // user must send result after write

// for running plugin, wait for commands
	int plugin_server_loop();

// debugging
	int dump();

// buffers
	long out_buffer_size;   // size of a send buffer to the plugin
	long in_buffer_size;    // size of a recieve buffer from the plugin
	int total_in_buffers;
	int total_out_buffers;
// for non realtime plugin a list of channels for each buffer
	ArrayList<PluginBuffer*> data_in;    // data coming in from plugin
	ArrayList<PluginBuffer*> data_out;   // data going out to plugin

// for realtime plugin
// Some channels have 1 double buffer and some have 2 double buffers.
// Buffer sizes inform plugin of the maximum size temporary buffer to allocate.
// ArrayList of channels (which double buffer*)(PluginBuffer *)
	ArrayList<PluginBuffer**> data_in_realtime;   // data coming in from plugin
	ArrayList<PluginBuffer**> data_out_realtime;  // data going out to plugin
// number of double buffers for each channel
	ArrayList<int> double_buffers_in;    
	ArrayList<int> double_buffers_out;
// Parameters for automation.  Setting autos to 0 disables automation.
	FloatAuto **start_auto, **end_auto;
	FloatAutos *autos;
	int reverse;

// size of each buffer
	ArrayList<long> realtime_in_size;
	ArrayList<long> realtime_out_size;

// When arming buffers need to know the offsets in all the buffers and which
// double buffers for each channel before rendering.
	ArrayList<long> offset_in_render;
	ArrayList<long> offset_out_render;
	ArrayList<long> double_buffer_in_render;
	ArrayList<long> double_buffer_out_render;

// don't delete buffers if they belong to a virtual module
	int shared_buffers;
// Send new buffer information for next render
	int new_buffers;

	Messages *messages;
// seperate render events from configuration events using a sema
// using seperate message queueues was too messy
	Sema *message_lock;

	PluginGUIServer *gui_server;
	PluginForkThread *fork_thread;

	int plugin_open;                 // Whether or not the plugin is open.
	int realtime, multichannel, fileio;  // Specifies where the plugin can be used.  One of these is set.
	int audio, video;      // What data types the plugin can handle.  One of these is set.
	char *title;               // name of plugin in english
	long written_samples, written_frames;
	char *path;           // location of plugin on disk
	char *data_text;      // pointer to the data that was requested by a save_data command
	char *args[4];
	int total_args;
	int error_flag;      // send plugin an error code on next request
	ArrayList<Track*> *tracks;     // tracks affected by this plugin during a non realtime operation
	MainWindow *mwindow;                // pointer to bcast
	AttachmentPoint *attachment;
	MenuEffectPrompt *prompt;
	int gui_on;

// For I/O plugin temporary frame
	PluginBuffer *temp_frame_buffer;
	VFrame *temp_frame;

private:
	int reset_parameters();
	int cleanup_plugin();
};


#endif
