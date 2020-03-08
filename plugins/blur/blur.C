#include <math.h>
#include "filehtal.h"
#include "blur.h"
#include "blurwindow.h"

main(int argc, char *argv[])
{
	BlurMain *plugin;

	plugin = new BlurMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
	srand(time(0));
}

BlurMain::BlurMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	vertical = 1;
	horizontal = 1;
	radius = 5;
	defaults = 0;
}

BlurMain::~BlurMain()
{
	if(defaults) delete defaults;
}

char* BlurMain::plugin_title() { return "Blur"; }
int BlurMain::plugin_is_realtime() { return 1; }
int BlurMain::plugin_is_multi_channel() { return 0; }

int BlurMain::start_realtime()
{
	int y1, y2, y_increment;
	last_radius = radius;
	redo_buffers = 1;

	y_increment = project_frame_h / smp;
	y1 = 0;

	engine = new BlurEngine*[smp];
	for(int i = 0; i < smp; i++)
	{
		y2 = y1 + y_increment;
		if(i == smp - 1 && y2 < project_frame_h - 1) y2 = project_frame_h - 1;
		engine[i] = new BlurEngine(this, y1, y2);
		engine[i]->start();
		y1 += y_increment;
	}
}

int BlurMain::stop_realtime()
{
}

int BlurMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	int i, j, k, l;
	int old_radius = radius;
	VPixel **input_rows, **output_rows;

	for(i = 0; i < size; i++)
	{
		input_rows = (VPixel**)input_ptr[i]->get_rows();
		output_rows = (VPixel**)output_ptr[i]->get_rows();
		radius = old_radius;

		if(automation_used())
		{
			radius += (int)(get_automation_value(i) * MAXRADIUS);
			if(radius < 0) radius = 0;
		}
//printf("BlurMain::process_realtime %d\n", radius);

		if(last_radius != radius || redo_buffers)
		{
			for(i = 0; i < smp; i++)
				engine[i]->reconfigure();
			redo_buffers = 0;
		}

		last_radius = radius;
		if(radius < 2 || (!vertical && !horizontal))
		{
// Data never processed so copy if necessary
			if(!buffers_identical(0))
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
// Process blur
			for(i = 0; i < smp; i++)
			{
				engine[i]->start_process_frame(output_ptr, input_ptr, size);
			}

			for(i = 0; i < smp; i++)
			{
				engine[i]->wait_process_frame();
			}
		}
	}
	radius = old_radius;
}

int BlurMain::start_gui()
{
	load_defaults();
	thread = new BlurThread(this);
	thread->start();
	thread->gui_started.lock();
}

int BlurMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int BlurMain::show_gui()
{
	thread->window->show_window();
}

int BlurMain::hide_gui()
{
	thread->window->hide_window();
}

int BlurMain::set_string()
{
	thread->window->set_title(gui_string);
}

int BlurMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sblur.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	vertical = defaults->get("VERTICAL", vertical);
	horizontal = defaults->get("HORIZONTAL", horizontal);
	radius = defaults->get("RADIUS", radius);
}

int BlurMain::save_defaults()
{
	defaults->update("VERTICAL", vertical);
	defaults->update("HORIZONTAL", horizontal);
	defaults->update("RADIUS", radius);
	defaults->save();
}

int BlurMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("BLUR");
	output.tag.set_property("VERTICAL", vertical);
	output.tag.set_property("HORIZONTAL", horizontal);
	output.tag.set_property("RADIUS", radius);
	output.append_tag();
	output.terminate_string();
// data is now in *text
}

int BlurMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("BLUR"))
			{
				vertical = input.tag.get_property("VERTICAL", vertical);
				horizontal = input.tag.get_property("HORIZONTAL", horizontal);
				radius = input.tag.get_property("RADIUS", radius);
			}
		}
	}
	if(thread) 
	{
		thread->window->vertical->update(vertical);
		thread->window->horizontal->update(horizontal);
		thread->window->radius->update(radius);
	}
	redo_buffers = 1;
}

BlurEngine::BlurEngine(BlurMain *plugin, int start_out, int end_out)
{
	int size = plugin->project_frame_w > plugin->project_frame_h ? plugin->project_frame_w : plugin->project_frame_h;
	this->plugin = plugin;
	this->start_out = start_out;
	this->end_out = end_out;
	last_frame = 0;
	val_p = new pixel_f[size];
	val_m = new pixel_f[size];
	src = new pixel_f[size];
	dst = new pixel_f[size];
	input_lock.lock();
	output_lock.lock();
}

BlurEngine::~BlurEngine()
{
	last_frame = 1;
	input_lock.unlock();
	join();
}

int BlurEngine::start_process_frame(VFrame **output, VFrame **input, int size)
{
	this->output = output;
	this->input = input;
	this->size = size;
	input_lock.unlock();
}

int BlurEngine::wait_process_frame()
{
	output_lock.lock();
}

void BlurEngine::run()
{
	register int i, j, k, l;
	VPixel **input_rows, **output_rows;
	int strip_size;

	while(1)
	{
		input_lock.lock();
		if(last_frame)
		{
			output_lock.unlock();
			return;
		}

		start_in = start_out - plugin->radius;
		end_in = end_out + plugin->radius;
		if(start_in < 0) start_in = 0;
		if(end_in > plugin->project_frame_h) end_in = plugin->project_frame_h;
		strip_size = end_in - start_in;
		for(i = 0; i < size; i++)
		{
			input_rows = (VPixel**)input[i]->get_rows();
			output_rows = (VPixel**)output[i]->get_rows();

			if(plugin->vertical)
			{
// Vertical pass
				for(j = 0; j < plugin->project_frame_w; j++)
				{
					for(l = 0, k = start_in; k < end_in; l++, k++)
					{
						val_p[l].r = val_p[l].g = val_p[l].b = val_p[l].a = 0;
						val_m[l].r = val_m[l].g = val_m[l].b = val_m[l].a = 0;
						src[l].r = (float)input_rows[k][j].r;
						src[l].g = (float)input_rows[k][j].g;
						src[l].b = (float)input_rows[k][j].b;
						src[l].a = (float)input_rows[k][j].a;
					}

					blur_strip(j, strip_size);

					for(l = start_out - start_in, k = start_out; k < end_out; l++, k++)
					{
						output_rows[k][j].r = (VWORD)dst[l].r;
						output_rows[k][j].g = (VWORD)dst[l].g;
						output_rows[k][j].b = (VWORD)dst[l].b;
						output_rows[k][j].a = (VWORD)dst[l].a;
					}
				}
			}

			if(plugin->horizontal)
			{
// Horizontal pass
				if(plugin->vertical) input_rows = output_rows;

				for(j = start_out; j < end_out; j++)
				{
					for(k = 0; k < plugin->project_frame_w; k++)
					{
						val_p[k].r = val_p[k].g = val_p[k].b = val_p[k].a = 0;
						val_m[k].r = val_m[k].g = val_m[k].b = val_m[k].a = 0;
						src[k].r = (float)input_rows[j][k].r;
						src[k].g = (float)input_rows[j][k].g;
						src[k].b = (float)input_rows[j][k].b;
						src[k].a = (float)input_rows[j][k].a;
					}

					blur_strip(j, plugin->project_frame_w);

					for(k = 0; k < plugin->project_frame_w; k++)
					{
						output_rows[j][k].r = (VWORD)dst[k].r;
						output_rows[j][k].g = (VWORD)dst[k].g;
						output_rows[j][k].b = (VWORD)dst[k].b;
						output_rows[j][k].a = (VWORD)dst[k].a;
					}
				}
			}
		}
		output_lock.unlock();
	}
}

int BlurEngine::reconfigure()
{
	std_dev = sqrt(-(double)(plugin->radius * plugin->radius) / (2 * log (1.0 / 255.0)));
	get_constants();
}

int BlurEngine::get_constants()
{
	int i;
	double constants[8];
	double div;

	div = sqrt(2 * M_PI) * std_dev;
	constants[0] = -1.783 / std_dev;
	constants[1] = -1.723 / std_dev;
	constants[2] = 0.6318 / std_dev;
	constants[3] = 1.997  / std_dev;
	constants[4] = 1.6803 / div;
	constants[5] = 3.735 / div;
	constants[6] = -0.6803 / div;
	constants[7] = -0.2598 / div;

	n_p[0] = constants[4] + constants[6];
	n_p[1] = exp(constants[1]) *
				(constants[7] * sin(constants[3]) -
				(constants[6] + 2 * constants[4]) * cos(constants[3])) +
				exp(constants[0]) *
				(constants[5] * sin(constants[2]) -
				(2 * constants[6] + constants[4]) * cos(constants[2]));

	n_p[2] = 2 * exp(constants[0] + constants[1]) *
				((constants[4] + constants[6]) * cos(constants[3]) * 
				cos(constants[2]) - constants[5] * 
				cos(constants[3]) * sin(constants[2]) -
				constants[7] * cos(constants[2]) * sin(constants[3])) +
				constants[6] * exp(2 * constants[0]) +
				constants[4] * exp(2 * constants[1]);

	n_p[3] = exp(constants[1] + 2 * constants[0]) *
				(constants[7] * sin(constants[3]) - 
				constants[6] * cos(constants[3])) +
				exp(constants[0] + 2 * constants[1]) *
				(constants[5] * sin(constants[2]) - constants[4] * 
				cos(constants[2]));
	n_p[4] = 0.0;

	d_p[0] = 0.0;
	d_p[1] = -2 * exp(constants[1]) * cos(constants[3]) -
				2 * exp(constants[0]) * cos(constants[2]);

	d_p[2] = 4 * cos(constants[3]) * cos(constants[2]) * 
				exp(constants[0] + constants[1]) +
				exp(2 * constants[1]) + exp (2 * constants[0]);

	d_p[3] = -2 * cos(constants[2]) * exp(constants[0] + 2 * constants[1]) -
				2 * cos(constants[3]) * exp(constants[1] + 2 * constants[0]);

	d_p[4] = exp(2 * constants[0] + 2 * constants[1]);

	for(i = 0; i < 5; i++) d_m[i] = d_p[i];

	n_m[0] = 0.0;
	for(i = 1; i <= 4; i++)
		n_m[i] = n_p[i] - d_p[i] * n_p[0];

	double sum_n_p, sum_n_m, sum_d;
	double a, b;

	sum_n_p = 0.0;
	sum_n_m = 0.0;
	sum_d = 0.0;
	for(i = 0; i < 5; i++)
	{
		sum_n_p += n_p[i];
		sum_n_m += n_m[i];
		sum_d += d_p[i];
	}

	a = sum_n_p / (1 + sum_d);
	b = sum_n_m / (1 + sum_d);

	for (i = 0; i < 5; i++)
	{
		bd_p[i] = d_p[i] * a;
		bd_m[i] = d_m[i] * b;
	}
}

#define BOUNDARY(x) if((x) > VMAX) (x) = VMAX; else if((x) < 0) (x) = 0;

int BlurEngine::transfer_pixels(pixel_f *src1, pixel_f *src2, pixel_f *dest, int size)
{
	register int i;
	register float sum;

	for(i = 0; i < size; i++)
    {
		sum = src1[i].r + src2[i].r;
		BOUNDARY(sum);
		dest[i].r = sum;
		sum = src1[i].g + src2[i].g;
		BOUNDARY(sum);
		dest[i].g = sum;
		sum = src1[i].b + src2[i].b;
		BOUNDARY(sum);
		dest[i].b = sum;
		sum = src1[i].a + src2[i].a;
		BOUNDARY(sum);
		dest[i].a = sum;
    }
}


int BlurEngine::multiply_alpha(pixel_f *row, int size)
{
	register int i;
	register float alpha;

	for(i = 0; i < size; i++)
	{
		alpha = (float)row[i].a / VMAX;
		row[i].r *= alpha;
		row[i].g *= alpha;
		row[i].b *= alpha;
	}
}

int BlurEngine::seperate_alpha(pixel_f *row, int size)
{
	register int i;
	register float alpha;
	register float result;
	
	for(i = 0; i < size; i++)
	{
		if(row[i].a > 0 && row[i].a < VMAX)
		{
			alpha = (float)row[i].a / VMAX;
			result = (float)row[i].r / alpha;
			row[i].r = (result > VMAX ? VMAX : result);
			result = (float)row[i].g / alpha;
			row[i].g = (result > VMAX ? VMAX : result);
			result = (float)row[i].b / alpha;
			row[i].b = (result > VMAX ? VMAX : result);
		}
	}
}

int BlurEngine::blur_strip(int &j, int &size)
{
	multiply_alpha(src, size);

	sp_p = src;
	sp_m = src + size - 1;
	vp = val_p;
	vm = val_m + size - 1;

	initial_p = sp_p[0];
	initial_m = sp_m[0];

	register int l;
	for(register int k = 0; k < size; k++)
	{
		terms = (k < 4) ? k : 4;
		for(l = 0; l <= terms; l++)
		{
			vp->r += n_p[l] * sp_p[-l].r - d_p[l] * vp[-l].r;
			vm->r += n_m[l] * sp_m[l].r - d_m[l] * vm[l].r;
			vp->g += n_p[l] * sp_p[-l].g - d_p[l] * vp[-l].g;
			vm->g += n_m[l] * sp_m[l].g - d_m[l] * vm[l].g;
			vp->b += n_p[l] * sp_p[-l].b - d_p[l] * vp[-l].b;
			vm->b += n_m[l] * sp_m[l].b - d_m[l] * vm[l].b;
			vp->a += n_p[l] * sp_p[-l].a - d_p[l] * vp[-l].a;
			vm->a += n_m[l] * sp_m[l].a - d_m[l] * vm[l].a;
		}
		for( ; l <= 4; l++)
		{
			vp->r += (n_p[l] - bd_p[l]) * initial_p.r;
			vm->r += (n_m[l] - bd_m[l]) * initial_m.r;
			vp->g += (n_p[l] - bd_p[l]) * initial_p.g;
			vm->g += (n_m[l] - bd_m[l]) * initial_m.g;
			vp->b += (n_p[l] - bd_p[l]) * initial_p.b;
			vm->b += (n_m[l] - bd_m[l]) * initial_m.b;
			vp->a += (n_p[l] - bd_p[l]) * initial_p.a;
			vm->a += (n_m[l] - bd_m[l]) * initial_m.a;
		}
		sp_p++;
		sp_m--;
		vp++;
		vm--;
	}
	transfer_pixels(val_p, val_m, dst, size);
	seperate_alpha(dst, size);
}

