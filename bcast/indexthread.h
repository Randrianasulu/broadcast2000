#ifndef INDEXTHREAD_H
#define INDEXTHREAD_H

#include "assets.inc"
#include "indexfile.inc"
#include "mainwindow.inc"
#include "mutex.h"
#include "pluginbuffer.h"
#include "thread.h"

#define TOTAL_BUFFERS 2

// Recieves buffers from Indexfile and calculates the index.

class IndexThread : public Thread
{
public:
	IndexThread(MainWindow *mwindow, 
		Asset *asset,
		char *index_filename,
		long buffer_size, 
		long length_source);
	~IndexThread();

	friend IndexFile;

	int start_build();
	int stop_build();
	void run();

	MainWindow *mwindow;
	Asset *asset;
	char *index_filename;
	long buffer_size, length_source;
	int current_buffer;

private:
	int redraw_edits(int flash);

	int interrupt_flag;
	PluginBuffer **shared_buffer_in[TOTAL_BUFFERS];
	float **buffer_in[TOTAL_BUFFERS];
	Mutex input_lock[TOTAL_BUFFERS], output_lock[TOTAL_BUFFERS];
	int last_buffer[TOTAL_BUFFERS];
	long input_len[TOTAL_BUFFERS];
};



#endif
