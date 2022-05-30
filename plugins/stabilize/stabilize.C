#include "filehtal.h"
#include "stabilize.h"
#include "stabilizewindow.h"

#define SQR(x) ((x) * (x))
#define INTENSITY(p) ((unsigned int)((p.r) * 77+ \
									(p.g * 150) + \
									(p.b * 29)) >> 8)

int main(int argc, char *argv[])
{
	StabilizeMain *plugin;

	plugin = new StabilizeMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

StabilizeMain::StabilizeMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	range = 64;
	size = 32;
	block = 0;
	redo_buffers = 0;
	accel = 256;
	engine = 0;
}

StabilizeMain::~StabilizeMain()
{
	if(defaults) delete defaults;
	if(block) delete block;
	block = 0;
}

char* StabilizeMain::plugin_title() { return "Stabilize"; }
int StabilizeMain::plugin_is_realtime() { return 1; }
int StabilizeMain::plugin_is_multi_channel() { return 0; }
	
int StabilizeMain::start_realtime()
{
	engine = new StabilizeEngine*[smp];
	for(int i = 0; i < smp; i++)
	{
		engine[i] = new StabilizeEngine(this);
		engine[i]->start();
	}

	x_offset = 0;
	y_offset = 0;
	block = 0;
	redo_buffers = 1;
return 0;
}

int StabilizeMain::stop_realtime()
{
	for(int i = 0; i < smp; i++)
	{
		delete engine[i];
	}
	delete engine;
	if(block) delete block;
	block = 0;
	redo_buffers = 0;
return 0;
}

int StabilizeMain::process_realtime(long frames, VFrame **input_ptr, VFrame **output_ptr)
{
	int i, old_accel;
	VPixel **input_rows, **output_rows;

	for(i = 0; i < frames; i++)
	{
		input_rows = (VPixel**)input_ptr[i]->get_rows();
		output_rows = (VPixel**)output_ptr[i]->get_rows();

		if(redo_buffers)
		{
			if(block) delete block;
			block = 0;
			redo_buffers = 0;
			
// Configure engines
			int y1, y2, y_increment;
			y_increment = range * 2 / smp;
			y1 = -range;

			for(int i = 0; i < smp; i++)
			{
				y2 = y1 + y_increment;
				if(i == smp - 1 && y2 < range) y2 = range;
				engine[i]->y1 = y1;
				engine[i]->y2 = y2;
				y1 += y_increment;
			}
		}

		if(range == 0 || size == 0)
		{
// Data never processed so copy if necessary
			if(!buffers_identical(0))
			{
				output_ptr[i]->copy_from(input_ptr[i]);
			}
		}
		else
		{
			if(size > project_frame_w / 2 || size > project_frame_h / 2)
				size = project_frame_w < project_frame_h ? project_frame_w / 2 : project_frame_h / 2;

			block_x = project_frame_w / 2 - size / 2;
			block_y = project_frame_h / 2 - size / 2;
			if(!block)
			{
// This frame defines the old block and doesn't look for motion
				block = new VFrame(0, size, size);
			}
			else
			{
// Get the motion vector for the current frame
				get_vector(block_x, block_y, block, input_ptr[i], x_offset, y_offset);
			}

// Read the new block before adjusting
			read_block(block_x, block_y, block, output_ptr[i]);

			old_accel = accel;
			if(automation_used())
			{
				accel += (int)(get_automation_value(i) * MAXACCEL);
				if(accel < 0) accel = 0;
			}

//printf("StabilizeMain %d %d\n", x_offset, y_offset);
			if(x_offset == 0 && y_offset == 0)
			{
				if(!buffers_identical(0))
				{
					output_ptr[i]->copy_from(input_ptr[i]);
				}
			}
			else
// Shift the entire frame
				offset_frame(input_ptr[i], output_ptr[i], -x_offset, -y_offset);

			accel = old_accel;
		}
	}
return 0;
}

int StabilizeMain::get_vector(int block_x, int block_y, VFrame *block, VFrame *input, int &x_offset, int &y_offset)
{
    int result;
    int new_x = 0, new_y = 0; // New offsets
	long score;
	double magnitude, angle;
	int x_sign, y_sign;

// Search for new location of old block in new frame relative to block_x and block_y.
// This automatically stays in the boundaries and prevents undoing the previous shift.
// Don't want to use results from blocks which have no details to compare.
//	score = three_step_search(block_x, block_y, block, input, new_x, new_y);
	if(block_randomness > VMAX / 10)
		score = exhaustive_search(block_x, block_y, block, input, new_x, new_y);

// Bias towards the center
	if(x_offset != 0 || y_offset != 0)
	{
		recenter(x_offset, y_offset);
	}

	x_sign = (new_x < 0) ? -1 : 1;
	y_sign = (new_y < 0) ? -1 : 1;

// Adjust acceleration
	magnitude = sqrt(SQR(new_x) + SQR(new_y));
	if(new_x != 0)
	{
		angle = atan(fabs((float)new_y / new_x));
	}

	if(magnitude > accel) magnitude = accel;
	
	if(new_x != 0)
	{
		new_y = (int)(y_sign * sin(angle) * magnitude);
		new_x = (int)(x_sign * cos(angle) * magnitude);
	}
	else
	{
		new_x = 0;
		new_y = (int)(y_sign * magnitude);
	}

	x_offset += new_x;
	y_offset += new_y;
	
	if(x_offset < -range) x_offset = -range;
	if(y_offset < -range) y_offset = -range;
	if(x_offset > range) x_offset = range;
	if(y_offset > range) y_offset = range;
return 0;
}

int StabilizeMain::recenter(int &new_x, int &new_y)
{
	double magnitude, angle;
	int x_sign, y_sign;

	x_sign = (new_x < 0) ? -1 : 1;
	y_sign = (new_y < 0) ? -1 : 1;

	magnitude = sqrt(SQR(new_x) + SQR(new_y));
	if(new_x != 0)
		angle = atan(fabs((float)new_y / new_x));

//printf("StabilizeMain::recenter %f %d %d\n", angle, new_x, new_y);
	magnitude--;
	if(new_x != 0)
	{
		new_y = (int)(y_sign * sin(angle) * magnitude);
		new_x = (int)(x_sign * cos(angle) * magnitude);
	}
	else
	{
		new_x = 0;
		new_y = (int)(y_sign * magnitude);
	}
return 0;
}

// int StabilizeMain::three_step_search(int block_x, int block_y, VFrame *block, VFrame *input, int &x_offset, int &y_offset)
// {
// 	int step_size;
// 	int x, y, center_x, center_y;
// 	long lowest_score = 0, score;
// 	int lowest_x, lowest_y;
// 
// 	center_x = block_x;
// 	center_y = block_y;
// 	for(step_size = range / 2; step_size > 0; )
// 	{
// 		for(x = center_x - step_size; x <= center_x + step_size; x += step_size)
// 		{
// 			for(y = center_y - step_size; y <= center_y + step_size; y += step_size)
// 			{
// 				if(x > 0 && y > 0 && x + size < project_frame_w && y + size < project_frame_h)
// 				{
// 					score = compare_blocks(x, y, block, input);
// 					if(score < lowest_score || lowest_score == 0)
// 					{
// 						lowest_x = x;
// 						lowest_y = y;
// 						lowest_score = score;
// 					}
// 				}
// 			}
// 		}
// 
// 		center_x = lowest_x;
// 		center_y = lowest_y;
// 		if(step_size > 1) step_size /= 2;
// 		else
// 		step_size = 0;
// 	}
// 
// 	if(lowest_score != 0)
// 	{
// 		x_offset = lowest_x - block_x;
// 		y_offset = lowest_y - block_y;
// 	}
// 	return lowest_score;
// }


int StabilizeMain::exhaustive_search(int block_x, int block_y, VFrame *block, VFrame *input, int &x_offset, int &y_offset)
{
	long score, lowest_score=0, i;
	int x, y;

	for(i = 0; i < smp; i++)
	{
		engine[i]->exhaustive_search(block_x, block_y, (VPixel**)block->get_rows(), (VPixel**)input->get_rows());
	}

	for(i = 0; i < smp; i++)
	{
		engine[i]->wait_completion(score, x, y);
		if(i < 1 || score < lowest_score)
		{
			lowest_score = score;
			x_offset = x;
			y_offset = y;
		}
	}

	return lowest_score;
}

int StabilizeMain::read_block(int x, int y, VFrame *block, VFrame *input)
{
	register int in_x, in_y, out_x, out_y;
	register int max = -1, min = -1;
    VPixel **in_rows, **out_rows;
	VPixel *in_row, *out_row;
	register int intensity;
	block_randomness = 0;

	out_rows = (VPixel**)block->get_rows();
	in_rows = (VPixel**)input->get_rows();
    for(out_y = 0, in_y = y; out_y < size; out_y++, in_y++)
    {
		in_row = &in_rows[in_y][x];
		out_row = out_rows[out_y];
	    for(out_x = 0, in_x = 0; out_x < size; out_x++, in_x++)
        {
        	out_row[out_x] = in_row[in_x];
			intensity = INTENSITY(in_row[in_x]);
			if(max < 0)
			{
				max = intensity;
				min = intensity;
			}
			else
			{
				if(intensity > max) max = intensity;
				if(intensity < min) min = intensity;
			}
        }
    }
	block_randomness = labs(max - min);
//printf("StabilizeMain::read_block %d\n", block_randomness);
return 0;
}


// Straight from destabilize
int StabilizeMain::clear_row(VPixel *row)
{
	for(int i = 0; i < project_frame_w; i++)
		row[i].r = row[i].g = row[i].b = row[i].a = 0;
return 0;
}

int StabilizeMain::offset_frame(VFrame *in, VFrame *out, int x_offset, int y_offset)
{
	int i, j, k, l;
	VPixel **in_rows = (VPixel**)in->get_rows();
	VPixel **out_rows = (VPixel**)out->get_rows();

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
	else
	{
		if(in_rows[0] != out_rows[0]) out->copy_from(in);
	}
return 0;
}


int StabilizeMain::start_gui()
{
	load_defaults();
	thread = new StabilizeThread(this);
	thread->start();
	thread->gui_started.lock();
return 0;
}

int StabilizeMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
return 0;
}

int StabilizeMain::show_gui()
{
	thread->window->show_window();
return 0;
}

int StabilizeMain::hide_gui()
{
	thread->window->hide_window();
return 0;
}

int StabilizeMain::set_string()
{
	thread->window->set_title(gui_string);
return 0;
}

int StabilizeMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%sstabilize.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	range = defaults->get("RANGE", range);
	size = defaults->get("SIZE", size);
	accel = defaults->get("ACCEL", accel);
return 0;
}

int StabilizeMain::save_defaults()
{
	defaults->update("RANGE", range);
	defaults->update("SIZE", size);
	defaults->update("ACCEL", accel);
	defaults->save();
return 0;
}

int StabilizeMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("STABILIZE");
	output.tag.set_property("RANGE", range);
	output.tag.set_property("SIZE", size);
	output.tag.set_property("ACCEL", accel);
	output.append_tag();
	output.terminate_string();
// data is now in *text
return 0;
}

int StabilizeMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("STABILIZE"))
			{
				range = input.tag.get_property("RANGE", range);
				size = input.tag.get_property("SIZE", size);
				accel = input.tag.get_property("ACCEL", accel);
			}
		}
	}
	if(thread) 
	{
		thread->window->range->update(range);
		thread->window->size->update(size);
		thread->window->accel->update(accel);
	}
	redo_buffers = 1;
return 0;
}




StabilizeEngine::StabilizeEngine(StabilizeMain *plugin)
 : Thread()
{
	this->plugin = plugin;
	Thread::synchronous = 1;
	done = 0;
	input_lock.lock();
	output_lock.lock();
}
StabilizeEngine::~StabilizeEngine()
{
	if(!done)
	{
		done = 1;
		input_lock.unlock();
		join();
	}
}

int StabilizeEngine::exhaustive_search(int block_x, int block_y, VPixel **block_rows, VPixel **input_rows)
{
	this->block_rows = block_rows;
	this->input_rows = input_rows;
	this->block_x = block_x;
	this->block_y = block_y;
	input_lock.unlock();
return 0;
}

int StabilizeEngine::wait_completion(long &score, int &x_offset, int &y_offset)
{
	output_lock.lock();
	score = this->score;
	x_offset = this->x_offset;
	y_offset = this->y_offset;
return 0;
}

long StabilizeEngine::compare_blocks(int x, int y, VPixel **block_rows, VPixel **input_rows)
{
	VPixel *block_row, *frame_row;
	register int block_x, block_y, frame_x, frame_y;
    register long difference;
    register long result = 0;

	for(block_y = 0, frame_y = y ; block_y < plugin->size; block_y++, frame_y++)
    {
		block_row = block_rows[block_y];
		frame_row = input_rows[frame_y];
    	for(block_x = 0, frame_x = x; block_x < plugin->size; block_x++, frame_x++)
        {
			difference = INTENSITY(block_row[block_x]) - INTENSITY(frame_row[frame_x]);
//			difference = (int)block_row[block_x].red - (int)frame_row[frame_x].red;
			if(difference < 0) difference *= -1;
			result += difference;
        }
    }

    return result;
}

void StabilizeEngine::run()
{
	while(!done)
	{
		input_lock.lock();
		if(done) return;

		int x, y, center_x, center_y;
		long lowest_score = 0, score;
		int lowest_x, lowest_y;

		center_x = block_x;
		center_y = block_y;
		x_offset = 0;
		y_offset = 0;
		for(y = center_y + y1; y <= center_y + y2; y++)
		{
			for(x = center_x - plugin->range; x <= center_x + plugin->range; x++)
			{
				if(x > 0 && y > 0 && x + plugin->size < plugin->project_frame_w && y + plugin->size < plugin->project_frame_h)
				{
					score = compare_blocks(x, y, block_rows, input_rows);
					if(score < lowest_score || lowest_score == 0)
					{
						lowest_x = x;
						lowest_y = y;
						lowest_score = score;
					}
				}
			}
		}

		if(lowest_score != 0)
		{
			x_offset = lowest_x - block_x;
			y_offset = lowest_y - block_y;
		}
		this->score = lowest_score;

		output_lock.unlock();
	}
}

