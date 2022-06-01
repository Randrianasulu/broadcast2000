#include "offsetdc.h"
#include "units.h"

int main(int argc, char *argv[])
{
	OffsetMain *plugin;
	
	plugin = new OffsetMain(argc, argv);
	plugin->plugin_run();
}

OffsetMain::OffsetMain(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
}

OffsetMain::~OffsetMain()
{
}

int OffsetMain::run_client()
{
	plugin_exit();
return 0;
}

char* OffsetMain::plugin_title() { return "DC Offset"; }
int OffsetMain::plugin_is_realtime() { return 0; }
int OffsetMain::plugin_is_multi_channel() { return 0; }

int OffsetMain::get_parameters()
{
// don't take parameters
	send_completed();
	return 0;
}

int OffsetMain::start_plugin()
{
// thread out progress
	BC_ProgressBox *progress;
	DB db;
	
	float *buffer_in, *buffer_out;
	float offset = 0;
	
	if(interactive)
	{
		progress = new BC_ProgressBox("", "Offset", end - start);
		progress->start();
	}

// get standard data buffers
	buffer_in = (float*)data_in[0]->get_data();
	buffer_out = (float*)data_out[0]->get_data();

// get average
	double total = 0;
	long input_advance = in_buffer_size;
	long end_test = start + 1000000;
	int result = 0;
	if(end_test > end) end_test = end;
	
	for(long input_position = start; input_position < end_test && !result; input_position += input_advance)
	{
		if(end_test - input_position < input_advance) input_advance = end_test - input_position;
		
// returns 1 for failure
		result = read_samples(input_position, input_advance);

		if(!result)
		for( long buffer_pointer = 0; buffer_pointer < input_advance; buffer_pointer++)
		{
			total += buffer_in[buffer_pointer];
		}
	}

	offset = total / (end_test - start);

// process the entire file
	if(!result)
	{
		input_advance = in_buffer_size;
		
		for(long input_position = start; input_position < end && !result; input_position += input_advance)
		{
			if(end - input_position < input_advance) input_advance = end - input_position;

// returns 1 for failure
			result = read_samples(input_position, input_advance);
			for( long buffer_pointer = 0; buffer_pointer < input_advance; buffer_pointer++)
			{
				buffer_out[buffer_pointer] = buffer_in[buffer_pointer] - offset;
			}

			result = write_samples(input_advance);
				
			if(interactive)
			{
				if(!result) result = progress->update(input_position + input_advance - start);
				if(progress->cancelled()) send_cancelled();
			}
		}
	}

	if(interactive)
	{
		progress->stop_progress();
		delete progress;
	}
return 0;
}
