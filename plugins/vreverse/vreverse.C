#include "vreverse.h"
//#include "units.h"

int main(int argc, char *argv[])
{
	VReverseMain *plugin;
	
	plugin = new VReverseMain(argc, argv);
	plugin->plugin_run();
}

VReverseMain::VReverseMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
}

VReverseMain::~VReverseMain()
{
}

int VReverseMain::run_client()
{
	plugin_exit();
}

char* VReverseMain::plugin_title() { return "Reverse Video"; }
int VReverseMain::plugin_is_realtime() { return 0; }
int VReverseMain::plugin_is_multi_channel() { return 0; }

int VReverseMain::get_parameters()
{
// don't take parameters
	send_completed();
	return 0;
}

int VReverseMain::start_plugin()
{
// thread out progress
	BC_ProgressBox *progress;
	char string[1024];

	VFrame **buffer_in, **buffer_out;
	float offset = 0;
	int i, j;

	if(interactive)
	{
		progress = new BC_ProgressBox("", "Reverse Video", end - start);
		progress->start();
	}

// get standard data buffers for 1 channel
	buffer_in = video_in[0];
	buffer_out = video_out[0];

	long output_advance = in_buffer_size;
	long input_position, output_position;
	int result = 0;

	input_position = end - 1;
	output_position = 0;

	for(input_position = end - 1, output_position = 0; 
		output_position < end - start && !result; 
		output_position++, input_position--)
	{
		result = read_frames(input_position, 1);

		swap_frames(buffer_out[0], buffer_in[0]);

		result = write_frames(1);

		if(interactive)
		{
			if(!result) result = progress->update(output_position);
			if(progress->cancelled()) send_cancelled();
		}
	}

	if(interactive)
	{
		progress->stop_progress();
		delete progress;
	}
}

int VReverseMain::swap_frames(VFrame *frame1, VFrame *frame2)
{
	int i, j;
	VPixel temp;
	VPixel **rows1, **rows2;

	rows1 = (VPixel**)frame1->get_rows();
	rows2 = (VPixel**)frame2->get_rows();

	frame1->copy_from(frame2);

//	for(i = 0; i < project_frame_h; i++)
//	{
//printf("%d %p\n", i, rows2[i]);
// 		for(j = 0; j < project_frame_w; j++)
// 		{
// 			temp = rows1[i][j];
// 			rows1[i][j] = rows2[i][j];
// 			rows2[i][j] = temp;
// 		}
//	}
	return 0;
}
