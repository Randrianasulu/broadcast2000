#ifndef ARENDERTHREAD_H
#define ARENDERTHREAD_H


#include "arender.inc"
#include "commonrenderthread.h"
#include "mainwindow.inc"
#include "pluginbuffer.inc"

class ARenderThread : public CommonRenderThread
{
public:
	ARenderThread(MainWindow *mwindow, ARender *arender);
	~ARenderThread();

	int set_transport(int reverse, float speed);

// set up virtual console and buffers
	int init_rendering(int duplicate);
	int build_virtual_console(int duplicate, long current_position);
	PluginBuffer** allocate_input_buffer(int double_buffer);
	int delete_input_buffer(int buffer);

// delete buffers, tables, and mutexes
	int stop_rendering(int duplicate);

// process a buffer
	int process_buffer(int buffer, long input_len, long input_position, long absolute_position);
	int send_last_output_buffer();  // cause audio device to quit

// buffers read from disk
// (which track*)(float*)[which double buffer]
	float **buffer_in_ptr[MAX_BUFFERS];
	PluginBuffer **shared_buffer_in_ptr[MAX_BUFFERS];  // Argument for read_samples

	ARender *arender;
};


#endif
