#include "normalize.h"
#include "normalizewindow.h"
#include "units.h"

int main(int argc, char *argv[])
{
	NormalizeMain *plugin;
	
	plugin = new NormalizeMain(argc, argv);
	plugin->plugin_run();
}

NormalizeMain::NormalizeMain(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
	load_defaults();
}

NormalizeMain::~NormalizeMain()
{
	save_defaults();
	delete defaults;
}

int NormalizeMain::run_client()
{
	plugin_exit();
return 0;
}

char* NormalizeMain::plugin_title() { return "Normalize"; }

int NormalizeMain::plugin_is_realtime() { return 0; }

int NormalizeMain::plugin_is_multi_channel() { return 1; }

int NormalizeMain::load_defaults()
{
	char directory[1024];

// set the default directory
	sprintf(directory, "%snormalize.rc", BCASTDIR);
	
// load the defaults

	defaults = new Defaults(directory);

	defaults->load();

	db_over = defaults->get("DBOVER", 0);
	separate_tracks = defaults->get("SEPERATE_TRACKS", 1);
return 0;
}

int NormalizeMain::save_defaults()
{
	defaults->update("DBOVER", db_over);
	defaults->update("SEPERATE_TRACKS", separate_tracks);
	defaults->save();
return 0;
}

int NormalizeMain::get_parameters()
{
	int result;
	
	{
		NormalizeWindow window;
		window.create_objects(&db_over, &separate_tracks);
		result = window.run_window();
	}
	
	send_completed();
	return result;
}

int NormalizeMain::start_plugin()
{
// thread out progress
	BC_ProgressBox *progress;
	DB db;

	float **buffer_in, **buffer_out;
	float power_over = db.fromdb(db_over);

	buffer_in = new float*[total_in_buffers];
	buffer_out = new float*[total_out_buffers];

	if(interactive)
	{
		progress = new BC_ProgressBox("", "Normalize", (end - start) * 2);
		progress->start();
	}

// get standard data buffers
	for(int i = 0; i < total_in_buffers; i++)
		buffer_in[i] = (float*)data_in[i]->get_data();

	for(int i = 0; i < total_out_buffers; i++)	
		buffer_out[i] = (float*)data_out[i]->get_data();

// get highest sample for each track
	float *highest_sample, test_sample, *current_highest;
	int result = 0;
	long input_advance = in_buffer_size;
	float *buffer_in_number, *buffer_out_number;

	highest_sample = new float[total_in_buffers];

	for(int i = 0; i < total_in_buffers; i++)
		highest_sample[i] = 0;

	for(long input_position = start; input_position < end && !result; input_position += input_advance)
	{
		if(end - input_position < input_advance) input_advance = end - input_position;

// returns 1 for failure
		result = read_samples(input_position, input_advance);

		if(!result)
		for(int buffer_number = 0; buffer_number < total_in_buffers; buffer_number++)
		{
			buffer_in_number = buffer_in[buffer_number];
			current_highest = &highest_sample[buffer_number];
			for(register long buffer_pointer = 0; buffer_pointer < input_advance; buffer_pointer++)
			{
				test_sample = fabs(buffer_in_number[buffer_pointer]);
				if(test_sample > *current_highest) *current_highest = test_sample;
			}
		}

		if(interactive)
		{
			if(!result) result = progress->update(input_position + input_advance - start);
			if(progress->cancelled()) send_cancelled();
		}
	}

// process the same data again
	if(!result)
	{
		input_advance = in_buffer_size;
		float *scale, current_scale;
		
		scale = new float[total_in_buffers];

		for(int i = 0; i < total_in_buffers; i++)
		{
			scale[i] = power_over / highest_sample[i];
		}

		if(!result)
		{
			char string[1024];
			sprintf(string, "Normalize %d%%", (int)(scale[0] * 100));
			progress->update_title(string);
		}

		if(!separate_tracks)
		{
// use lowest scale if tracks are to be treated equally
			float lowest_scale = scale[0];
			for(int i = 1; i < total_in_buffers; i++)
			{
				if(scale[i] < lowest_scale) lowest_scale = scale[i];
			}

			for(int i = 0; i < total_in_buffers; i++)
			{
				scale[i] = lowest_scale;
			}
		}

		for(long input_position = start; input_position < end && !result; input_position += input_advance)
		{
			if(end - input_position < input_advance) input_advance = end - input_position;

// returns 1 for failure
			result = read_samples(input_position, input_advance);
			float *output;

			if(!result)
			{
				for(int buffer_number = 0; buffer_number < total_in_buffers; buffer_number++)
				{
					buffer_in_number = buffer_in[buffer_number];
					buffer_out_number = buffer_out[buffer_number];
					current_scale = scale[buffer_number];
					for(register long buffer_pointer = 0; buffer_pointer < input_advance; buffer_pointer++)
					{
						output = &buffer_out_number[buffer_pointer];
						*output = buffer_in_number[buffer_pointer] * current_scale;
						if(*output > 1) *output = 1;
						else
						if(*output < -1) *output = -1;
					}
				}

				result = write_samples(input_advance);

				if(interactive)
				{
					if(!result) result = progress->update(end - start + input_position + input_advance - start);
					if(progress->cancelled()) send_cancelled();
				}
			}
		}
		delete [] scale;
	}

	if(interactive)
	{
		progress->stop_progress();
		delete progress;
	}
	
	delete [] buffer_in;
	delete [] buffer_out;
	delete [] highest_sample;
return 0;
}
