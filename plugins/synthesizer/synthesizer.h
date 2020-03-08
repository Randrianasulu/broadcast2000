#ifndef SYNTHESIZER_H
#define SYNTHESIZER_H

#define TOTALOSCILLATORS 10
#define SINE 0
#define SAWTOOTH 1
#define SQUARE 2
#define TRIANGLE 3
#define PULSE 4
#define NOISE 5

#include "bcbase.h"
#include "filehtal.h"
#include "headers.h"
#include "pluginaclient.h"


class Synth : public PluginAClient
{
public:
	Synth(int argc, char *argv[]);
	~Synth();

// data for synthesizer

	int update_gui();         // update all the controls with new values
	int load_from_file(char *path);
	int save_to_file(char *path);
	int reset_parameters();
	int create_oscillators(int total);
	int add_oscillator();
	int delete_oscillator();
	int destroy_oscillators();
	int overlay_synth(long start, long length);
	int redo_buffers_procedure();
	int solve_eqn(double *dsp_buffer, double x1, double x2);
	int oscillator_height();
	int reset();
	double get_total_power();  // get the total power of all oscillators for normalizing
	double get_point(float x, double normalize_constant);       // for drawing
	
	float *main_in, *main_out;   // convenience pointers to input_ptr and output_ptr
	double *dsp_buffer;    // buffer to be looped for waveform
	long waveform_length;           // length of loop buffer
	long waveform_sample;           // current sample in waveform of loop
	long samples_rendered;          // samples of the dsp_buffer rendered since last buffer redo
	float period;            // number of samples in a period for this frequency
	long base_freq;         // base frequency for oscillators
	int wavefunction;        // SINE, SAWTOOTH, etc
	SynthOscillator *oscillators[1024];     // array of oscillators allocated at render time
	int total_oscillators;           // number of oscillators
	int redo_buffers;         // redo buffers before next render
	char config_directory[1024];
	DB db;
	int w, h;           // dimensions of the window

// required for all single channel/realtime plugins

	int process_realtime(long size, float *input_ptr, float *output_ptr);
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	int plugin_is_audio();
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
	SynthThread *thread;          // for the GUI which is dynamically allocated

// extra non realtime support
	int load_defaults();
	int save_defaults();
	Defaults *defaults;
};

class SynthOscillator
{
public:
	SynthOscillator(Synth *synth, int number);
	~SynthOscillator();

	int load_defaults(Defaults *defaults);
	int read_data(FileHTAL *input);
	int save_defaults(Defaults *defaults);
	int save_data(FileHTAL *output);
	int create_objects(int y);
	int set_y(int position);
	int reset();
	int update_gui();
	double solve_eqn(double *output, double x1, double x2, double normalize_constant);
	double function_square(double x);
	double function_sawtooth(double x);
	double function_triangle(double x);
	double function_pulse(double x);
	double function_noise();
	double get_point(float x, double normalize_constant);       // for drawing

	float level;
	float phase;
	float freq_factor;
	int number;
	SynthOscGUI *gui;
	Synth *synth;
};

#endif
