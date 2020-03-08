#include "filehtal.h"
#include "sharpen.h"
#include "sharpenwindow.h"

main(int argc, char *argv[])
{
	SharpenMain *plugin;

	plugin = new SharpenMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

SharpenMain::SharpenMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	sharpness = 0;
}

SharpenMain::~SharpenMain()
{
}

char* SharpenMain::plugin_title() { return "Sharpen"; }
int SharpenMain::plugin_is_realtime() { return 1; }
int SharpenMain::plugin_is_multi_channel() { return 0; }

int SharpenMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%ssharpen.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	sharpness = defaults->get("SHARPNESS", 50);
	interlace = defaults->get("INTERLACE", 0);
}

int SharpenMain::save_defaults()
{
	defaults->update("SHARPNESS", sharpness);
	defaults->update("INTERLACE", interlace);
	defaults->save();
}

int SharpenMain::start_realtime()
{
// Initialize
	get_luts(pos_lut, neg_lut);
	last_sharpness = sharpness;

	total_engines = smp > 1 ? 2 : 1;
	engine = new SharpenEngine*[total_engines];
	for(int i = 0; i < total_engines; i++)
	{
		engine[i] = new SharpenEngine(this);
		engine[i]->start();
	}
}

int SharpenMain::stop_realtime()
{
	for(int i = 0; i < total_engines; i++)
	{
		delete engine[i];
	}
	delete engine;
}

int SharpenMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	register int i, j, k;
	float old_sharpness = sharpness;
	VPixel **input_rows, **output_rows;

	for(i = 0; i < size; i++)
	{
		sharpness = old_sharpness;

		if(automation_used())
			sharpness += (get_automation_value(i) * MAXSHARPNESS);

		if(sharpness > MAXSHARPNESS) 
			sharpness = MAXSHARPNESS;
		else
			if(sharpness < 0) sharpness = 0;

		if(last_sharpness != sharpness) get_luts(pos_lut, neg_lut);
		if(sharpness != 0)
		{
// Arm first row
			row_step = interlace ? 2 : 1;

			for(j = 0; j < row_step; j += total_engines)
			{
				for(k = 0; k < total_engines && k + j < row_step; k++)
				{
					engine[k]->start_process_frame(input_ptr[i], input_ptr[i], k + j);
				}
				for(k = 0; k < total_engines && k + j < row_step; k++)
				{
					engine[k]->wait_process_frame();
				}
			}
		}
		else
		if(!buffers_identical(0))
		{
			input_rows = (VPixel**)input_ptr[i]->get_rows();
			output_rows = (VPixel**)output_ptr[i]->get_rows();

// Data never processed so copy if necessary
			for(j = 0; j < project_frame_h; j++)
			{
				for(k = 0; k < project_frame_w; k++)
				{
					output_rows[j][k] = input_rows[j][k];
				}
			}
		}
		last_sharpness = sharpness;
	}
	sharpness = old_sharpness;
}


int SharpenMain::get_luts(int *pos_lut, int *neg_lut)
{
	int i, inv_sharpness;

	inv_sharpness = (int)(100 - sharpness);
	if(inv_sharpness < 1) inv_sharpness = 1;
	for(i = 0; i < VMAX + 1; i++)
	{
		pos_lut[i] = 800 * i / inv_sharpness;
		neg_lut[i] = (4 + pos_lut[i] - (i << 3)) >> 3;
	}
}

int SharpenMain::start_gui()
{
	load_defaults();
	thread = new SharpenThread(this);
	thread->start();
	thread->gui_started.lock();
}

int SharpenMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int SharpenMain::show_gui()
{
	thread->window->show_window();
}

int SharpenMain::hide_gui()
{
	thread->window->hide_window();
}

int SharpenMain::set_string()
{
	thread->window->set_title(gui_string);
}

int SharpenMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("SHARPNESS");
	output.tag.set_property("VALUE", sharpness);
	output.append_tag();

	if(interlace)
	{
		output.tag.set_title("INTERLACE");
		output.append_tag();
	}
	output.terminate_string();
// data is now in *text
}

int SharpenMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;
	int new_interlace = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("SHARPNESS"))
			{
				sharpness = input.tag.get_property("VALUE", sharpness);
				get_luts(pos_lut, neg_lut);
				last_sharpness = sharpness;
			}
			else
			if(input.tag.title_is("INTERLACE"))
			{
				new_interlace = 1;
			}
		}
	}

	interlace = new_interlace;
	if(thread) 
	{
		thread->window->sharpen_slider->update((int)sharpness);
		thread->window->sharpen_interlace->update(interlace);
	}
}




SharpenEngine::SharpenEngine(SharpenMain *plugin)
 : Thread()
{
	this->plugin = plugin;
	last_frame = 0;
	for(int i = 0; i < 4; i++)
	{
		neg_rows[i] = new int[plugin->project_frame_w * 4];
	}
	input_lock.lock();
	output_lock.lock();
}

SharpenEngine::~SharpenEngine()
{
	last_frame = 1;
	input_lock.unlock();
	join();

	for(int i = 0; i < 4; i++)
	{
		delete neg_rows[i];
	}
}

int SharpenEngine::start_process_frame(VFrame *output, VFrame *input, int field)
{
	this->output = output;
	this->input = input;
	this->field = field;
	input_lock.unlock();
}

int SharpenEngine::wait_process_frame()
{
	output_lock.lock();
}

void SharpenEngine::run()
{
	int i, j, k, count, row;
	VPixel **input_rows, **output_rows;

	while(1)
	{
		input_lock.lock();
		if(last_frame)
		{
			output_lock.unlock();
			return;
		}

		input_rows = (VPixel**)input->get_rows();
		output_rows = (VPixel**)output->get_rows();
		src_rows[0] = input_rows[field];
		src_rows[1] = input_rows[field];
		src_rows[2] = input_rows[field];
		src_rows[3] = input_rows[field];
		for(j = 0; j < plugin->project_frame_w; j++)
		{
			neg_rows[0][j * 4] = plugin->neg_lut[src_rows[0][j].r];
			neg_rows[0][j * 4 + 1] = plugin->neg_lut[src_rows[0][j].g];
			neg_rows[0][j * 4 + 2] = plugin->neg_lut[src_rows[0][j].b];
			neg_rows[0][j * 4 + 3] = plugin->neg_lut[src_rows[0][j].a];
		}

		row = 1;
		count = 1;

		for(i = field; i < plugin->project_frame_h; i += plugin->row_step)
		{
			if((i + plugin->row_step) < plugin->project_frame_h)
			{
				if(count >= 3) count--;
// Arm next row
				src_rows[row] = input_rows[i + plugin->row_step];
				for(k = 0; k < plugin->project_frame_w; k++)
				{
					neg_rows[row][k * 4] = plugin->neg_lut[src_rows[row][k].r];
					neg_rows[row][k * 4 + 1] = plugin->neg_lut[src_rows[row][k].g];
					neg_rows[row][k * 4 + 2] = plugin->neg_lut[src_rows[row][k].b];
					neg_rows[row][k * 4 + 3] = plugin->neg_lut[src_rows[row][k].a];
				}

				count++;
				row = (row + 1) & 3;
			}
			else
			{
				count--;
			}

			dst_row = output_rows[i];
			if(count == 3)
			{
// Do the filter
				filter(plugin->project_frame_w, 
					src_rows[(row + 2) & 3], 
					dst_row,
					neg_rows[(row + 1) & 3] + 4,
					neg_rows[(row + 2) & 3] + 4,
					neg_rows[(row + 3) & 3] + 4);
			}
			else 
			if(count == 2)
			{
				if(i == 0)
					for(k = 0; k < plugin->project_frame_w; k++) dst_row[k] = src_rows[0][k];
				else
					for(k = 0; k < plugin->project_frame_w; k++) dst_row[k] = src_rows[2][k];
			}
		}

		output_lock.unlock();
	}
}

int SharpenEngine::filter(register int w, VPixel *src, VPixel *dst,
	int *neg0, int *neg1, int *neg2)
{
	int pixel;
	
	*dst++ = *src++;
	w -= 2;
	
	while(w > 0)
	{
		pixel = plugin->pos_lut[src->r] - neg0[-4] - neg0[0] - neg0[4] - 
			neg1[-4] - neg1[4] - neg2[-4] - neg2[0] - neg2[4];
		pixel = (pixel + 4) >> 3;
		if(pixel < 0) dst->r = 0;
		else
		if(pixel > VMAX) dst->r = VMAX;
		else
		dst->r = pixel;

		pixel = plugin->pos_lut[src->g] - neg0[-3] - neg0[1] - neg0[5] - 
			neg1[-3] - neg1[5] - neg2[-3] - neg2[1] - neg2[5];
		pixel = (pixel + 4) >> 3;
		if(pixel < 0) dst->g = 0;
		else
		if(pixel > VMAX) dst->g = VMAX;
		else
		dst->g = pixel;

		pixel = plugin->pos_lut[src->b] - neg0[-2] - neg0[2] - neg0[6] - 
			neg1[-2] - neg1[6] - neg2[-2] - neg2[2] - neg2[6];
		pixel = (pixel + 4) >> 3;
		if(pixel < 0) dst->b = 0;
		else
		if(pixel > VMAX) dst->b = VMAX;
		else
		dst->b = pixel;

// Don't touch alpha
		*dst = *src;

		src++;
		dst++;
		
		neg0 += 4;
		neg1 += 4;
		neg2 += 4;
		w--;
	}
	
	*dst++ = *src++;
}
