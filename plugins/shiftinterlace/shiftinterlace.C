#include "filehtal.h"
#include "shiftinterlace.h"
#include "shiftwindow.h"

int main(int argc, char *argv[])
{
	ShiftInterlaceMain *plugin;

	plugin = new ShiftInterlaceMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

ShiftInterlaceMain::ShiftInterlaceMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	odd_offset = even_offset = 0;
	thread = 0;
}

ShiftInterlaceMain::~ShiftInterlaceMain()
{
}

char* ShiftInterlaceMain::plugin_title() { return "Shift Interlace"; }
int ShiftInterlaceMain::plugin_is_realtime() { return 1; }
int ShiftInterlaceMain::plugin_is_multi_channel() { return 0; }

int ShiftInterlaceMain::start_realtime()
{
return 0;
}

int ShiftInterlaceMain::stop_realtime()
{
return 0;
}

int ShiftInterlaceMain::start_gui()
{
	load_defaults();
printf("ShiftInterlaceMain::start_gui 1\n");
	thread = new ShiftThread(this);
printf("ShiftInterlaceMain::start_gui 1\n");
	thread->start();
printf("ShiftInterlaceMain::start_gui 1\n");
	thread->gui_started.lock();
printf("ShiftInterlaceMain::start_gui 2\n");
return 0;
}

int ShiftInterlaceMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int ShiftInterlaceMain::show_gui()
{
	if(thread)
		thread->window->show_window();
return 0;
}

int ShiftInterlaceMain::hide_gui()
{
	if(thread)
		thread->window->hide_window();
return 0;
}

int ShiftInterlaceMain::set_string()
{
	if(thread)
		thread->window->set_title(gui_string);
return 0;
}

int ShiftInterlaceMain::load_defaults()
{
	char directory[1024];
	sprintf(directory, BCASTDIR "bluescreen.rc");
	
	defaults = new Defaults(directory);
	defaults->load();
	
	odd_offset = defaults->get("ODD_OFFSET", 0);
	even_offset = defaults->get("EVEN_OFFSET", 0);
	return 0;
}

int ShiftInterlaceMain::save_defaults()
{
	defaults->update("ODD_OFFSET", odd_offset);
	defaults->update("EVEN_OFFSET", even_offset);
	defaults->save();
	return 0;
}


int ShiftInterlaceMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("ODD_OFFSET");
	output.tag.set_property("VALUE", odd_offset);
	output.append_tag();
	output.tag.set_title("EVEN_OFFSET");
	output.tag.set_property("VALUE", even_offset);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int ShiftInterlaceMain::read_data(char *text)
{
	FileHTAL input;
	int result = 0;

	input.set_shared_string(text, strlen(text));

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("ODD_OFFSET"))
			{
				odd_offset = input.tag.get_property("VALUE", 0);
			}
			else
			if(input.tag.title_is("EVEN_OFFSET"))
			{
				even_offset = input.tag.get_property("VALUE", 0);
			}
		}
	}
	if(thread) 
	{
		thread->window->odd_offset->update(odd_offset);
		thread->window->even_offset->update(even_offset);
	}
	return 0;
}



int ShiftInterlaceMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	register int i, j, k, l, m;
	VPixel **input_rows, **output_rows;
	VPixel *input_row, *output_row;

	for(i = 0; i < size; i++)
	{
		input_rows = ((VPixel**)input_ptr[i]->get_rows());
		output_rows = ((VPixel**)output_ptr[i]->get_rows());

		for(j = 0; j < project_frame_h; j++)
		{
			if(j % 2)
			{
				shift_row(output_rows[j], input_rows[j], even_offset);
			}
			else
			{
				shift_row(output_rows[j], input_rows[j], odd_offset);
			}
		}
	}
	return 0;
}

int ShiftInterlaceMain::shift_row(VPixel *output_row, VPixel *input_row, int offset)
{
	int i, j;
	if(offset < 0)
	{
		for(i = 0, j = -offset; j < project_frame_w; i++, j++)
			output_row[i] = input_row[j];

		for( ; i < project_frame_w; i++)
			VFrame::clear_pixel(output_row[i]);
	}
	else
	{
		for(i = project_frame_w - 1 - offset, j = project_frame_w - 1; j >= offset; i--, j--)
			output_row[j] = output_row[i];
		
		for( ; j >= 0; j--)
			VFrame::clear_pixel(output_row[j]);
	}
	return 0;
}
