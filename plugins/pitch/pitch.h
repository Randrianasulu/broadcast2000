#ifndef PITCH_H
#define PITCH_H

class Pitch;


#include "bcbase.h"
#include "../fourier/fourier.h"
#include "pitchwindow.h"
#include "pluginaclient.h"

#define MAXOFFSET 100
#define MINOFFSET 50

class PitchEngine : public CrossfadeFFT
{
public:
	PitchEngine();
	~PitchEngine();
	
	int set_plugin(Pitch *pitch);
//	int update_buffers();
	int signal_process();

	Pitch *pitch;
	long position;
//	double *temp_real, *temp_imag;
//	int last_window;
};


class Pitch : public PluginAClient
{
public:
	Pitch(int argc, char *argv[]);
	~Pitch();

// procedures

	int update_gui();         // update all the controls with new values
	int load_from_file(char *path);
	int save_to_file(char *path);
	int reset_parameters();
	int reset();
	int redo_buffers_procedure();


// data for pitch

	long freq_offset;     // frequency to shift
	int redo_buffers;         // redo buffers before next render

// required for all single channel/realtime plugins

	int process_realtime(long size, float *input_ptr, float *output_ptr);
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	char* plugin_title();
	int start_realtime();
	int stop_realtime();
	int start_gui();
	int stop_gui();
	int show_gui();
	int hide_gui();
	int set_string();
	int save_data(char *text);
	int read_data(char *text);
	PitchThread *thread;          // for the GUI which is dynamically allocated only when needed
	int load_defaults();
	int save_defaults();
	Defaults *defaults;
	PitchEngine engine;

private:
};

#endif
