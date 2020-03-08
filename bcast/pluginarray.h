#ifndef PLUGINARRAY_H
#define PLUGINARRAY_H


#include "arraylist.h"
#include "file.inc"
#include "mainwindow.inc"
#include "pluginbuffer.inc"
#include "pluginserver.inc"
#include "arraylist.h"
#include "pluginbuffer.inc"
#include "pluginserver.inc"
#include "recordableatracks.inc"
#include "track.inc"

// The plugin array does the real work of a non realtime effect.


class PluginArray : public ArrayList<PluginServer*>
{
public:
	PluginArray(MainWindow *mwindow);
	virtual ~PluginArray();

	int start_plugins(PluginServer *old_plugin);
	int start_realtime_plugins(PluginServer *old_plugin, char *plugin_data = 0);
	int set_range(long start, long end);
	int set_file(File *file);
	int set_multichannel();
	int run_plugins();
	int run_realtime_plugins(char *title);
	int stop_plugins();
	int stop_realtime_plugins();
	virtual long get_bufsize() { return 0; };
	virtual PluginBuffer* get_buffer() { return 0; };
	virtual int total_tracks() { return 0; };
	virtual Track* track_number(int number) { return 0; };
	virtual int write_samples_derived(long samples_written) { return 0; };
	virtual int write_frames_derived(long frames_written) { return 0; };
	virtual int start_plugins_derived() { return 0; };
	virtual int start_realtime_plugins_derived() { return 0; };
	virtual int stop_plugins_derived() { return 0; };
	virtual int render_track(int track, long fragment_len, long position) { return 0; };

	MainWindow *mwindow;
	File *file;             // output file
// for realtime plugins
	int realtime;
	long buffer_size;  // Should get rid of this in lieu of get_bufsize()
	PluginBuffer **realtime_buffers;        
	long realtime_start, realtime_end;
	int multichannel;
};



#endif
