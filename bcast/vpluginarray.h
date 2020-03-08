#ifndef VPLUGINARRAY_H
#define VPLUGINARRAY_H

#include "pluginarray.h"
#include "pluginbuffer.inc"
#include "recordablevtracks.inc"
#include "track.inc"
#include "vframe.inc"

class VPluginArray : public PluginArray
{
public:
	VPluginArray(MainWindow *mwindow);
	~VPluginArray();

	long get_bufsize();
	PluginBuffer* get_buffer();
	int total_tracks();
	Track* track_number(int number);
	int write_frames_derived(long frames_written);
	int write_samples_derived(long frames_written);
	int start_plugins_derived();
	int start_realtime_plugins_derived();
	int stop_plugins_derived();
	int render_track(int track, long fragment_len, long position);

	RecordableVTracks *tracks;
// fake buffer for plugin output
	VFrame ***buffer;
// Buffer for reading from file
	PluginBuffer ***shared_buffer; 
// Buffer for threaded file (VFrame*)(VFrame array [])(Track [])
	VFrame ***video_output;
};

#endif
