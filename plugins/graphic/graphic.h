#ifndef GRAPHIC_H
#define GRAPHIC_H

class Graphic;


#include "bcbase.h"
#include "../fourier/fourier.h"
#include "graphicwindow.h"

#define MAXPOINTS 1024
#define MAXLEVEL 15

class GraphicEngine : public CrossfadeFFT
{
public:
	GraphicEngine();
	~GraphicEngine();

	int set_plugin(Graphic *graphic);

	int signal_process();
	Graphic *graphic;
};

class Graphic : public PluginAClient
{
public:
	Graphic(int argc, char *argv[]);
	~Graphic();

// procedures

	int update_gui();         // update all the controls with new values
	int load_from_file(char *path);
	int save_to_file(char *path);
	int reset_parameters();
	int reset();
	int init_buffers();
	int redo_buffers_procedure();

// Graphic EQ commands
	double get_coefficient(long freq);
	int delete_buffers();

// data for graphic
	char config_directory[1024];
	long freqs[MAXPOINTS];     // frequency to shift
	float amounts[MAXPOINTS];
	int total_points;
	double *fft_coefs;     // List of coefficients to scale the fft window by
	int redo_buffers;

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
	int load_defaults();
	int save_defaults();

	Defaults *defaults;
	GraphicThread *thread;          // for the GUI which is dynamically allocated only when needed
	DB db;
	Freq freq;
	GraphicEngine fourier;

private:
};



#endif
