#include "filehtal.h"
#include "freezeframe.h"

int main(int argc, char *argv[])
{
	FreezeFrameMain *plugin;

	plugin = new FreezeFrameMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

FreezeFrameMain::FreezeFrameMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
}

FreezeFrameMain::~FreezeFrameMain()
{
	if(first_frame) delete first_frame;
}

char* FreezeFrameMain::plugin_title() { return "FreezeFrame"; }
int FreezeFrameMain::plugin_is_realtime() { return 1; }
int FreezeFrameMain::plugin_is_multi_channel() { return 0; }

int FreezeFrameMain::start_realtime()
{
	first_frame = 0;
}

int FreezeFrameMain::stop_realtime()
{
}

int FreezeFrameMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	if(!first_frame)
	{
		first_frame = new VFrame(0, project_frame_w, project_frame_h);
		first_frame->copy_from(input_ptr[0]);
	}
	else
	{
		output_ptr[0]->copy_from(first_frame);
	}
}


int FreezeFrameMain::start_gui()
{
}

int FreezeFrameMain::stop_gui()
{
}

int FreezeFrameMain::show_gui()
{
}

int FreezeFrameMain::hide_gui()
{
}

int FreezeFrameMain::set_string()
{
}

int FreezeFrameMain::save_data(char *text)
{
}

int FreezeFrameMain::read_data(char *text)
{
}
