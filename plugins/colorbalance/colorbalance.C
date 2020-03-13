#include "filehtal.h"
#include "colorbalance.h"

#define SQR(a) (a) * (a)

main(int argc, char *argv[])
{
	ColorBalanceMain *plugin;

	plugin = new ColorBalanceMain(argc, argv);
	plugin->plugin_run();
	delete plugin;
}

ColorBalanceMain::ColorBalanceMain(int argc, char *argv[])
 : PluginVClient(argc, argv)
{
	cyan = 0;
	magenta = 0;
	yellow = 0;
    redo_buffers = 0;
    preserve = 0;
    lock_params = 0;
}

ColorBalanceMain::~ColorBalanceMain()
{
	if(defaults) delete defaults;
}

char* ColorBalanceMain::plugin_title() { return "Color Balance"; }
int ColorBalanceMain::plugin_is_realtime() { return 1; }
int ColorBalanceMain::plugin_is_multi_channel() { return 0; }
	
int ColorBalanceMain::start_realtime()
{
	int i;

	for (i = 0; i < VMAX; i++)
    {
//		highlights_add[i] = (1.075 - 1 / ((double)i / (VMAX / 4) + 1));
		highlights_add[i] = highlights_sub[i] = 0.667 * (1 - SQR(((double)i - (VMAX / 2)) / (VMAX / 2)));
    }
    redo_buffers = 1;
}

int ColorBalanceMain::stop_realtime()
{
}

int ColorBalanceMain::reconfigure()
{
	int r_n, g_n, b_n;
    double *cyan_red_transfer;
    double *magenta_green_transfer;
    double *yellow_blue_transfer;

    cyan_red_transfer = cyan > 0 ? highlights_add : highlights_sub;
    magenta_green_transfer = magenta > 0 ? highlights_add : highlights_sub;
    yellow_blue_transfer = yellow > 0 ? highlights_add : highlights_sub;

	for(int i = 0; i < VMAX + 1; i++)
    {
    	r_n = g_n = b_n = i;
	    r_n += (int)(cyan / 100 * VMAX * cyan_red_transfer[r_n]);
	    g_n += (int)(magenta / 100 * VMAX  * magenta_green_transfer[g_n]);
	    b_n += (int)(yellow / 100 * VMAX  * yellow_blue_transfer[b_n]);

        if(r_n > VMAX) r_n = VMAX;
        if(r_n < 0) r_n = 0;
        if(g_n > VMAX) g_n = VMAX;
        if(g_n < 0) g_n = 0;
        if(b_n > VMAX) b_n = VMAX;
        if(b_n < 0) b_n = 0;

        r_lookup[i] = r_n;
        g_lookup[i] = g_n;
        b_lookup[i] = b_n;
    }
    redo_buffers = 0;
}

int ColorBalanceMain::test_boundary(float &value)
{
	if(value < -100) value = -100;
    if(value > 100) value = 100;
}

int ColorBalanceMain::synchronize_params(ColorBalanceSlider *slider, float difference)
{
	if(thread && lock_params)
    {
	    if(slider != thread->window->cyan)
        {
        	cyan += difference;
            test_boundary(cyan);
        	thread->window->cyan->update(cyan);
        }
	    if(slider != thread->window->magenta)
        {
        	magenta += difference;
            test_boundary(magenta);
        	thread->window->magenta->update(magenta);
        }
	    if(slider != thread->window->yellow)
        {
        	yellow += difference;
            test_boundary(yellow);
        	thread->window->yellow->update(yellow);
        }
    }
}

int ColorBalanceMain::process_realtime(long size, VFrame **input_ptr, VFrame **output_ptr)
{
	register int i, j, k;
	VPixel **input_rows, **output_rows;
	register int r, g, b, r_n, g_n, b_n;
    register float h, s, v, h_old, s_old, r_f, g_f, b_f;

	if(redo_buffers) reconfigure();

	for(i = 0; i < size; i++)
	{
		input_rows = (VPixel**)input_ptr[i]->get_rows();
		output_rows = (VPixel**)output_ptr[i]->get_rows();

		if(cyan != 0 || magenta != 0 || yellow != 0)
		{
#pragma omp parallel for schedule (static) private (j,k,r,g,b,r_n,g_n,b_n) collapse(2) num_threads(4)
			for(j = 0; j < project_frame_h; j++)
			{
				for(k = 0; k < project_frame_w; k++)
				{
                	r = input_rows[j][k].r;
                	g = input_rows[j][k].g;
                	b = input_rows[j][k].b;

                    r_n = r_lookup[r];
                    g_n = g_lookup[g];
                    b_n = b_lookup[b];

					if(preserve)
                    {
					    hsv.rgb_to_hsv((float)r_n, (float)g_n, (float)b_n, h, s, v);
					    hsv.rgb_to_hsv((float)r, (float)g, (float)b, h_old, s_old, v);
                        hsv.hsv_to_rgb(r_f, g_f, b_f, h, s, v);
                	    output_rows[j][k].r = (VWORD)r_f;
                	    output_rows[j][k].g = (VWORD)g_f;
                	    output_rows[j][k].b = (VWORD)b_f;
					}
                    else
                    {
                	    output_rows[j][k].r = r_n;
                	    output_rows[j][k].g = g_n;
                	    output_rows[j][k].b = b_n;
					}
				}
			}
		}
		else
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
}

int ColorBalanceMain::test_clip(int &r, int &g, int &b)
{
	if(r > VMAX) r = VMAX;
	else
	if(r < 0) r = 0;

	if(g > VMAX) g = VMAX;
	else
	if(g < 0) g = 0;

	if(b > VMAX) b = VMAX;
	else
	if(b < 0) b = 0;

	return 0;
}


int ColorBalanceMain::start_gui()
{
	load_defaults();
	thread = new ColorBalanceThread(this);
	thread->start();
	thread->gui_started.lock();
}

int ColorBalanceMain::stop_gui()
{
	save_defaults();
	thread->window->set_done(0);
	thread->join();
	delete thread;
	thread = 0;
}

int ColorBalanceMain::show_gui()
{
	thread->window->show_window();
}

int ColorBalanceMain::hide_gui()
{
	thread->window->hide_window();
}

int ColorBalanceMain::set_string()
{
	thread->window->set_title(gui_string);
}

int ColorBalanceMain::load_defaults()
{
	char directory[1024], string[1024];
// set the default directory
	sprintf(directory, "%scolorbalance.rc", BCASTDIR);

// load the defaults
	defaults = new Defaults(directory);
	defaults->load();

	cyan = defaults->get("CYAN", cyan);
	magenta = defaults->get("MAGENTA", magenta);
	yellow = defaults->get("YELLOW", yellow);
	preserve = defaults->get("PRESERVELUMINOSITY", preserve);
	lock_params = defaults->get("LOCKPARAMS", lock_params);
}

int ColorBalanceMain::save_defaults()
{
	defaults->update("CYAN", cyan);
	defaults->update("MAGENTA", magenta);
	defaults->update("YELLOW", yellow);
	defaults->update("PRESERVELUMINOSITY", preserve);
	defaults->update("LOCKPARAMS", lock_params);
	defaults->save();
}

int ColorBalanceMain::save_data(char *text)
{
	FileHTAL output;

// cause data to be stored directly in text
	output.set_shared_string(text, MESSAGESIZE);
	output.tag.set_title("COLORBALANCE");
	output.tag.set_property("CYAN", cyan);
	output.tag.set_property("MAGENTA", magenta);
	output.tag.set_property("YELLOW", yellow);
	output.tag.set_property("PRESERVELUMINOSITY", preserve);
	output.tag.set_property("LOCKPARAMS", lock_params);
	output.append_tag();
	output.terminate_string();
// data is now in *text
}

int ColorBalanceMain::read_data(char *text)
{
	FileHTAL input;

	input.set_shared_string(text, strlen(text));

	int result = 0;

	while(!result)
	{
		result = input.read_tag();

		if(!result)
		{
			if(input.tag.title_is("COLORBALANCE"))
			{
				cyan = input.tag.get_property("CYAN", cyan);
				magenta = input.tag.get_property("MAGENTA", magenta);
				yellow = input.tag.get_property("YELLOW", yellow);
				preserve = input.tag.get_property("PRESERVELUMINOSITY", preserve);
				lock_params = input.tag.get_property("LOCKPARAMS", lock_params);
			}
		}
	}
	if(thread) 
	{
		thread->window->cyan->update((int)cyan);
		thread->window->magenta->update((int)magenta);
		thread->window->yellow->update((int)yellow);
		thread->window->preserve->update(preserve);
        thread->window->lock_params->update(lock_params);
	}
    redo_buffers = 1;
}
