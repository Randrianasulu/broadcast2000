#include "filehtal.h"
#include "deinterlace.h"
#include "deinterwindow.h"

main(int argc, char *argv[])
{
	DeInterlaceMain *plugin;

	plugin = new DeInterlaceMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

DeInterlaceMain::DeInterlaceMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	odd_fields = 1;
	even_fields = 0;
	average_fields = 0;
	previous = 0;
	test_frame = 0;
}

DeInterlaceMain::~DeInterlaceMain()
{
	if(previous) delete previous;
	if(test_frame) delete test_frame;
}

char* DeInterlaceMain::plugin_title() { return "Deinterlace"; }
int DeInterlaceMain::plugin_is_realtime() { return 1; }
int DeInterlaceMain::plugin_is_multi_channel() { return 0; }

int DeInterlaceMain::start_realtime()
{
}

int DeInterlaceMain::stop_realtime()
{
}

int DeInterlaceMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	register int i, j, k, l, m;
	VPixel **input_rows, **output_rows;
	VPixel *input_row, *output_row;

//printf("DeInterlaceMain::process_realtime %d %d\n", odd_fields, even_fields);
	for(i = 0; i < size; i++)
	{
		input_rows = ((VPixel**)input_ptr[i]->get_rows());
		output_rows = ((VPixel**)output_ptr[i]->get_rows());

		if(even_fields)
		{
//printf(" even_fields\n");
			for(j = 0, m = 0; j < project_frame_h; )
			{
				copy_row(input_rows[m], output_rows[j], project_frame_w);
				j++;
				if(j < project_frame_h)
					copy_row(input_rows[m], output_rows[j], project_frame_w);
				j++;
				m += 2;
			}
		}
		else
		if(odd_fields)
		{
//printf(" even_fields\n");
			for(j = 0, m = 1; j < project_frame_h; )
			{
				copy_row(input_rows[m], output_rows[j], project_frame_w);
				j++;
				if(j < project_frame_h)
					copy_row(input_rows[m], output_rows[j], project_frame_w);
				j++;
				m += 2;
			}
		}
		else
		if(average_fields)
		{
			for(j = 0, k = 1; k < project_frame_h; j += 2, k += 2)
			{
				average_row(output_rows[j], output_rows[k], input_rows[j], input_rows[k], project_frame_w);
			}
			if(j < project_frame_h)
				copy_row(input_rows[j], output_rows[j], project_frame_w);
		}
		else
		if(swap_fields)
		{
			for(j = 0, k = 1; k < project_frame_h; j += 2, k += 2)
			{
				swap_row(output_rows[j], output_rows[k], input_rows[j], input_rows[k], project_frame_w);
			}
		}
		else
		if(smart_fields)
		{
			inverse_telecine(input_ptr[i], output_ptr[i]);
		}
	}
	return 0;
}

void DeInterlaceMain::copy_field(int field, VFrame *input_ptr, VFrame *output_ptr)
{
	VPixel **input_rows = ((VPixel**)input_ptr->get_rows());
	VPixel **output_rows = ((VPixel**)output_ptr->get_rows());
	int row = field;

	while(row < input_ptr->get_h())
	{
		memcpy(output_rows[row], input_rows[row], sizeof(VPixel) * input_ptr->get_w());
		row += 2;
	}
}

#define INTENSITY(p) ((unsigned int)((p.r) * 77+ \
									(p.g * 150) + \
									(p.b * 29)) >> 8)

void DeInterlaceMain::inverse_telecine(VFrame *input_ptr, VFrame *output_ptr)
{
	if(!previous) previous = new VFrame(0, 
		project_frame_w, 
		project_frame_h);
	if(!test_frame) test_frame = new VFrame(0, 
		project_frame_w, 
		project_frame_h);

// Test 3 combinations of the previous frame
	longest score_nchange;
	longest score_bottom;

	test_frame->copy_from(input_ptr);
	score_nchange = 0;

	copy_field(1, previous, test_frame);
	score_bottom = 0;

//printf("DeInterlaceMain::inverse_telecine %d %lld %lld\n", source_position, score_nchange, score_bottom);

	if(1) 
	{
		copy_field(1, previous, test_frame);
		copy_field(0, input_ptr, test_frame);
		previous->copy_from(input_ptr);
		output_ptr->copy_from(test_frame);
		return;
	}
	else
	{
		previous->copy_from(input_ptr);
		return;
	}
}


int DeInterlaceMain::copy_row(VPixel *input_row, VPixel *output_row, int w)
{
	for(int k = 0; k < w; k++)
	{
		output_row[k] = input_row[k];
	}
}

int DeInterlaceMain::swap_row(VPixel *output1, VPixel *output2, VPixel *input1, VPixel *input2, int w)
{
	VPixel temp;
	for(int k = 0; k < w; k++)
	{
		temp = input2[k];
		output2[k] = input1[k];
		output1[k] = temp;
	}
}

int DeInterlaceMain::average_row(VPixel *output1, VPixel *output2, VPixel *input1, VPixel *input2, int w)
{
	int r, g, b, a;
	for(int i = 0; i < w; i++)
	{
		r = input1[i].r + input2[i].r;
		g = input1[i].g + input2[i].g;
		b = input1[i].b + input2[i].b;
		a = input1[i].a + input2[i].a;
		r /= 2;
		g /= 2;
		b /= 2;
		a /= 2;
		output1[i].r = output2[i].r = r;
		output1[i].g = output2[i].g = g;
		output1[i].b = output2[i].b = b;
		output1[i].a = output2[i].a = a;
	}
}

int DeInterlaceMain::start_gui()
{
	thread = new DeInterlaceThread(this);
	thread->start();
	thread->gui_started.lock();
}

int DeInterlaceMain::stop_gui()
{
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int DeInterlaceMain::show_gui()
{
	thread->window->show_window();
}

int DeInterlaceMain::hide_gui()
{
	thread->window->hide_window();
}

int DeInterlaceMain::set_string()
{
	thread->window->set_title(gui_string);
}

int DeInterlaceMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	if(odd_fields)
	{
		output.tag.set_title("ODDFIELDS");
		output.append_tag();
	}
	if(even_fields)
	{
		output.tag.set_title("EVENFIELDS");
		output.append_tag();
	}
	if(average_fields)
	{
		output.tag.set_title("AVGFIELDS");
		output.append_tag();
	}
	if(swap_fields)
	{
		output.tag.set_title("SWAPFIELDS");
		output.append_tag();
	}
	if(smart_fields)
	{
		output.tag.set_title("REVERSETELECINE");
		output.append_tag();
	}
	output.terminate_string();
// data is now in *text
	return 0;
}

int DeInterlaceMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;
	odd_fields = 0;
	even_fields = 0;
	average_fields = 0;
	swap_fields = 0;
	smart_fields = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("ODDFIELDS"))
			{
				odd_fields = 1;
// You'll usually set a flag here to inform process_realtime
// that it needs to reconfigure itself or something.
			}
			else
			if(input.tag.title_is("EVENFIELDS"))
			{
				even_fields = 1;
// You'll usually set a flag here to inform process_realtime
// that it needs to reconfigure itself or something.
			}
			else
			if(input.tag.title_is("AVGFIELDS"))
			{
				average_fields = 1;
			}
			else
			if(input.tag.title_is("SWAPFIELDS"))
			{
				swap_fields = 1;
			}
			else
			if(input.tag.title_is("REVERSETELECINE"))
			{
				smart_fields = 1;
			}
		}
	}
	if(thread) 
	{
		thread->window->odd_fields->update(odd_fields);
		thread->window->even_fields->update(even_fields);
		thread->window->average_fields->update(average_fields);
		thread->window->swap_fields->update(swap_fields);
		thread->window->smart_fields->update(smart_fields);
	}
	return 0;
}
