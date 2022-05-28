#include "filehtal.h"
#include "destabilize.h"

int main(int argc, char *argv[])
{
	DestabilizeMain *plugin;

	plugin = new DestabilizeMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

DestabilizeMain::DestabilizeMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	range = 10;
	accel = 1;
	current_position = 0;
	speed = 10;
	x_offset = x_offset1 = x_offset2 = 0;
	y_offset = y_offset1 = y_offset2 = 0;
	srand(time(0));
}

DestabilizeMain::~DestabilizeMain()
{
	if(defaults) delete defaults;
}

char* DestabilizeMain::plugin_title() { return "Destabilize"; }
int DestabilizeMain::plugin_is_realtime() { return 1; }
int DestabilizeMain::plugin_is_multi_channel() { return 0; }
	
int DestabilizeMain::start_realtime()
{
	x_offset = x_offset1 = x_offset2 = 0;
	y_offset = y_offset1 = y_offset2 = 0;
	current_position = -100;
	srand(time(0));
return 0;
}

int DestabilizeMain::stop_realtime()
{
return 0;
}

int DestabilizeMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	int i;
	register int j, k;
	VPixel **input_rows, **output_rows;
	int r, g, b;

	for(i = 0; i < size; i++)
	{
		input_rows = ((VPixel**)input_ptr[i]->get_rows());
		output_rows = ((VPixel**)output_ptr[i]->get_rows());

// Get coordinates
		get_coordinate(x_offset, x_offset1, x_offset2);
		get_coordinate(y_offset, y_offset1, y_offset2);
		advance_position();

		if(x_offset == 0 && y_offset == 0)
		{
// Data never processed so copy if necessary
			if(input_rows[0] != output_rows[0])
			{
				for(j = 0; j < project_frame_h; j++)
				{
					for(k = 0; k < project_frame_w; k++)
					{
						output_rows[j][k] = input_rows[j][k];
					}
				}
			}
		}
		else
		{
			offset_frame(input_ptr[i], output_ptr[i], x_offset, y_offset);
		}
	}

return 0;
}

int DestabilizeMain::get_coordinate(int &x, int x1, int x2)
{
	x = (int)((x2 - x1) * (current_position / 100) + x1);
return 0;
}

int DestabilizeMain::advance_position()
{
	if(current_position >= 0)
		current_position += speed;

	if(current_position >= 100 || current_position < 0)
	{
		current_position = 0;
		x_offset1 = x_offset2;
		y_offset1 = y_offset2;
		do
		{
			x_offset2 = (rand() % range - range / 2);
			y_offset2 = (rand() % range - range / 2);
		}while(x_offset2 == 0 || y_offset2 == 0);
	}
return 0;
}

int DestabilizeMain::offset_frame(VFrame *in, VFrame *out, int x_offset, int y_offset)
{
	int i, j, k, l;
	VPixel **in_rows = ((VPixel**)in->get_rows());
	VPixel **out_rows = ((VPixel**)out->get_rows());

	if(x_offset <= -project_frame_w || x_offset >= project_frame_w ||
		y_offset <= -project_frame_h || y_offset >= project_frame_h)
		return 0;

	if(x_offset <= 0 && y_offset <= 0)
	{
		for(i = -y_offset, j = 0; i < project_frame_h; i++, j++)
		{
			for(k = -x_offset, l = 0; k < project_frame_w; k++, l++)
			{
				out_rows[j][l] = in_rows[i][k];
			}
			for( ; l < project_frame_w; l++)
				out_rows[j][l].r = out_rows[j][l].g = out_rows[j][l].b = out_rows[j][l].a = 0;
		}
		for( ; j < project_frame_h; j++)
			clear_row(out_rows[j]);
	}
	else
	if(x_offset >= 0 && y_offset <= 0)
	{
		for(i = -y_offset, j = 0; i < project_frame_h; i++, j++)
		{
			for(k = project_frame_w - x_offset - 1, l = project_frame_w - 1; k >= 0; k--, l--)
			{
				out_rows[j][l] = in_rows[i][k];
			}
			for( ; l >= 0; l--)
				out_rows[j][l].r = out_rows[j][l].g = out_rows[j][l].b = out_rows[j][l].a = 0;
		}
		for( ; j < project_frame_h; j++)
			clear_row(out_rows[j]);
	}
	else
	if(x_offset >= 0 && y_offset >= 0)
	{
		for(i = project_frame_h - y_offset - 1, j = project_frame_h - 1; i >= 0; i--, j--)
		{
			for(k = project_frame_w - x_offset - 1, l = project_frame_w - 1; k >= 0; k--, l--)
			{
				out_rows[j][l] = in_rows[i][k];
			}
			for( ; l >= 0; l--)
				out_rows[j][l].r = out_rows[j][l].g = out_rows[j][l].b = out_rows[j][l].a = 0;
		}
		for( ; j >= 0; j--)
			clear_row(out_rows[j]);
	}
	else
	if(x_offset <= 0 && y_offset >= 0)
	{
		for(i = project_frame_h - y_offset - 1, j = project_frame_h - 1; i >= 0; i--, j--)
		{
			for(k = -x_offset, l = 0; k < project_frame_w; k++, l++)
			{
				out_rows[j][l] = in_rows[i][k];
			}
			for( ; l < project_frame_w; l++)
				out_rows[j][l].r = out_rows[j][l].g = out_rows[j][l].b = out_rows[j][l].a = 0;
		}
		for( ; j >= 0; j--)
			clear_row(out_rows[j]);
	}

return 0;
}

int DestabilizeMain::clear_row(VPixel *row)
{
	for(int i = 0; i < project_frame_w; i++)
		row[i].r = row[i].g = row[i].b = row[i].a = 0;
return 0;
}


int DestabilizeMain::start_gui()
{
	load_defaults();
	thread = new DestabilizeThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int DestabilizeMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int DestabilizeMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int DestabilizeMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int DestabilizeMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int DestabilizeMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sdestabilize.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	range = defaults->get("RANGE", range);
	accel = defaults->get("ACCEL", accel);
	speed = defaults->get("SPEED", speed);
return 0;
}

int DestabilizeMain::save_defaults()
{
	defaults->update("RANGE", range);
	defaults->update("ACCEL", accel);
	defaults->update("SPEED", speed);
	defaults->save();
return 0;
}

int DestabilizeMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("DESTABILIZE");
	output.tag.set_property("RANGE", range);
	output.tag.set_property("ACCEL", accel);
	output.tag.set_property("SPEED", speed);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int DestabilizeMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("DESTABILIZE"))
			{
				range = input.tag.get_property("RANGE", range);
				accel = input.tag.get_property("ACCEL", accel);
				speed = input.tag.get_property("SPEED", speed);
			}
		}
	}
	if(range < 1) range = 1;
	if(accel < 1) accel = 1;
	if(speed < 1) speed = 1;
	if(thread)
	{
		thread->window->range->update(range);
//		thread->window->accel->update(accel);
		thread->window->speed->update(speed);
	}
return 0;
}
