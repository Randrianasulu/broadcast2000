#include "filehtal.h"
#include "polar.h"
#include "polarwindow.h"

#define SQR(x) ((x) * (x))
#define WITHIN(a, b, c) ((((a) <= (b)) && ((b) <= (c))) ? 1 : 0)

int main(int argc, char *argv[])
{
	PolarMain *plugin;

	plugin = new PolarMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

PolarMain::PolarMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	automated_function = 0;
	depth = 0;
	angle = 0;
	backwards = 0;
	inverse = 1;
	polar_to_rectangular = 1;
	defaults = 0;
	reconfigure_flag = 0;
}

PolarMain::~PolarMain()
{
	if(defaults) delete defaults;
}

const char* PolarMain::plugin_title() { return "Polarize"; }
int PolarMain::plugin_is_realtime() { return 1; }
int PolarMain::plugin_is_multi_channel() { return 0; }
	
int PolarMain::start_realtime()
{
	int y1, y2, y_increment;
	y_increment = project_frame_h / smp;
	y1 = 0;
	reconfigure_flag = 1;

	temp_frame = new VFrame(0, project_frame_w, project_frame_h);
	engine = new PolarEngine*[smp];
	for(int i = 0; i < smp; i++)
	{
		y2 = y1 + y_increment;
		if(i == smp - 1 && y2 < project_frame_h - 1) y2 = project_frame_h - 1;
		engine[i] = new PolarEngine(this, y1, y2);
		engine[i]->start();
		y1 += y_increment;
	}
return 0;
}


int PolarMain::stop_realtime()
{
	for(int i = 0; i < smp; i++)
	{
		delete engine[i];
	}
	delete engine;
	delete temp_frame;
return 0;
}


int PolarMain::start_gui()
{
	load_defaults();
	thread = new PolarThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int PolarMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int PolarMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int PolarMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int PolarMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int PolarMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%spolarize.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	depth = defaults->get("DEPTH", 0);
	angle = defaults->get("ANGLE", 0);
	automated_function = defaults->get("AUTOMATION", automated_function);
return 0;
}

int PolarMain::save_defaults()
{
	defaults->update("DEPTH", depth);
	defaults->update("ANGLE", angle);
	defaults->update("AUTOMATION", automated_function);
	defaults->save();
return 0;
}

int PolarMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("DEPTH");
	output.tag.set_property("VALUE", depth);
	output.append_tag();
	output.tag.set_title("ANGLE");
	output.tag.set_property("VALUE", angle);
	output.append_tag();
	output.tag.set_title("AUTOMATED");
	output.tag.set_property("FUNCTION", automated_function);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int PolarMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("DEPTH"))
			{
				depth = input.tag.get_property("VALUE", depth);
			}
			else
			if(input.tag.title_is("ANGLE"))
			{
				angle = input.tag.get_property("VALUE", angle);
			}
			else
			if(input.tag.title_is("AUTOMATED"))
			{
				automated_function = input.tag.get_property("FUNCTION", automated_function);
			}
		}
	}
	if(thread) 
	{
		thread->window->depth_slider->update(depth);
		thread->window->angle_slider->update(angle);
		for(int i = 0; i < 2; i++)
		{
			thread->window->automation[i]->update(automated_function == i);
		}
	}
return 0;
}

int PolarMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	int i;
	int old_depth = depth;
	int old_angle = angle;

	if(automation_used())
	{
		switch(automated_function)
		{
			case 0:
				depth += (int)(get_automation_value(0) * MAXDEPTH);
				break;
			case 1:
				angle += (int)(get_automation_value(0) * MAXANGLE);
				break;
		}
		if(depth > MAXDEPTH) depth = MAXDEPTH;
		if(depth < 0) depth = 0;
		if(angle > MAXANGLE) angle = MAXANGLE;
		if(angle < 0) angle = 0;
	}
	
	if(old_depth != depth || old_angle != angle || reconfigure_flag)
		reconfigure();

	for(i = 0; i < smp; i++)
	{
		engine[i]->start_process_frame(output_ptr, input_ptr, size);
	}

	for(i = 0; i < smp; i++)
	{
		engine[i]->wait_process_frame();
	}

	output_ptr[0]->copy_from(temp_frame);

	depth = old_depth;
	angle = old_angle;
	return 0;
}

int PolarMain::reconfigure()
{
return 0;
}

PolarEngine::PolarEngine(PolarMain *plugin, int start_y, int end_y)
 : Thread()
{
	this->plugin = plugin;
	this->start_y = start_y;
	this->end_y = end_y;
	last_frame = 0;
	input_lock.lock();
	output_lock.lock();
}

PolarEngine::~PolarEngine()
{
	last_frame = 1;
	input_lock.unlock();
	join();
}

int PolarEngine::start_process_frame(VFrame **output, VFrame **input, int size)
{
	this->output = output;
	this->input = input;
	this->size = size;
	input_lock.unlock();
return 0;
}

int PolarEngine::wait_process_frame()
{
	output_lock.lock();
return 0;
}

int PolarEngine::calc_undistorted_coords(int wx,
			 int wy,
			 double &x,
			 double &y)
{
	int inside;
	double phi, phi2;
	double xx, xm, ym, yy;
	int xdiff, ydiff;
	double r;
	double m;
	double xmax, ymax, rmax;
	double x_calc, y_calc;
	double xi, yi;
	double circle, angl, t, angle;
	int x1, x2, y1, y2;

/* initialize */

	phi = 0.0;
	r = 0.0;

	x1 = 0;
	y1 = 0;
	x2 = plugin->project_frame_w;
	y2 = plugin->project_frame_h;
	xdiff = x2 - x1;
	ydiff = y2 - y1;
	xm = xdiff / 2.0;
	ym = ydiff / 2.0;
	circle = plugin->depth;
	angle = plugin->angle;
	angl = (double)angle / 180.0 * M_PI;

    if(plugin->polar_to_rectangular)
    {
        if(wx >= cen_x)
		{
	  		if(wy > cen_y)
	    	{
	      		phi = M_PI - atan(((double)(wx - cen_x)) / ((double)(wy - cen_y)));
	      		r   = sqrt(SQR(wx - cen_x) + SQR(wy - cen_y));
	    	}
	  		else 
			if(wy < cen_y)
	    	{
	    		phi = atan(((double)(wx - cen_x)) / ((double)(cen_y - wy)));
	    		r   = sqrt(SQR(wx - cen_x) + SQR(cen_y - wy));
	    	}
			else
	    	{
	    		phi = M_PI / 2;
	    		r   = wx - cen_x;
	    	}
		}
      	else 
		if(wx < cen_x)
		{
			if(wy < cen_y)
	    	{
	    		phi = 2 * M_PI - atan (((double)(cen_x -wx)) /
				    ((double)(cen_y - wy)));
	    		r   = sqrt(SQR(cen_x - wx) + SQR(cen_y - wy));
	    	}
			else 
			if(wy > cen_y)
	    	{
	    		phi = M_PI + atan(((double)(cen_x - wx)) / ((double)(wy - cen_y)));
	    		r   = sqrt(SQR(cen_x - wx) + SQR(wy - cen_y));
	    	}
			else
	    	{
	    		phi = 1.5 * M_PI;
	    		r   = cen_x - wx;
	    	}
		}
      	if (wx != cen_x)
		{
			m = fabs(((double)(wy - cen_y)) / ((double)(wx - cen_x)));
		}
    	else
		{
		    m = 0;
		}
    
    	if(m <= ((double)(y2 - y1) / (double)(x2 - x1)))
		{
			if(wx == cen_x)
	    	{
	    		xmax = 0;
	    		ymax = cen_y - y1;
	    	}
			else
	    	{
	    		xmax = cen_x - x1;
	    		ymax = m * xmax;
	    	}
		}
    	else
		{
			ymax = cen_y - y1;
			xmax = ymax / m;
		}
    
    	rmax = sqrt((double)(SQR(xmax) + SQR(ymax)));

    	t = ((cen_y - y1) < (cen_x - x1)) ? (cen_y - y1) : (cen_x - x1);
    	rmax = (rmax - t) / 100 * (100 - circle) + t;

    	phi = fmod(phi + angl, 2 * M_PI);

    	if(plugin->backwards)
			x_calc = x2 - 1 - (x2 - x1 - 1) / (2 * M_PI) * phi;
    	else
			x_calc = (x2 - x1 - 1) / (2 * M_PI) * phi + x1;

    	if(plugin->inverse)
			y_calc = (y2 - y1) / rmax * r + y1;
    	else
			y_calc = y2 - (y2 - y1) / rmax * r;

    	xi = (int)(x_calc + 0.5);
    	yi = (int)(y_calc + 0.5);
    
    	if(WITHIN(0, xi, plugin->project_frame_w - 1) && WITHIN(0, yi, plugin->project_frame_h - 1))
		{
			x = x_calc;
			y = y_calc;

			inside = 1;
		}
    	else
		{
			inside = 0;
		}
    }
	else
    {
    	if(plugin->backwards)
			phi = (2 * M_PI) * (x2 - wx) / xdiff;
    	else
			phi = (2 * M_PI) * (wx - x1) / xdiff;

    	phi = fmod (phi + angl, 2 * M_PI);
    
    	if(phi >= 1.5 * M_PI)
			phi2 = 2 * M_PI - phi;
    	else
		if (phi >= M_PI)
			phi2 = phi - M_PI;
		else
		if(phi >= 0.5 * M_PI)
	    	phi2 = M_PI - phi;
		else
	        phi2 = phi;

    	xx = tan (phi2);
    	if(xx != 0)
			m = (double)1.0 / xx;
    	else
			m = 0;
    
    	if(m <= ((double)(ydiff) / (double)(xdiff)))
		{
			if(phi2 == 0)
	    	{
	    		xmax = 0;
	    		ymax = ym - y1;
	    	}
			else
	    	{
	    		xmax = xm - x1;
	    		ymax = m * xmax;
	    	}
		}
    	else
		{
			ymax = ym - y1;
			xmax = ymax / m;
		}
    
    	rmax = sqrt((double)(SQR(xmax) + SQR(ymax)));
    
    	t = ((ym - y1) < (xm - x1)) ? (ym - y1) : (xm - x1);

    	rmax = (rmax - t) / 100.0 * (100 - circle) + t;

    	if(plugin->inverse)
			r = rmax * (double)((wy - y1) / (double)(ydiff));
    	else
			r = rmax * (double)((y2 - wy) / (double)(ydiff));
    
    	xx = r * sin (phi2);
    	yy = r * cos (phi2);

    	if(phi >= 1.5 * M_PI)
		{
			x_calc = (double)xm - xx;
			y_calc = (double)ym - yy;
		}
    	else
		if(phi >= M_PI)
		{
	    	x_calc = (double)xm - xx;
	    	y_calc = (double)ym + yy;
		}
		else
		if(phi >= 0.5 * M_PI)
	    {
	    	x_calc = (double)xm + xx;
	    	y_calc = (double)ym + yy;
	    }
		else
	    {
	    	x_calc = (double)xm + xx;
	    	y_calc = (double)ym - yy;
	    }

    	xi = (int)(x_calc + 0.5);
    	yi = (int)(y_calc + 0.5);
  
    	if(WITHIN(0, xi, plugin->project_frame_w - 1) && WITHIN(0, yi, plugin->project_frame_h - 1)) 
		{
			x = x_calc;
			y = y_calc;

			inside = 1;
    	}
    	else
		{
			inside = 0;
		}
    }
  
	return inside;
}

void PolarEngine::get_pixel(const int &x, const int &y, VPixel *pixel, VPixel **input_rows)
{
	if((x < 0) || (x >= plugin->project_frame_w) ||
    	(y < 0) || (y >= plugin->project_frame_h))
    {
        pixel->r = pixel->g = pixel->b = pixel->a = 0;
    }
	else
	*pixel = input_rows[y][x];
}

void PolarEngine::run()
{
	VPixel **input_rows, **output_rows;
	int x, y;
    VPixel pixel[4], *output_pixel;
	VWORD values[4];
	double cx, cy;

	cen_x = (double)(plugin->project_frame_w - 1) / 2.0;
	cen_y = (double)(plugin->project_frame_h - 1) / 2.0;

	while(1)
	{
		input_lock.lock();
		if(last_frame)
		{
			output_lock.unlock();
			return;
		}

		input_rows = (VPixel**)input[0]->get_rows();
		output_rows = (VPixel**)plugin->temp_frame->get_rows();
		for(y = start_y; y < end_y; y++)
		{
			for(x = 0; x < plugin->project_frame_w; x++)
			{
				output_pixel = &output_rows[y][x];
	      		if(calc_undistorted_coords(x, y, cx, cy))
				{
					get_pixel(cx, cy, &pixel[0], input_rows);
					get_pixel(cx + 1, cy, &pixel[1], input_rows);
					get_pixel(cx, cy + 1, &pixel[2], input_rows);
					get_pixel(cx + 1, cy + 1, &pixel[3], input_rows);
					
		    		values[0] = pixel[0].r;
		    		values[1] = pixel[1].r;
		    		values[2] = pixel[2].r;
		    		values[3] = pixel[3].r;
		    		output_pixel->r = bilinear(cx, cy, values);
					
		    		values[0] = pixel[0].g;
		    		values[1] = pixel[1].g;
		    		values[2] = pixel[2].g;
		    		values[3] = pixel[3].g;
		    		output_pixel->g = bilinear(cx, cy, values);
					
		    		values[0] = pixel[0].b;
		    		values[1] = pixel[1].b;
		    		values[2] = pixel[2].b;
		    		values[3] = pixel[3].b;
		    		output_pixel->b = bilinear(cx, cy, values);
					
		    		values[0] = pixel[0].a;
		    		values[1] = pixel[1].a;
		    		values[2] = pixel[2].a;
		    		values[3] = pixel[3].a;
		    		output_pixel->a = bilinear(cx, cy, values);
				}
				else
				{
					output_pixel->r = output_pixel->g = output_pixel->b = output_pixel->a = 0;
				}
			}
		}

		output_lock.unlock();
	}
}
