#ifndef APLUGINARRAY_H
#define APLUGINARRAY_H

#include "pluginarray.h"
#include "pluginbuffer.inc"
#include "recordableatracks.inc"
#include "track.inc"

class APluginArray : public PluginArray
{
public:
	APluginArray(MainWindow *mwindow);
	~APluginArray();

	long get_bufsize();
	PluginBuffer* get_buffer();
	int total_tracks();
	Track* track_number(int number);
	int write_samples_derived(long samples_written);
	int start_plugins_derived();
	int start_realtime_plugins_derived();
	int stop_plugins_derived();
	int render_track(int track, long fragment_len, long position);

	RecordableATracks *tracks;
// Pointers to plugin buffers for plugin output
	float **buffer;         // Buffer for processing
	PluginBuffer **shared_buffer; // Buffer for reading from file
// Pointer to file output
	float **output_buffer;
};

#endif
