#ifndef SPECTROGRAM_H
#define SPECTROGRAM_H

class Spectrogram;

#define TO_DSP 1
#define TO_GUI 2
#define START_TRIGGER 0
#define END_TRIGGER 1
#define UPDATE_GUI 3


#include "bcbase.h"
#include "../fourier/fourier.h"
#include "spectrogramwin.h"
#include "pluginaclient.h"


class Spectrogram : public PluginAClient
{
public:
	Spectrogram(int argc, char *argv[]);
	~Spectrogram();

// procedures
	int get_canvas_w();
	int get_canvas_h();
	int update_gui();         // update all the controls with new values
	int load_from_file(char *path);
	int save_to_file(char *path);
	int reset_parameters();
	int reset();
	int redo_buffers_procedure();
	int load_bitmap(VFrame *bitmap);
    int max_window_size(int fragment_size);  // Return the largest window size based on the fragment size

// data for spectrogram
	int redo_buffers;         // redo buffers before next render
	int w, h;	// Size of gui
	Messages *trigger;  // Messages to trigger GUI plugin from DSP plugin
	SharedMem *magnitude;  // Memory shared with GUI for the magnitudes.  First entry is the number of magnitudes
	int magnitudes_id;             // ID of shared memory to send
	int trigger_id;         // ID of messages to send
	SpectrogramTrigger *trigger_thread;    // Thread to handle plugin triggers
	int window_size;            // Size of the FFT window desired
	int allocated_size;         // Number of magnitudes + array size entry allocated
	double *fft_in, *freq_real, *freq_imag;
	FFT fft;
	Freq freq_table;
	DB db_table;

// required for all single channel/realtime plugins
// Triggers the GUI to refresh
	int process_realtime(long size, float **input_ptr, float **output_ptr);
	int plugin_is_realtime();
	int plugin_is_multi_channel();
	char* plugin_title();
	int start_realtime();
	int stop_realtime();
	int start_gui();
	int stop_gui();
	int cleanup_gui();
	int show_gui();
	int hide_gui();
	int set_string();
	int save_data(char *text);
	int read_data(char *text);
	SpectrogramThread *thread;          // for the GUI which is dynamically allocated only when needed
	int load_defaults();
	int save_defaults();
	Defaults *defaults;

private:
};

#endif
