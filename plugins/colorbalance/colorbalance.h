#ifndef COLORBALANCE_H
#define COLORBALANCE_H

class ColorBalanceMain;

#include "bcbase.h"
#include "colorbalancewindow.h"
#include "../colors/colors.h"
#include "pluginvclient.h"

#define SHADOWS 0
#define MIDTONES 1
#define HIGHLIGHTS 2

class ColorBalanceMain : public PluginVClient
{
public:
	ColorBalanceMain(int argc, char *argv[]);
	~ColorBalanceMain();

// required for all realtime plugins
	int process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr);
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
	int load_defaults();
	int save_defaults();
	int save_data(char *text);
	int read_data(char *text);

// parameters needed for processor
	int reconfigure();
    int synchronize_params(ColorBalanceSlider *slider, float difference);
    int test_boundary(float &value);
	float cyan;
	float magenta;
    float yellow;
    int preserve;
    int lock_params;

// a thread for the GUI
	ColorBalanceThread *thread;

private:
// Used by the processor
	int test_clip(int &r, int &g, int &b);
	Defaults *defaults;
    int r_lookup[VMAX + 1];
    int g_lookup[VMAX + 1];
    int b_lookup[VMAX + 1];
    double highlights_add[VMAX + 1];
    double highlights_sub[VMAX + 1];
    int redo_buffers;
    HSV hsv;
};


#endif
