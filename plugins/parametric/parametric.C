#include <math.h>


#include "filehtal.h"
#include "parametric.h"
#include "parametricwindow.h"

main(int argc, char *argv[])
{
	ParametricMain *plugin;
	
	plugin = new ParametricMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

ParametricMain::ParametricMain(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	for(int i = 0; i < TOTALEQS; i++)
	{
		units[i] = new EQUnit(this);
	}
	wetness = INFINITYGAIN;
}

ParametricMain::~ParametricMain()
{
	for(int i = 0; i < TOTALEQS; i++)
	{
		delete units[i];
	}
}

char* ParametricMain::plugin_title() { return "EQ Parametric"; }
int ParametricMain::plugin_is_realtime() { return 1; }
int ParametricMain::plugin_is_multi_channel() { return 0; }
	
int ParametricMain::start_realtime()
{
	dsp_buffer = new double[in_buffer_size];
	dsp_in = new double[in_buffer_size];
	dsp_out = new double[out_buffer_size];

// reset skirt
	for(int i = 0; i < TOTALEQS; i++)
	{
		for(int j = 0; j < SKIRT_LEN; j++)
		{
			eq_skirt[i][j] = 0;
			bp_skirt[i][j] = 0;
		}
	}
}

int ParametricMain::stop_realtime()
{
	delete dsp_buffer;
	delete dsp_in;
	delete dsp_out;
}

int ParametricMain::process_realtime(long size, float *input_ptr, float *output_ptr)
{
	int eqpass = 0, bandpass = 0, bandreject = 0, i, in_output = 0;
	long buffer_end = size;

	for(i = 0; i < TOTALEQS; i++)
	{
		if(units[i]->eqpass && units[i]->level > 0) eqpass = 1;
		else
		if(units[i]->eqpass && units[i]->level < 0) bandreject = 1;
		else
		if(units[i]->bandpass || units[i]->lowpass || units[i]->highpass) bandpass = 1;
	}

	if(!eqpass && !bandpass && !bandreject)
	{
// No EQ is desired.  Copy to output buffer if different.
		if(output_ptr != input_ptr)
			for(int j = 0; j < size; j++) output_ptr[j] = input_ptr[j];
	}
	else
	{
		if(eqpass)
		{
// process eq passes
			int count = 0;
			
			for(i = 0; i < TOTALEQS; i++)
			{
				if(units[i]->eqpass && units[i]->level > 0)
				{
					float level = get_eq_level(units[i]->level);

// copy input into a buffer for mangling
					for(register int j = 0; j < size; j++) dsp_in[j] = input_ptr[j];
// Perform bandpass
					process_bp(eq_skirt[i][0], eq_skirt[i][1], eq_skirt[i][2], dsp_in, size, units[i]->quality, units[i]->frequency, level);

// overlay on dsp_out
					if(count == 0)
						for(register int j = 0; j < size; j++) dsp_out[j] = dsp_in[j] * level;
					else
						for(register int j = 0; j < size; j++) dsp_out[j] += dsp_in[j] * level;

					count++;
				}
			}

// overlay the source signal
			for(register int j = 0; j < size; j++) dsp_out[j] += input_ptr[j];
			in_output = 1;
		}

		if(bandreject)
		{
// Process band rejections
			int count = 0;
			int band_width;

			if(in_output)
				for(int j = 0; j < size; j++) dsp_buffer[j] = dsp_out[j];
			else
				for(int j = 0; j < size; j++) dsp_buffer[j] = input_ptr[j];

			in_output = 0;
			for(i = 0; i < TOTALEQS; i++)
			{
				if(units[i]->eqpass && units[i]->level < 0)
				{
					float level = get_bandreject_level(units[i]->level);
					band_width = (int)(units[i]->quality / 100 * units[i]->frequency);

// Get input from previous band reject
					if(in_output)
						for(int j = 0; j < size; j++) dsp_buffer[j] = dsp_out[j];

// Copy input into buffer for mangling
					for(int j = 0; j < size; j++) dsp_in[j] = dsp_buffer[j];
// Perform low pass
					process_low(bp_skirt[i][0], 
						bp_skirt[i][1], 
						dsp_in, 
						size, 
						(units[i]->frequency - band_width > 0) ? units[i]->frequency - band_width : 0, 
						1 - level);
// Copy to output
					for(int j = 0; j < size; j++) dsp_out[j] = dsp_in[j];

// Copy input into buffer for mangling
					for(int j = 0; j < size; j++) dsp_in[j] = dsp_buffer[j];
// Perform high pass
					process_high(bp_skirt[i][2], 
						bp_skirt[i][3], 
						dsp_in, 
						size, 
						(units[i]->frequency + band_width < project_sample_rate / 2) ? units[i]->frequency + band_width : project_sample_rate / 2, 
						1 - level);
// Overlay on output
					for(int j = 0; j < size; j++) dsp_out[j] += dsp_in[j];

// Overlay input on output
					if(level)
						for(int j = 0; j < size; j++) dsp_out[j] += dsp_buffer[j] * level;

					in_output = 1;
				}
			}
		}

		if(bandpass)
		{
// get input from any previous eq
			if(in_output)
				for(register int j = 0; j < size; j++) dsp_buffer[j] = dsp_out[j];
			else
				for(register int j = 0; j < size; j++) dsp_buffer[j] = input_ptr[j];

// process band passes
			int count = 0;
			for(i = 0; i < TOTALEQS; i++)
			{
				float level = get_bp_level(units[i]->level);
			
				if(units[i]->bandpass)
				{
// get input into a buffer for mangling
					for(register int j = 0; j < size; j++) dsp_in[j] = dsp_buffer[j];

// Process bandpass
					process_bp(bp_skirt[i][0], bp_skirt[i][1], bp_skirt[i][2], dsp_in, size, units[i]->quality, units[i]->frequency, level);

// overlay on output
					if(count == 0)
						for(register int j = 0; j < size; j++) dsp_out[j] = dsp_in[j] * level;
					else
						for(register int j = 0; j < size; j++) dsp_out[j] += dsp_in[j] * level;
					
					count++;
				}
				else
				if(units[i]->lowpass)
				{
// get input into a buffer for mangling
					for(register int j = 0; j < size; j++) dsp_in[j] = dsp_buffer[j];

// Process lowpass
					process_low(bp_skirt[i][0], bp_skirt[i][1], dsp_in, size, units[i]->frequency, level);

// overlay on output
					if(count == 0)
						for(register int j = 0; j < size; j++) dsp_out[j] = dsp_in[j];
					else
						for(register int j = 0; j < size; j++) dsp_out[j] += dsp_in[j];
					
					count++;
				}
				else
				if(units[i]->highpass)
				{
// get input into a buffer for mangling
					for(register int j = 0; j < size; j++) dsp_in[j] = dsp_buffer[j];

// Process highpass
					process_high(bp_skirt[i][0], bp_skirt[i][1], dsp_in, size, units[i]->frequency, level);

// overlay on output
					if(count == 0)
						for(register int j = 0; j < size; j++) dsp_out[j] = dsp_in[j];
					else
						for(register int j = 0; j < size; j++) dsp_out[j] += dsp_in[j];

					count++;
				}
			}
		}

// final write to 32 bit out
		if(!bandpass)
			for(register int j = 0; j < size; j++) output_ptr[j] = dsp_out[j];
		else
			for(register int j = 0; j < size; j++)
			{
// factor in wetness for a bandpass
				float level = db.fromdb(wetness);
				output_ptr[j] = input_ptr[j] * level;
				output_ptr[j] += dsp_out[j];
			}
	}
}

int ParametricMain::process_bp_soundtools(double &in_sample3, double &in_sample2, double *dsp_in, long size, double band_width, int center, double scale)
{
	band_width /= 200;
	band_width *= project_sample_rate / 2;
	double *in_sample1 = dsp_in;
	double *end_input = dsp_in + size;
	double variable1, variable2, variable3;

	double c = exp(-2 * M_PI * (double)band_width / project_sample_rate);
	double b = -4 * c / (1 + c) * cos(2 * M_PI * center / project_sample_rate);
	double a = scale * sqrt(1 - b * b / (4 * c)) * (1 - c);

// process the data
	while(in_sample1 < end_input)
	{
		variable1 = a * *in_sample1;
		variable2 = b * in_sample2;
		variable3 = c * in_sample3;
		
		*in_sample1 = variable1 - variable2 - variable3;

		in_sample3 = in_sample2;
		in_sample2 = *in_sample1;
		in_sample1++;
	}
}

int ParametricMain::process_bp(double &in_sample4, double &in_sample3, double &in_sample2, double *dsp_in, long size, double band_width, double center, double scale)
{
	band_width /= 200;
	band_width *= center;
	double *in_sample1 = dsp_in;
	double a = 0.25 * 2 * M_PI * center / project_sample_rate;
	double b = 0.25 * 4 * M_PI * band_width / project_sample_rate;
	double c = b * sqrt(a * a - 0.0625 * b * b);


	a = -a * a;
	double *end_input = dsp_in + size;

	while(in_sample1 < end_input)
	{
		in_sample4 += a * in_sample3 - b * in_sample4 + 0.75 * in_sample2 + 0.25 * *in_sample1; in_sample3 += in_sample4;
		in_sample4 += a * in_sample3 - b * in_sample4 + 0.50 * in_sample2 + 0.50 * *in_sample1; in_sample3 += in_sample4;
		in_sample4 += a * in_sample3 - b * in_sample4 + 0.25 * in_sample2 + 0.75 * *in_sample1; in_sample3 += in_sample4;
		in_sample4 += a * in_sample3 - b * in_sample4 + *in_sample1; in_sample3 += in_sample4;
		in_sample2 = *in_sample1;
		*in_sample1++ = c * in_sample3;
	}
}

int ParametricMain::process_low(double &in_sample2, double &in_sample3, double *dsp_in, long size, int center, double scale)
{
	double *in_sample1 = dsp_in;
	double coef = 0.25 * 2.0 * M_PI * (double)center / (double)project_sample_rate;
	double a = coef * 0.25;
	double b = coef * 0.50;

	double *end_input = dsp_in + size;

	while(in_sample1 < end_input)
	{
		in_sample3 += a * (3 * in_sample2 + *in_sample1 - in_sample3);
		in_sample3 += b * (in_sample2 + *in_sample1 - in_sample3);
		in_sample3 += a * (in_sample2 + 3 * *in_sample1 - in_sample3);
		in_sample3 += coef * (*in_sample1 - in_sample3);
		in_sample2 = *in_sample1;
		*in_sample1++ = in_sample3 * scale;
	}
}

int ParametricMain::process_high(double &in_sample2, double &in_sample3, double *dsp_in, long size, int center, double scale)
{
	double *in_sample1 = dsp_in;
	double coef = 0.25 * 2.0 * M_PI * (double)center / project_sample_rate;
	double *end_input = dsp_in + size;
	double sample;

	while(in_sample1 < end_input)
	{
		sample = 0.25 * (*in_sample1 - in_sample2);
		in_sample3 += sample - coef * in_sample3;
		in_sample3 += sample - coef * in_sample3;
		in_sample3 += sample - coef * in_sample3;
		in_sample3 += sample - coef * in_sample3;
		in_sample2 = *in_sample1;
		*in_sample1++ = in_sample3 * scale;
	}
}

float ParametricMain::get_bp_level(float level)
{
	if(level == -15) level = INFINITYGAIN;
	level = db.fromdb(level);
	return level;
}

float ParametricMain::get_eq_level(float level)
{
	if(level == -15) level = -1;
	level = db.fromdb(level);
	if(level < 1) level = -(1 - level);
	else level -= 1;

	return level;
}

float ParametricMain::get_bandreject_level(float level)
{
	if(level == -15) level = INFINITYGAIN;
	level = db.fromdb(level);

	return level;
}
	
	
int ParametricMain::start_gui()
{
// defaults only need to be accessed when the GUI is spawned
	load_defaults();
	thread = new ParametricThread(this);
	thread->start();
	thread->gui_started.lock();
}

int ParametricMain::stop_gui()
{
// defaults only need to be saved when the GUI is closed
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int ParametricMain::show_gui()
{
	thread->window->show_window();
}

int ParametricMain::hide_gui()
{
	thread->window->hide_window();
}

int ParametricMain::set_string()
{
	thread->window->set_title(gui_string);
}

int ParametricMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);

	output.tag.set_title("WETNESS");
	output.tag.set_property("LEVEL", wetness);
	output.append_tag();
	output.append_newline();
	for(int i = 0; i < TOTALEQS; i++)
	{
		units[i]->save(&output);
	}
	
	output.terminate_string();
// data is now in *text
}

int ParametricMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sparametric.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	wetness = defaults->get("WETNESS", (float)INFINITYGAIN);
	for(int i = 0; i < TOTALEQS; i++)
	{
		units[i]->load_defaults(defaults, i);
	}
	if(thread) thread->window->wetness->update(wetness);
}

int ParametricMain::save_defaults()
{
	defaults->update("WETNESS", wetness);
	for(int i = 0; i < TOTALEQS; i++)
	{
		units[i]->save_defaults(defaults, i);
	}
	defaults->save();
}


int ParametricMain::read_data(char *text)
{
	FileHTAL input;
	
	input.set_shared_string(text, strlen(text));

	int unit = 0;
	int result = 0;

	while(!result && unit < TOTALEQS)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("WETNESS"))
			{
				wetness = input.tag.get_property("LEVEL", (float)0);
			}
			else
			if(input.tag.title_is("UNIT"))
			{
				units[unit]->load(&input);
				unit++;
			}
		}
	}
	
	if(thread) thread->window->wetness->update(wetness);
}




EQUnit::EQUnit(ParametricMain *client)
{
	level = 1;
	frequency = 0;
	quality = 25;
	bandpass = eqpass = lowpass = highpass = 0;
	gui_unit = 0;
	this->client = client;
}

EQUnit::~EQUnit()
{
}

int EQUnit::save(FileHTAL *htal)
{
	htal->tag.set_title("UNIT");
	htal->tag.set_property("LEVEL", (float)level);
	htal->tag.set_property("FREQ", (long)frequency);
	htal->tag.set_property("WIDTH", (long)quality);
	htal->append_tag();

	if(bandpass)
	{
		htal->tag.set_title("BANDPASS");
		htal->append_tag();
	}

	if(eqpass)
	{
		htal->tag.set_title("EQPASS");
		htal->append_tag();
	}

	if(lowpass)
	{
		htal->tag.set_title("LOWPASS");
		htal->append_tag();
	}

	if(highpass)
	{
		htal->tag.set_title("HIGHPASS");
		htal->append_tag();
	}
	
	htal->tag.set_title("/UNIT");
	htal->append_tag();
	htal->append_newline();
}

int EQUnit::load(FileHTAL *htal)
{
	level = htal->tag.get_property("LEVEL", (float)level);
	frequency = htal->tag.get_property("FREQ", (long)frequency);
	quality = htal->tag.get_property("WIDTH", (long)quality);

	int result = 0;
	eqpass = bandpass = lowpass = highpass = 0; // reset
	
	while(!result)
	{
		result = htal->read_tag();
		if(!result)
		{
			if(htal->tag.title_is("/UNIT"))
			{
				result = 1;
			}
			else
			if(htal->tag.title_is("BANDPASS"))
			{
				bandpass = 1;
			}
			else
			if(htal->tag.title_is("EQPASS"))
			{
				eqpass = 1;
			}
			else
			if(htal->tag.title_is("LOWPASS"))
			{
				lowpass = 1;
			}
			else
			if(htal->tag.title_is("HIGHPASS"))
			{
				highpass = 1;
			}
		}
	}

// some are virtual plugins and some have GUIs
	if(gui_unit) gui_unit->update();
}

int EQUnit::load_defaults(Defaults *defaults, int number)
{
	char string[1024];
	sprintf(string, "LEVEL%d", number);
	level = defaults->get(string, (float)0);
	sprintf(string, "FREQUENCY%d", number);
	frequency = defaults->get(string, (long)440);
	sprintf(string, "QUALITY%d", number);
	quality = defaults->get(string, (long)2);
	sprintf(string, "EQ%d", number);
	eqpass = defaults->get(string, 0);
	sprintf(string, "BP%d", number);
	bandpass = defaults->get(string, 0);
	sprintf(string, "LOW%d", number);
	lowpass = defaults->get(string, 0);
	sprintf(string, "HIGH%d", number);
	highpass = defaults->get(string, 0);
	if(gui_unit) gui_unit->update();
}

int EQUnit::save_defaults(Defaults *defaults, int number)
{
	char string[1024];
	sprintf(string, "LEVEL%d", number);
	defaults->update(string, level);
	sprintf(string, "FREQUENCY%d", number);
	defaults->update(string, frequency);
	sprintf(string, "QUALITY%d", number);
	defaults->update(string, quality);
	sprintf(string, "EQ%d", number);
	defaults->update(string, eqpass);
	sprintf(string, "BP%d", number);
	defaults->update(string, bandpass);
	sprintf(string, "LOW%d", number);
	defaults->update(string, lowpass);
	sprintf(string, "HIGH%d", number);
	defaults->update(string, highpass);
}

