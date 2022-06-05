#include "filehtal.h"
#include "timeavg.h"
#include "timeavgwindow.h"

int main(int argc, char *argv[])
{
	TimeAvgMain *plugin;

	plugin = new TimeAvgMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

TimeAvgMain::TimeAvgMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	total_frames = 1;
	frames_allocated = 0;
	ring_buffer = 0;
	redo_buffers = 1;
}

TimeAvgMain::~TimeAvgMain()
{
	if(defaults) delete defaults;
}

const char* TimeAvgMain::plugin_title() { return "Time Average"; }
int TimeAvgMain::plugin_is_realtime() { return 1; }
int TimeAvgMain::plugin_is_multi_channel() { return 0; }
	
int TimeAvgMain::start_realtime()
{
return 0;
}

int TimeAvgMain::stop_realtime()
{
	int i;
	if(ring_buffer)
	{
		for(i = 0; i < frames_allocated; i++)
			delete ring_buffer[i];
		delete ring_buffer;
	}
return 0;
}

int TimeAvgMain::redo_buffers_procedure()
{
	VFrame **new_frames;
	int i;

	new_frames = new VFrame*[total_frames];
	for(i = 0; i < total_frames; i++)
	{
		new_frames[i] = new VFrame(0, project_frame_w, project_frame_h);
		if(i < frames_allocated && ring_buffer)
			new_frames[i]->copy_from(ring_buffer[i]);
	}
	if(ring_buffer)
	{
		for(i = 0; i < frames_allocated; i++)
			delete ring_buffer[i];
		delete ring_buffer;
	}
	frames_allocated = total_frames;
	ring_buffer = new_frames;
	if(current_frame >= total_frames) current_frame = 0;
	redo_buffers = 0;
return 0;
}


int TimeAvgMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	if(redo_buffers) redo_buffers_procedure();

	int i, j, k, l, m;
	VPixel *input_row, *output_row;
	typedef struct {
		int r;
		int g;
		int b;
		int a;
	} temp_pixel;
	temp_pixel *temp_row;
	int r, g, b;

	temp_row = new temp_pixel[project_frame_w];

	for(i = 0; i < size; i++)
	{
// Copy frame to ring buffer
		ring_buffer[current_frame]->copy_from(input_ptr[i]);

// Overlay one row at a time.
		for(k = 0; k < project_frame_h; k++)
		{
			j = current_frame;
// Overlay one row from each frame onto the temp_row
			for(l = 0; l < total_frames; l++)
			{
				input_row = ((VPixel**)ring_buffer[j]->get_rows())[k];
// First frame overwrites
				if(l == 0)
					for(m = 0; m < project_frame_w; m++)
					{
						temp_row[m].r = input_row[m].r;
						temp_row[m].g = input_row[m].g;
						temp_row[m].b = input_row[m].b;
						temp_row[m].a = input_row[m].a;
					}
				else
					for(m = 0; m < project_frame_w; m++)
					{
						temp_row[m].r += input_row[m].r;
						temp_row[m].g += input_row[m].g;
						temp_row[m].b += input_row[m].b;
						temp_row[m].a += input_row[m].a;
					}

// Get next ring_buffer frame
				j++;
				if(j >= total_frames) j = 0;
			}

// Normalize the row and write to output frame
			output_row = ((VPixel**)output_ptr[i]->get_rows())[k];
			for(m = 0; m < project_frame_w; m++)
			{
				temp_row[m].r /= total_frames;
				temp_row[m].g /= total_frames;
				temp_row[m].b /= total_frames;
				temp_row[m].a /= total_frames;
				test_clip(temp_row[m].r, temp_row[m].g, temp_row[m].b, temp_row[m].a);
				output_row[m].r = temp_row[m].r;
				output_row[m].g = temp_row[m].g;
				output_row[m].b = temp_row[m].b;
				output_row[m].a = temp_row[m].a;
			}
// Next row
		}
// Overlay next frame onto ring_buffer
		current_frame++;
		if(current_frame >= total_frames) current_frame = 0;
	}
	delete [] temp_row;
return 0;
}

int TimeAvgMain::test_clip(int &r, int &g, int &b, int &a)
{
	if(r > VMAX) r = VMAX;
	if(g > VMAX) g = VMAX;
	if(b > VMAX) b = VMAX;
	if(a > VMAX) a = VMAX;

	return 0;
}


int TimeAvgMain::start_gui()
{
	load_defaults();
	thread = new TimeAvgThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int TimeAvgMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int TimeAvgMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int TimeAvgMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int TimeAvgMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int TimeAvgMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sbrightness.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	total_frames = defaults->get("TOTAL_FRAMES", total_frames);
return 0;
}

int TimeAvgMain::save_defaults()
{
	defaults->update("TOTAL_FRAMES", total_frames);
	defaults->save();
return 0;
}

int TimeAvgMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("TIME_AVERAGE");
	output.tag.set_property("TOTAL_FRAMES", total_frames);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int TimeAvgMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("TIME_AVERAGE"))
			{
				total_frames = input.tag.get_property("TOTAL_FRAMES", total_frames);
			}
		}
	}
	if(thread) 
	{
		thread->window->total_frames->update(total_frames);
	}
	redo_buffers = 1;
return 0;
}
