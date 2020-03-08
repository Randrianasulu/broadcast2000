#ifndef COMMONRENDERTHREAD_H
#define COMMONRENDERTHREAD_H

#include "arraylist.h"
#include "commonrender.inc"
#include "mainwindow.inc"
#include "maxbuffers.h"
#include "mutex.inc"
#include "playabletracks.inc"
#include "pluginbuffer.inc"
#include "renderengine.inc"
#include "thread.h"
#include "virtualnode.inc"

class CommonRenderThread : public Thread
{
public:
	CommonRenderThread(MainWindow *mwindow, CommonRender *commonrender);
	~CommonRenderThread();

	void run();

// allocate an array of pluginbuffers for all the playable tracks
	virtual PluginBuffer** allocate_input_buffer(int double_buffer) { return 0; };
	virtual int delete_input_buffer(int buffer) { return 0; };
	virtual int init_rendering(int duplicate) { return 0; };
	int allocate_input_buffers();
	int sort_virtual_console();
	int delete_virtual_console();
	int delete_input_buffers();
	int swap_thread_buffer();
	int swap_input_buffer();

// Set duplicate when this virtual console is to share the old resources.
	int start_rendering(int duplicate);
	virtual int stop_rendering(int duplicate) { return 0; };

// for synchronizing start and stop
	int wait_for_completion();
	int wait_for_startup();

	virtual int process_buffer(int buffer, long input_len, long input_position, long absolute_position) { return 0; };
	virtual int send_last_output_buffer() { return 0; };
	ArrayList<VirtualNode*> render_list;

// exit conditions
	int interrupt, done;

	int total_buffers;      // number of sets of input buffers 
							// 1 for one at a time / MAX_BUFFERS for realtime playback
// virtual console
	int total_virtual_modules;     // the same as total_in_buffers
	VirtualNode **virtual_modules; // one top level node for each track

// playable tracks
	int total_tracks;          // number of input buffers per sets, was total_in_buffers
	int current_input_buffer;      // input buffer being read from disk
	PlayableTracks *playable_tracks;

	RenderEngine *render_engine;
	MainWindow *mwindow;
	CommonRender *commonrender;

// buffer to read from disk
// The derived thread points its proper data structures to this shared
// segment for rapid rendering through the console.
// (Array of track buffers*)(PluginBuffer*)[which double buffer]
	PluginBuffer **buffer_in[MAX_BUFFERS];
	Mutex *input_lock[MAX_BUFFERS];     // lock before sending input buffers through console
	Mutex *output_lock[MAX_BUFFERS];	// lock before loading input buffers
	Mutex *startup_lock;

// information for each buffer
	int last_playback[MAX_BUFFERS];      // last buffer in playback range
	int last_reconfigure[MAX_BUFFERS];   // last buffer before deletion and reconfiguration
	long input_len[MAX_BUFFERS];         // number of samples to render from this buffer
	long input_position[MAX_BUFFERS];    // highest numbered sample of this buffer in project
										// or frame of this buffer in project
	long absolute_position[MAX_BUFFERS];  // Absolute position at start of buffer for peaks.
	int current_thread_buffer;           // input buffer being rendered by thread
};



#endif
