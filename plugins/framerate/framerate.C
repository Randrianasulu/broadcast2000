#include "errorbox.h"
#include "framerate.h"
#include "fratewindow.h"
//#include "units.h"

main(int argc, char *argv[])
{
	FrameRateMain *plugin;
	
	plugin = new FrameRateMain(argc, argv);
	plugin->plugin_run();
}

FrameRateMain::FrameRateMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
}

FrameRateMain::~FrameRateMain()
{
}

int FrameRateMain::run_client()
{
	plugin_exit();
}

char* FrameRateMain::plugin_title() { return "Reframe"; }
int FrameRateMain::plugin_is_realtime() { return 0; }
int FrameRateMain::plugin_is_multi_channel() { return 0; }

int FrameRateMain::load_defaults()
{
	char directory[1024];

// set the default directory
	sprintf(directory, "%sframerate.rc", BCASTDIR);

// load the defaults

	defaults = new Defaults(directory);

	defaults->load();

	output_rate = defaults->get("OUTPUTRATE", (float)24);
}

int FrameRateMain::save_defaults()
{
	defaults->update("OUTPUTRATE", output_rate);
	defaults->save();
}

float FrameRateMain::get_plugin_framerate()
{
	return output_rate;
}

int FrameRateMain::get_parameters()
{
// don't take parameters
	int frame_rate_conflicts;
	int result = 0;

	input_rate = get_project_framerate();

	do{
		frame_rate_conflicts = 0;
		{
			FRateWindow window;
			window.create_objects(&output_rate);
			result = window.run_window();
		}

		if(!result)
		{
			if(input_rate == output_rate)
			{
				frame_rate_conflicts = 1;
				ErrorBox window;
				window.create_objects("The new frame rate is the", "same as the old frame rate.");
				window.run_window();
			}
			else
			if(output_rate > 200000 || output_rate < 0)
			{
				frame_rate_conflicts = 1;
				ErrorBox window;
				window.create_objects("The new frame rate is out of range.");
				window.run_window();
			}
		}
	}while(frame_rate_conflicts);

	send_completed();
	return result;
}

int FrameRateMain::start_plugin()
{
// thread out progress
	BC_ProgressBox *progress;

	VFrame **buffer_in, **buffer_out;
	float offset = 0;
	int i, j;

	if(interactive)
	{
		progress = new BC_ProgressBox("", "Reframe Video", end - start);
		progress->start();
	}

// get arrays of frames for 1 channel
	buffer_in = video_in[0];
	buffer_out = video_out[0];

	long input_position_i, output_position_i, last_input;
	float input_position_f;
	float input_advance;
	int result = 0;

	input_rate = get_project_framerate();
	last_input = -1;
	input_position_i = start;
	input_position_f = start;
	output_position_i = 0;
	input_advance = input_rate / output_rate;
//printf("%ld input_advance  %f\n", input_position_i, input_advance);

	while(!result && input_position_i < end)
	{
// Read a new frame if different
		if(input_position_i != last_input)
			result = read_frames(input_position_i, 1);

		if(!result)
		{
			if(buffer_in[0]->get_rows() != buffer_out[0]->get_rows())
			buffer_out[0]->copy_from(buffer_in[0]);

			result = write_frames(1);
		}

		last_input = input_position_i;
		output_position_i++;
		input_position_f += input_advance;
		input_position_i = (long)(input_position_f + 0.5);

		if(interactive)
		{
			if(!result) result = progress->update(input_position_i - start);
			if(progress->cancelled()) send_cancelled();
		}
	}while(!result && input_position_i < end);

	if(interactive)
	{
		progress->stop_progress();
		delete progress;
	}
}
