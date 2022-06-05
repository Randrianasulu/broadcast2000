#ifndef PARAMETRIC_H
#define PARAMETRIC_H

class ParametricMain;
class EQUnit;
#define TOTALEQS 3
#define SKIRT_LEN 6

#include "bcbase.h"
#include "parametricwindow.h"
#include "pluginaclient.h"


class ParametricMain : public PluginAClient
{
public:
	ParametricMain(int argc, char *argv[]);
	~ParametricMain();

// perform a bp filter on dsp_in
	int process_bp(double &in_sample4, double &in_sample3, double &in_sample2, double *dsp_in, long size, double band_width, double center, double scale);
	int process_bp_soundtools(double &in_sample3, double &in_sample2, double *dsp_in, long size, double band_width, int center, double scale);
	int process_low(double &in_sample2, double &in_sample3, double *dsp_in, long size, int center, double scale);
	int process_high(double &in_sample2, double &in_sample3, double *dsp_in, long size, int center, double scale);
	float get_bp_level(float level);
	float get_eq_level(float level);
	float get_bandreject_level(float level);

// parameters needed
	int process_realtime(long size, float *input_ptr, float *output_ptr);

	double *dsp_buffer, *dsp_in, *dsp_out;
	float wetness;
	EQUnit *units[TOTALEQS];
	double eq_skirt[TOTALEQS][SKIRT_LEN];
	double bp_skirt[TOTALEQS][SKIRT_LEN];
	ParametricThread *thread;
	DB db;


// required for all realtime/single channel plugins

	int plugin_is_realtime();
	int plugin_is_multi_channel();
	const char* plugin_title();
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
};

class EQUnit 
{
public:
	EQUnit(ParametricMain *client);
	~EQUnit();
	
	int save(FileHTAL *htal);
	int load(FileHTAL *htal);
	int load_defaults(Defaults *defaults, int number);
	int save_defaults(Defaults *defaults, int number);

	float level;    // linear converted level value
	int frequency;
	float quality;
	int bandpass, eqpass, lowpass, highpass;
	EQGuiUnit *gui_unit;
	ParametricMain *client;
};


#endif
