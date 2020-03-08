#ifndef ATRACK_H
#define ATRACK_H

#include "arraylist.h"
#include "autoconf.inc"
#include "filehtal.inc"
#include "floatautos.inc"
#include "linklist.h"
#include "pluginbuffer.inc"
#include "track.h"




class ATrack : public Track
{
public:
	ATrack() { };
	ATrack(MainWindow *mwindow, Tracks *tracks);
	~ATrack();

// ====================================== initialization
	int create_derived_objs(int flash);
	int set_index_files(int flash, Asset *asset);

	int save_derived(FileHTAL *htal);
	int load_derived(FileHTAL *htal, int automation_only, int edits_only, int load_all, int &current_channel);

	int change_channels(int oldchannels, int newchannels);

// ==================================== rendering
// Offset: offset in floats from the start of the buffer.
// Input_len: length in floats of the segment to read.
	int render(PluginBuffer *shared_output, 
		long offset, long input_len, long input_position);

// ==================================== drawing
	int draw_derived(int x, int w, int y, int h, int flash = 1);
	int draw_autos_derived(float view_start, float zoom_units, AutoConf *auto_conf);
	int draw_floating_autos_derived(float view_start, float zoom_units, AutoConf *auto_conf, int flash);

// ===================================== editing
	int copy_derived(long start, long end, FileHTAL *htal);
	int paste_derived(long start, long end, long total_length, FileHTAL *htal, int &current_channel);
	int paste_output(long startproject, long endproject, long startsource, long endsource, int channel, Asset *asset);
	int clear_derived(long start, long end);

	int copy_automation_derived(AutoConf *auto_conf, long selectionstart, long selectionend, FileHTAL *htal);
	int paste_automation_derived(long selectionstart, long selectionend, long total_length, FileHTAL *htal, int shift_autos, int &current_pan);
	int clear_automation_derived(AutoConf *auto_conf, long selectionstart, long selectionend, int shift_autos = 1);
	int paste_auto_silence_derived(long start, long end);

	int select_auto_derived(float zoom_units, float view_start, AutoConf *auto_conf, int cursor_x, int cursor_y);
	int move_auto_derived(float zoom_units, float view_start, AutoConf *auto_conf, int cursor_x, int cursor_y, int shift_down);
	int release_auto_derived();
	int modify_handles(long oldposition, long newposition, int currentend);
	int scale_time_derived(float rate_scale, int scale_edits, int scale_autos, long start, long end);

	long length();
	int get_dimensions(float &view_start, float &view_units, float &zoom_units);
	long samples_to_units(long &samples);
	long units_to_samples(long &units);

// ===================================== for handles, titles, etc

	FloatAutos *pan_autos[10];
};

#endif
