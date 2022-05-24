#include "filehtal.h"
#include "whirl.h"
#include "whirlwindow.h"

#define MAX(x, y) ((x) > (y) ? (x) : (y))

int main(int argc, char *argv[])
{
	WhirlMain *plugin;

	plugin = new WhirlMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

WhirlMain::WhirlMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	automated_function = 0;
	angle = 0;
	pinch = 0;
	radius = 0;
	defaults = 0;
	reconfigure_flag = 0;
}

WhirlMain::~WhirlMain()
{
	if(defaults) delete defaults;
}

char* WhirlMain::plugin_title() { return "Whirl"; }
int WhirlMain::plugin_is_realtime() { return 1; }
int WhirlMain::plugin_is_multi_channel() { return 0; }
	
int WhirlMain::start_realtime()
{
	int y1, y2, y_increment;
	y_increment = project_frame_h / smp;
	y1 = 0;
	reconfigure_flag = 1;
	temp_frame = new VFrame(0, project_frame_w, project_frame_h);

// Only want half the rows processed
	engine = new WhirlEngine*[smp];
	for(int i = 0; i < smp; i++)
	{
		y2 = y1 + y_increment;
		if(i == smp - 1 && y2 < project_frame_h - 1) y2 = project_frame_h - 1;
		engine[i] = new WhirlEngine(this, y1, y2);
		engine[i]->start();
		y1 += y_increment;
	}
}

int WhirlMain::stop_realtime()
{
	for(int i = 0; i < smp; i++)
	{
		delete engine[i];
	}
	delete engine;
	delete temp_frame;
}


int WhirlMain::start_gui()
{
	load_defaults();
	thread = new WhirlThread(this);
	thread->start();
	thread->gui_started.lock();
}

int WhirlMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int WhirlMain::show_gui()
{
	thread->window->show_window();
}

int WhirlMain::hide_gui()
{
	thread->window->hide_window();
}

int WhirlMain::set_string()
{
	thread->window->set_title(gui_string);
}

int WhirlMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%swhirl.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	angle = defaults->get("ANGLE", 0);
	pinch = defaults->get("PINCH", 0);
	radius = defaults->get("RADIUS", 0);
	automated_function = defaults->get("AUTOMATION", automated_function);
}

int WhirlMain::save_defaults()
{
	defaults->update("ANGLE", angle);
	defaults->update("PINCH", pinch);
	defaults->update("RADIUS", radius);
	defaults->update("AUTOMATION", automated_function);
	defaults->save();
}

int WhirlMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("ANGLE");
	output.tag.set_property("VALUE", angle);
	output.append_tag();
	output.tag.set_title("PINCH");
	output.tag.set_property("VALUE", pinch);
	output.append_tag();
	output.tag.set_title("RADIUS");
	output.tag.set_property("VALUE", radius);
	output.append_tag();
	output.tag.set_title("AUTOMATED");
	output.tag.set_property("FUNCTION", automated_function);
	output.append_tag();
	output.terminate_string();
// data is now in *text
}

int WhirlMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("ANGLE"))
			{
				angle = input.tag.get_property("VALUE", angle);
			}
			else
			if(input.tag.title_is("PINCH"))
			{
				pinch = input.tag.get_property("VALUE", pinch);
			}
			else
			if(input.tag.title_is("RADIUS"))
			{
				radius = input.tag.get_property("VALUE", radius);
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
		thread->window->angle_slider->update(angle);
		thread->window->pinch_slider->update(pinch);
		thread->window->radius_slider->update(radius);
		for(int i = 0; i < 2; i++)
		{
			thread->window->automation[i]->update(automated_function == i);
		}
	}
}

int WhirlMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	int i;
	int old_angle = angle;
	int old_pinch = pinch;
	int old_radius = radius;

	if(automation_used())
	{
		switch(automated_function)
		{
			case 0:
				angle += (int)(get_automation_value(0) * MAXANGLE);
				break;
			case 1:
				pinch += (int)(get_automation_value(0) * MAXPINCH);
				break;
			case 2:
				radius += (int)(get_automation_value(0) * MAXRADIUS);
				break;
		}
		if(angle > MAXANGLE) angle = MAXANGLE;
		if(angle < -MAXANGLE) angle = -MAXANGLE;
		if(pinch > MAXPINCH) pinch = MAXPINCH;
		if(pinch < -MAXPINCH) pinch = -MAXPINCH;
		if(radius > MAXRADIUS) radius = MAXRADIUS;
		if(radius < 0) radius = -0;
	}
	
	if(old_angle != angle || old_pinch != pinch || old_radius != radius || reconfigure_flag)
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

	angle = old_angle;
	pinch = old_pinch;
	radius = old_radius;
	return 0;
}

int WhirlMain::reconfigure()
{
}

WhirlEngine::WhirlEngine(WhirlMain *plugin, int start_y, int end_y)
 : Thread()
{
	this->plugin = plugin;
	this->start_y = start_y;
	this->end_y = end_y;
	last_frame = 0;
	input_lock.lock();
	output_lock.lock();
}

WhirlEngine::~WhirlEngine()
{
	last_frame = 1;
	input_lock.unlock();
	join();
}

int WhirlEngine::start_process_frame(VFrame **output, VFrame **input, int size)
{
	this->output = output;
	this->input = input;
	this->size = size;
	input_lock.unlock();
}

int WhirlEngine::wait_process_frame()
{
	output_lock.lock();
}

int WhirlEngine::calc_undistorted_coords(double  wx,
			 double wy,
			 double &whirl,
			 double &pinch,
			 double &x,
			 double &y)
{
	double dx, dy;
	double d, factor;
	double dist;
	double ang, sina, cosa;
	int inside;

/* Distances to center, scaled */

	dx = (wx - cen_x) * scale_x;
	dy = (wy - cen_y) * scale_y;

/* Distance^2 to center of *circle* (scaled ellipse) */

	d = dx * dx + dy * dy;

/*  If we are inside circle, then distort.
 *  Else, just return the same position
 */

	inside = (d < radius2);

	if(inside)
    {
    	dist = sqrt(d / radius3) / radius;

/* Pinch */

    	factor = pow(sin(M_PI / 2 * dist), -pinch);

    	dx *= factor;
    	dy *= factor;

/* Whirl */

    	factor = 1.0 - dist;

    	ang = whirl * factor * factor;

    	sina = sin(ang);
    	cosa = cos(ang);

    	x = (cosa * dx - sina * dy) / scale_x + cen_x;
    	y = (sina * dx + cosa * dy) / scale_y + cen_y;
    }

	return inside;
}

void WhirlEngine::get_pixel(const int &x, const int &y, VPixel *pixel, VPixel **input_rows)
{
	if((x < 0) || (x >= plugin->project_frame_w) ||
    	(y < 0) || (y >= plugin->project_frame_h))
    {
        pixel->r = pixel->g = pixel->b = pixel->a = 0;
    }
	else
	*pixel = input_rows[y][x];
}

void WhirlEngine::run()
{
	int row, col;
	VPixel **input_rows;

	double whirl;
  	double cx, cy;
    int ix, iy;
	VPixel pixel[4];
	VWORD values[4];
	VPixel *top_row, *bot_row, *top_p, *bot_p;

	cen_x = (double)(plugin->project_frame_w - 1) / 2.0;
	cen_y = (double)(plugin->project_frame_h - 1) / 2.0;
	radius = MAX(plugin->project_frame_w, plugin->project_frame_h);

	if(plugin->project_frame_w < plugin->project_frame_h)
    {
    	scale_x = (double)plugin->project_frame_h / plugin->project_frame_w;
    	scale_y = 1.0;
    }
	else 
	if(plugin->project_frame_w > plugin->project_frame_h)
    {
    	scale_x = 1.0;
    	scale_y = (double)plugin->project_frame_w / plugin->project_frame_h;
    }
	else
    {
    	scale_x = 1.0;
    	scale_y = 1.0;
    }

	while(1)
	{
		input_lock.lock();
		if(last_frame)
		{
			output_lock.unlock();
			return;
		}

		input_rows = (VPixel**)input[0]->get_rows();

		whirl = plugin->angle * M_PI / 180;
		radius3 = (double)plugin->radius / MAXRADIUS;
  		radius2 = radius * radius * radius3;
		pinch = (double)plugin->pinch / MAXPINCH;

		for(row = start_y; row < (start_y + end_y) / 2; row++)
		{
			top_row = ((VPixel**)plugin->temp_frame->get_rows())[row];
			bot_row = ((VPixel**)plugin->temp_frame->get_rows())[plugin->project_frame_h - row - 1];
			top_p = top_row;
			bot_p = bot_row + plugin->project_frame_w - 1;

			for(col = 0; col < plugin->project_frame_w; col++)
			{
				if(calc_undistorted_coords(col, row, whirl, pinch, cx, cy))
				{
// Inside distortion area
// Do top
					if(cx >= 0.0)
						ix = (int)cx;
					else
						ix = -((int)-cx + 1);

					if(cy >= 0.0)
						iy = (int)cy;
					else
						iy = -((int)-cy + 1);

					get_pixel(ix, iy, &pixel[0], input_rows);
					get_pixel(ix + 1, iy, &pixel[1], input_rows);
					get_pixel(ix, iy + 1, &pixel[2], input_rows);
					get_pixel(ix + 1, iy + 1, &pixel[3], input_rows);

					values[0] = pixel[0].r;
					values[1] = pixel[1].r;
					values[2] = pixel[2].r;
					values[3] = pixel[3].r;
					top_p->r = bilinear(cx, cy, values);

					values[0] = pixel[0].g;
					values[1] = pixel[1].g;
					values[2] = pixel[2].g;
					values[3] = pixel[3].g;
					top_p->g = bilinear(cx, cy, values);

					values[0] = pixel[0].b;
					values[1] = pixel[1].b;
					values[2] = pixel[2].b;
					values[3] = pixel[3].b;
					top_p->b = bilinear(cx, cy, values);

					values[0] = pixel[0].a;
					values[1] = pixel[1].a;
					values[2] = pixel[2].a;
					values[3] = pixel[3].a;
					top_p->a = bilinear(cx, cy, values);
					top_p++;
// Do bottom
	    			cx = cen_x + (cen_x - cx);
	    			cy = cen_y + (cen_y - cy);

	    			if(cx >= 0.0)
						ix = (int)cx;
	    			else
						ix = -((int)-cx + 1);

	    			if(cy >= 0.0)
						iy = (int)cy;
	    			else
						iy = -((int)-cy + 1);

					get_pixel(ix, iy, &pixel[0], input_rows);
					get_pixel(ix + 1, iy, &pixel[1], input_rows);
					get_pixel(ix, iy + 1, &pixel[2], input_rows);
					get_pixel(ix + 1, iy + 1, &pixel[3], input_rows);

					values[0] = pixel[0].r;
					values[1] = pixel[1].r;
					values[2] = pixel[2].r;
					values[3] = pixel[3].r;
					bot_p->r = bilinear(cx, cy, values);

					values[0] = pixel[0].g;
					values[1] = pixel[1].g;
					values[2] = pixel[2].g;
					values[3] = pixel[3].g;
					bot_p->g = bilinear(cx, cy, values);

					values[0] = pixel[0].b;
					values[1] = pixel[1].b;
					values[2] = pixel[2].b;
					values[3] = pixel[3].b;
					bot_p->b = bilinear(cx, cy, values);

					values[0] = pixel[0].a;
					values[1] = pixel[1].a;
					values[2] = pixel[2].a;
					values[3] = pixel[3].a;
					bot_p->a = bilinear(cx, cy, values);
					bot_p--;
				}
				else
				{
// Outside distortion area
// Do top
					*top_p++ = input_rows[row][col];
// Do bottom
					*bot_p-- = input_rows[plugin->project_frame_h - 1 - row][plugin->project_frame_w - 1 - col];
				}
			}
		}

		output_lock.unlock();
	}
}
