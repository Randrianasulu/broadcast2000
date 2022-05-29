#include "filehtal.h"
#include "rotate.h"

#define SQR(x) ((x) * (x))

int main(int argc, char *argv[])
{
	RotateMain *plugin;

	plugin = new RotateMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

RotateMain::RotateMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	angle = 0;
	temp_frame = 0;
	engine = 0;
	thread = 0;
	int_matrix = 0;
	int_rows = 0;
	float_matrix = 0;
	float_rows = 0;
	last_angle = 0;
}

RotateMain::~RotateMain()
{
}

char* RotateMain::plugin_title() { return "Rotate"; }
int RotateMain::plugin_is_realtime() { return 1; }
int RotateMain::plugin_is_multi_channel() { return 0; }

int RotateMain::start_realtime()
{
	int y1, y2, y_increment;
	y_increment = project_frame_h / smp;
	y1 = 0;

	engine = new RotateEngine*[smp];
	for(int i = 0; i < smp; i++)
	{
		y2 = y1 + y_increment;
		if(i == smp - 1 && y2 < project_frame_h - 1) y2 = project_frame_h - 1;
		engine[i] = new RotateEngine(this, y1, y2);
		engine[i]->start();
		y1 += y_increment;
	}
return 0;
}

int RotateMain::stop_realtime()
{
	for(int i = 0; i < smp; i++)
	{
		delete engine[i];
	}
	delete engine;
return 0;
}

int RotateMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	register int i, j, k;
	VPixel **input_rows, **output_rows;
	int r, g, b;
	float old_angle = angle;

	for(i = 0; i < size; i++)
	{
		if(automation_used())
		{
			angle = old_angle + get_automation_value(i) * MAXANGLE;
			if(angle >= 360) angle -= 360;
			else
			if(angle < 0) angle += 360;
		}

		input_rows = (VPixel**)input_ptr[i]->get_rows();
		output_rows = (VPixel**)output_ptr[i]->get_rows();

		if(angle != 0)
		{
        	if(angle == 90 || angle == 180 || angle == 270)
            	rotate_rightangle((VPixel**)input_ptr[i]->get_rows(), (VPixel**)output_ptr[i]->get_rows(), (int)angle);
			else
			{
				rotate_obliqueangle((VPixel**)input_ptr[i]->get_rows(), output_ptr[i], (int)angle);
			}
		}
		else
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
		angle = old_angle;
	}
return 0;
}

int RotateMain::clear_unused(VPixel **input_rows, VPixel **output_rows, int out_x1, int out_y1, int out_x2, int out_y2)
{
	int i, j;
    for(i = 0; i < out_y1; i++)
    {
        for(j = 0; j < project_frame_w; j++)
        {
	        output_rows[i][j].r = output_rows[i][j].g = output_rows[i][j].b = output_rows[i][j].a = 0;
        }
    }
    for(i = out_y2; i < project_frame_h; i++)
    {
        for(j = 0; j < project_frame_w; j++)
        {
	        output_rows[i][j].r = output_rows[i][j].g = output_rows[i][j].b = output_rows[i][j].a = 0;
        }
    }
    for(i = out_y1; i < out_y2; i++)
    {
        for(j = 0; j < out_x1; j++)
        {
	        output_rows[i][j].r = output_rows[i][j].g = output_rows[i][j].b = output_rows[i][j].a = 0;
        }
    }
    for(i = out_y1; i < out_y2; i++)
    {
        for(j = out_x2; j < project_frame_w; j++)
        {
	        output_rows[i][j].r = output_rows[i][j].g = output_rows[i][j].b = output_rows[i][j].a = 0;
        }
    }
return 0;
}

int RotateMain::get_rightdimensions(int &diameter, int &in_x1, int &in_y1, int &in_x2, int &in_y2, int &out_x1, int &out_y1, int &out_x2, int &out_y2)
{
    diameter = project_frame_w < project_frame_h ? project_frame_w : project_frame_h;
    out_x1 = in_x1 = project_frame_w / 2 - diameter / 2;
    out_x2 = in_x2 = in_x1 + diameter - 1;
    out_y1 = in_y1 = project_frame_h / 2 - diameter / 2;
    out_y2 = in_y2 = in_y1 + diameter - 1;
return 0;
}

int RotateMain::rotate_rightangle(VPixel **input_rows, VPixel **output_rows, int angle)
{
	int i, j, k, l, m, n, o, p;
	int in_x1, in_y1, in_x2, in_y2, out_x1, out_y1, out_x2, out_y2;
	VPixel temp_pixel;
    int diameter;

	in_x1 = 0;
    in_y1 = 0;
    in_x2 = project_frame_w;
    in_y2 = project_frame_h;

	switch(angle)
    {
    	case 90:
			get_rightdimensions(diameter, in_x1, in_y1, in_x2, in_y2, out_x1, out_y1, out_x2, out_y2);
            while(in_x2 > in_x1)
            {
            	diameter = in_x2 - in_x1;
                for(i = 0; i < diameter; i++)
                {
                    temp_pixel = input_rows[in_y1 + i][in_x2];
                    output_rows[in_y1 + i][in_x2] = input_rows[in_y1][in_x1 + i];
                    output_rows[in_y1][in_x1 + i] = input_rows[in_y2 - i][in_x1];
                    output_rows[in_y2 - i][in_x1] = input_rows[in_y2][in_x2 - i];
                    output_rows[in_y2][in_x2 - i] = temp_pixel;
                }

                in_x2--;
                in_x1++;
                in_y2--;
                in_y1++;
            }
            clear_unused(input_rows, output_rows, out_x1, out_y1, out_x2, out_y2);
        	break;
        
        case 180:
        	for(i = 0, j = project_frame_h - 1; i < j; i++, j--)
            {
            	for(k = 0, l = project_frame_w - 1; k < project_frame_w; k++, l--)
                {
                	temp_pixel = input_rows[j][k];
                    output_rows[j][k] = input_rows[i][l];
                    output_rows[i][l] = temp_pixel;
                }
            }
        	break;
        
        case 270:
			get_rightdimensions(diameter, in_x1, in_y1, in_x2, in_y2, out_x1, out_y1, out_x2, out_y2);
            while(in_x2 > in_x1)
            {
            	diameter = in_x2 - in_x1;
                for(i = 0; i < diameter; i++)
                {
                    temp_pixel = input_rows[in_y1 + i][in_x1];
                    output_rows[in_y1 + i][in_x1] = input_rows[in_y1][in_x2 - i];
                    output_rows[in_y1][in_x2 - i] = input_rows[in_y2 - i][in_x2];
                    output_rows[in_y2 - i][in_x2] = input_rows[in_y2][in_x1 + i];
                    output_rows[in_y2][in_x1 + i] = temp_pixel;
                }

                in_x2--;
                in_x1++;
                in_y2--;
                in_y1++;
            }
            clear_unused(input_rows, output_rows, out_x1, out_y1, out_x2, out_y2);
        	break;
    }
return 0;
}

int RotateMain::rotate_obliqueangle(VPixel **input_rows, VFrame *output, int angle)
{
	int i;
	int cen_x, cen_y;
	
	cen_x = project_frame_w / 2;
	cen_y = project_frame_h / 2;

	if(!temp_frame) temp_frame = new VFrame(0, project_frame_w, project_frame_h);
	if(last_angle != angle || (use_interpolation && !float_matrix) || (!use_interpolation && !int_matrix))
	{
		if(use_interpolation && !float_matrix)
		{
			float_matrix = new SourceCoord[project_frame_w * project_frame_h];
			float_rows = new SourceCoord*[project_frame_h];
			for(i = 0; i < project_frame_h; i++)
			{
				float_rows[i] = &float_matrix[i * project_frame_w];
			}
		}
		else
		if(!use_interpolation && !int_matrix)
		{
			int_matrix = new int[project_frame_w * project_frame_h];
			int_rows = new int*[project_frame_h];
			for(i = 0; i < project_frame_h; i++)
			{
				int_rows[i] = &int_matrix[i * project_frame_w];
			}
		}

// Last angle != angle implied by first buffer needing to be allocated
		for(i = 0; i < smp; i++)
		{
			engine[i]->generate_matrix(use_interpolation);
		}

		for(i = 0; i < smp; i++)
		{
			engine[i]->wait_completion();
		}
	}
	last_angle = angle;

// Perform the rotation
	for(i = 0; i < smp; i++)
	{
		engine[i]->perform_rotation(input_rows, (VPixel**)temp_frame->get_rows(), use_interpolation);
	}

	for(i = 0; i < smp; i++)
	{
		engine[i]->wait_completion();
	}
	((VPixel**)temp_frame->get_rows())[cen_y][cen_x] = input_rows[cen_y][cen_x];
	output->copy_from(temp_frame);
return 0;
}

int RotateMain::start_gui()
{
	load_defaults();
	thread = new RotateThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int RotateMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int RotateMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int RotateMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int RotateMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int RotateMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%srotate.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	angle = defaults->get("ANGLE", (float)angle);
return 0;
}

int RotateMain::save_defaults()
{
	defaults->update("ANGLE", angle);
	defaults->save();
return 0;
}

int RotateMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("ROTATE");
	output.tag.set_property("ANGLE", angle);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int RotateMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("ROTATE"))
			{
				angle = input.tag.get_property("ANGLE", angle);
			}
		}
	}
	if(thread) 
	{
		thread->window->update_parameters();
	}
return 0;
}

RotateEngine::RotateEngine(RotateMain *plugin, int row1, int row2) : Thread()
{
	this->plugin = plugin;
	Thread::synchronous = 1;
	do_matrix = do_rotation = 0;
	done = 0;
	this->row1 = row1;
	this->row2 = row2;
	input_lock.lock();
	output_lock.lock();
}
RotateEngine::~RotateEngine()
{
	if(!done)
	{
		done = 1;
		input_lock.unlock();
		join();
	}
}

int RotateEngine::generate_matrix(int interpolate)
{
	this->do_matrix = 1;
	this->interpolate = interpolate;
	input_lock.unlock();
return 0;
}

int RotateEngine::perform_rotation(VPixel **input_rows, VPixel **output_rows, int interpolate)
{
	this->input_rows = input_rows;
	this->output_rows = output_rows;
	this->do_rotation = 1;
	this->interpolate = interpolate;
	input_lock.unlock();
return 0;
}


int RotateEngine::wait_completion()
{
	output_lock.lock();
return 0;
}

int RotateEngine::coords_to_pixel(int &input_y, int &input_x)
{
	if(input_y < 0) return -1;
	else
	if(input_y >= plugin->project_frame_h) return -1;
	else
	if(input_x < 0) return -1;
	else
	if(input_x >= plugin->project_frame_w) return -1;
	else
	return input_y * plugin->project_frame_w + input_x;
}

int RotateEngine::coords_to_pixel(SourceCoord &float_pixel, float &input_y, float &input_x)
{
	if(input_y < 0) float_pixel.y = -1;
	else
	if(input_y >= plugin->project_frame_h) float_pixel.y = -1;
	else
	float_pixel.y = input_y;

	if(input_x < 0) float_pixel.x = -1;
	else
	if(input_x >= plugin->project_frame_w) float_pixel.x = -1;
	else
	float_pixel.x = input_x;
return 0;
}


int RotateEngine::create_matrix()
{
// Polar coords of pixel
	double k, l, magnitude, angle, offset_angle, offset_angle2;
	double x_offset, y_offset;
	int i, j;
	int *int_row;
	SourceCoord *float_row;
	int input_x_i, input_y_i;
	float input_x_f, input_y_f;

//printf("RotateEngine::create_matrix 1\n");
// The following is the result of pure trial and error.
// Fix the angles
// The source pixels are seen as if they were rotated counterclockwise so the sign is OK.
	offset_angle = -(plugin->angle - 90) / 360 * 2 * M_PI;
	offset_angle2 = -(plugin->angle - 270) / 360 * 2 * M_PI;

// Calculate an offset to add to all the pixels to compensate for the quadrant
	y_offset = plugin->project_frame_h / 2;
	x_offset = plugin->project_frame_w / 2;

	for(i = row1, l = row1 - plugin->project_frame_h / 2; i < row2; i++, l++)
	{
		if(!interpolate)
			int_row = plugin->int_rows[i];
		else
			float_row = plugin->float_rows[i];

//printf("RotateEngine::create_matrix 2 %d %f\n", i, l);
		for(j = 0, k = -plugin->project_frame_w / 2; j < plugin->project_frame_w; j++, k++)
		{
// Add offsets to input
// Convert to polar coords
			magnitude = sqrt(SQR(k) + SQR(l));
//printf("RotateEngine::create_matrix 3.2 %f %f\n", k, l);
			if(l != 0)
				angle = atan(-k / l);
			else
			if(k < 0)
				angle = M_PI / 2;
			else
				angle = M_PI * 1.5;
//printf("RotateEngine::create_matrix 3.3\n");
// Rotate
			angle += (l < 0) ? offset_angle2 : offset_angle;

// Convert back to cartesian coords
			if(!interpolate)
			{
				input_y_i = (int)(y_offset + magnitude * sin(angle));
				input_x_i = (int)(x_offset + magnitude * cos(angle));
				int_row[j] = coords_to_pixel(input_y_i, input_x_i);
			}
			else
			{
				input_y_f = y_offset + magnitude * sin(angle);
				input_x_f = x_offset + magnitude * cos(angle);
				coords_to_pixel(float_row[j], input_y_f, input_x_f);
			}
		}
//printf("RotateEngine::create_matrix 3\n");
	}
//printf("RotateEngine::create_matrix 2\n");
return 0;
}

int RotateEngine::perform_rotation()
{
	VPixel zero_pixel = {0, 0, 0, 0};

	if(!interpolate)
	{
		int *int_row;
		register int i, j;
		for(i = row1; i < row2; i++)
		{
			int_row = plugin->int_rows[i];
			for(j = 0; j < plugin->project_frame_w; j++)
			{
				if(int_row[j] < 0) output_rows[i][j] = zero_pixel;
				else
				output_rows[i][j] = *(input_rows[0] + int_row[j]);
			}
		}
	}
	else
	{
		SourceCoord *float_row;
		register int i, j;
		register float k, l;
		register float x_fraction1, x_fraction2, y_fraction1, y_fraction2;
		register float fraction1, fraction2, fraction3, fraction4;
		register int x_pixel1, x_pixel2, y_pixel1, y_pixel2;
		register VPixel *pixel1, *pixel2, *pixel3, *pixel4;

		for(i = row1, k = row1; i < row2; i++, k++)
		{
			float_row = plugin->float_rows[i];
			for(j = 0, l = 0; j < plugin->project_frame_w; j++, l++)
			{
				if(float_row[j].x < 0 || float_row[j].y < 0) output_rows[i][j] = zero_pixel;
				else
				{
// Interpolate input pixels
					x_pixel1 = (int)float_row[j].x;
					x_pixel2 = (int)(float_row[j].x + 1);
					y_pixel1 = (int)(float_row[j].y);
					y_pixel2 = (int)(float_row[j].y + 1);
					x_fraction1 = float_row[j].x - x_pixel1;
					x_fraction2 = (float)x_pixel2 - float_row[j].x;
					y_fraction1 = float_row[j].y - y_pixel1;
					y_fraction2 = (float)y_pixel2 - float_row[j].y;
// By trial and error this fraction order seems to work.
					fraction4 = x_fraction1 * y_fraction1;
					fraction3 = x_fraction2 * y_fraction1;
					fraction2 = x_fraction1 * y_fraction2;
					fraction1 = x_fraction2 * y_fraction2;
					pixel1 = &input_rows[y_pixel1][x_pixel1];
					pixel2 = (x_pixel2 >= plugin->project_frame_w) ? &zero_pixel : &input_rows[y_pixel1][x_pixel2];
					pixel3 = (y_pixel2 >= plugin->project_frame_h) ? &zero_pixel : &input_rows[y_pixel2][x_pixel1];
					pixel4 = (x_pixel2 >= plugin->project_frame_w || y_pixel2 >= plugin->project_frame_h) ? &zero_pixel : &input_rows[y_pixel2][x_pixel2];

					output_rows[i][j].r = 
						(VWORD)((pixel1->r * fraction1) + 
							(pixel2->r * fraction2) + 
							(pixel3->r * fraction3) + 
							(pixel4->r * fraction4));
					output_rows[i][j].g = 
						(VWORD)((pixel1->g * fraction1) + 
							(pixel2->g * fraction2) + 
							(pixel3->g * fraction3) + 
							(pixel4->g * fraction4));
					output_rows[i][j].b = 
						(VWORD)((pixel1->b * fraction1) + 
							(pixel2->b * fraction2) + 
							(pixel3->b * fraction3) + 
							(pixel4->b * fraction4));
					output_rows[i][j].a = 
						(VWORD)((pixel1->a * fraction1) + 
							(pixel2->a * fraction2) + 
							(pixel3->a * fraction3) + 
							(pixel4->a * fraction4));
				}
			}
		}
	}
return 0;
}

void RotateEngine::run()
{
	while(!done)
	{
		input_lock.lock();
		if(done) return;

		if(do_matrix)
		{
			create_matrix();
		}
		else
		if(do_rotation)
		{
			perform_rotation();
		}

		do_matrix = 0;
		do_rotation = 0;
		output_lock.unlock();
	}
}
