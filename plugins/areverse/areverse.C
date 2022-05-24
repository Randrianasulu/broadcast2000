#include "areverse.h"
#include "units.h"

int main(int argc, char *argv[])
{
	AReverseMain *plugin;
	
	plugin = new AReverseMain(argc, argv);
	plugin->plugin_run();
}

AReverseMain::AReverseMain(int argc, char *argv[])
 : PluginAClient(argc, argv)
{
}

AReverseMain::~AReverseMain()
{
}

int AReverseMain::run_client()
{
	plugin_exit();
}

char* AReverseMain::plugin_title() { return "Reverse Audio"; }
int AReverseMain::plugin_is_realtime() { return 0; }
int AReverseMain::plugin_is_multi_channel() { return 0; }

int AReverseMain::get_parameters()
{
// don't take parameters
	send_completed();
	return 0;
}

int AReverseMain::start_plugin()
{
// thread out progress
	BC_ProgressBox *progress;

	float *buffer_in, *buffer_out, temp;
	float offset = 0;

	if(interactive)
	{
		progress = new BC_ProgressBox("", "Reverse Audio", end - start);
		progress->start();
	}

// get standard data buffers
	buffer_in = (float*)data_in[0]->get_data();
	buffer_out = (float*)data_out[0]->get_data();

	double total = 0;
	long output_advance = in_buffer_size;
	long input_position, output_position;
	long i, j;
	int result = 0;

	input_position = end;
	output_position = 0; 

	do
	{
		output_advance = in_buffer_size;
		if(input_position - output_advance < start)
			output_advance = input_position - start;

		input_position -= output_advance;

		result = read_samples(input_position, output_advance);
		for(i = 0, j = output_advance - 1; j >= 0; i++, j--)
		{
//printf("AReverseMain::start_plugin %ld %ld\n", i, j);
			buffer_out[i] = buffer_in[j];
		}

		result = write_samples(output_advance);
		output_position += output_advance;

		if(interactive)
		{
			if(!result) result = progress->update(output_position);
			if(progress->cancelled()) send_cancelled();
		}
	}while(!result && input_position > start);

	if(interactive)
	{
		progress->stop_progress();
		delete progress;
	}
}
