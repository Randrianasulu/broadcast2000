#ifndef VRENDERTHREAD_H
#define VRENDERTHREAD_H

#include "bcbase.h"
#include "commonrenderthread.h"
#include "maxbuffers.h"
#include "mainwindow.inc"
#include "pluginbuffer.inc"
#include "vframe.inc"
#include "vrender.inc"


class VRenderThread : public CommonRenderThread
{
public:
	VRenderThread(MainWindow *mwindow, VRender *vrender);
	~VRenderThread();

	int init_rendering(int duplicate);
	int build_virtual_console(int duplicate, long current_position);
	PluginBuffer** allocate_input_buffer(int double_buffer);
	int delete_input_buffer(int buffer);
	int stop_rendering(int duplicate);

	int process_buffer(int buffer, long input_len, long input_position, long absolute_position);

	int send_last_output_buffer();

	long absolute_frame;        // absolute frame the buffer starts on
// pointers to frames in shared memory to read from disk
// (Which track*)(Array of frames*)(VFrame*)
	VFrame ***buffer_in_ptr;
// Argument for file handler
	PluginBuffer ***shared_buffer_in_ptr;  
	VRender *vrender;
};











#endif
