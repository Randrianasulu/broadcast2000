#ifndef DBX_H
#define DBX_H

// the simplest plugin possible

#define RMSLEN 65536
class DBXMain;

#include "guicast.h"
#include "dbxwindow.h"
#include "pluginaclient.h"


class DBXMain : public PluginAClient
{
public:
	DBXMain(int argc, char *argv[]);
	~DBXMain();

// parameters needed
	DB db;

// required for all realtime plugins
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

	void store_rms_data(float value);

// a thread for the GUI
	DBXThread *thread;
	Defaults *defaults;
	float rms_data[RMSLEN];
	double rms_total;
	long rms_size;
	long rms_position;
	float gain;
	long window;
};


#endif
