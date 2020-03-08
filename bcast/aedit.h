#ifndef AEDIT_H
#define AEDIT_H


#include "bcbase.h"
#include "edit.h"
#include "filehtal.inc"
#include "aedits.inc"
#include "mainwindow.inc"
#include "pluginbuffer.inc"

// UNITS ARE SAMPLES

class AEdit : public Edit
{
public:
	AEdit(MainWindow *mwindow, AEdits *edits);
	~AEdit();

	int load_properties_derived(FileHTAL *htal);
	int set_index_file(int flash, int center_pixel, int x, int y, int w, int h);

// ========================================= drawing

	int draw(int flash, int center_pixel, int x, int w, int y, int h, int set_index_file = 0);

// ========================================= editing

	int copy_properties_derived(FileHTAL *htal, long length_in_selection);
	int get_handle_parameters(long &left, long &right, long &left_sample, long &right_sample, float view_start, float zoom_units);
	int dump_derived();
	long get_source_end();
	int render(PluginBuffer *shared_output, 
			   long offset, 
			   long input_len, 
               long input_position);
	int channel;            // channel of source in edit

private:
	int draw_source(int pixel, int center_pixel, int h, long zoomx, long zoomy, long startsource, long endsource, int vertical);
	int draw_direct(int pixel, int center_pixel, int h, long zoomx, long zoomy, long startsource, long endsource, int vertical);

	AEdits *aedits;
};

#endif
