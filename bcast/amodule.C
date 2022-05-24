#include <string.h>
#include "amodule.h"
#include "aplugin.h"
#include "console.h"
#include "filehtal.h"
#include "floatautos.h"
#include "mainwindow.h"
#include "module.h"
#include "modules.h"
#include "patch.h"
#include "plugin.h"
#include "preferences.h"
#include "sharedpluginlocation.h"

AModule::AModule(MainWindow *mwindow)
 : Module(mwindow)
{
	peak_history = 0;
	data_type = TRACK_AUDIO;
	module_height = AMODULEHEIGHT;
	module_width = AMODULEWIDTH;
	gui = 0;
}

AModule::~AModule()
{
	for(int i = 0; i < PLUGINS; i++)
	{
		delete (APlugin*)plugins[i];
	}

// delete subwindow last
	if(gui) delete gui;
}

int AModule::create_objects(int pixel)
{
// get the variables
	int i, x=0, y=0;
	inv = 0;
	fade = 0;
	mute = 0;
	strcpy(title, get_patch_of()->title);
	this->pixel = pixel;

// get alternating values for pans
	int interval = modules->number_of_audio(this);

	while(interval >= mwindow->output_channels) interval -= mwindow->output_channels;

	for(i = 0; i < mwindow->output_channels; i++)
	{
		if(i != interval) pan[i] = 0;
		else
		pan[i] = 1;
	}
	pan_x = pan_y = 0;

// build the GUI if necessary
	if(mwindow->gui)
	{
		if(console->vertical)
		console->gui->add_subwindow(gui = new AModuleGUI(mwindow, this, pixel, 0, AMODULEWIDTH, console->gui->get_h() - 17));
		else
		console->gui->add_subwindow(gui = new AModuleGUI(mwindow, this, 0, pixel, console->gui->get_w() - 17, AMODULEHEIGHT));
	}
	else
	create_plugins(x, y);
return 0;
}

int AModule::create_plugins(int &x, int &y)
{
	for(int i = 0; i < PLUGINS; i++)
	{
		plugins[i] = new APlugin(mwindow, this, i+1);
		((APlugin*)plugins[i])->create_objects(x, y);
		y += 44;

		if(!console->vertical && y + 44 > AMODULEHEIGHT) 
		{
			y = 3;
			x += AMODULEWIDTH - 3;
		}
	}
	if(!console->vertical && y > 3) { x += AMODULEWIDTH; y = 3; }
return 0;
}

int AModule::flip_plugins(int &x, int &y)
{
	if(mwindow->gui)
	{
		for(int i = 0; i < PLUGINS; i++)
		{
			((APlugin*)plugins[i])->resize_plugin(x, y);
			y += 44;
			if(!console->vertical && y + 44 > AMODULEHEIGHT)
			{
				y = 3;
				x += AMODULEWIDTH - 3;
			}
		}
		if(!console->vertical && y > 3) { x += AMODULEWIDTH; y = 3; }
	}
return 0;
}

int AModule::set_pixel(int pixel)
{
	if(mwindow->gui)
	{
		this->pixel = pixel;

		if(console->vertical)
		gui->resize_window(pixel, 0, gui->get_w(), gui->get_h());
		else
		gui->resize_window(0, pixel, gui->get_w(), gui->get_h());
	}
return 0;
}








// ================================ file operations

int AModule::save(FileHTAL *htal)
{
	htal->tag.set_title("MODULE");
	htal->append_tag();
	htal->append_newline();


// title must come before plugins to allow plugins to boot
	htal->tag.set_title("TITLE");
	htal->append_tag();
	htal->append_text(title);
	htal->tag.set_title("/TITLE");
	htal->append_tag();
	htal->append_newline();

	int i;
	
	if(inv)
	{
		htal->tag.set_title("INV");
		htal->append_tag();
		htal->append_newline();
	}
	
	if(mute)
	{
		htal->tag.set_title("MUTE");
		htal->append_tag();
		htal->append_newline();
	}

	for(i = 0; i < PLUGINS; i++) plugins[i]->save(htal, "PLUGIN");
	
	htal->tag.set_title("PAN");
	htal->tag.set_property("X", (long)pan_x);
	htal->tag.set_property("Y", (long)pan_y);
	for(i = 0; i < mwindow->output_channels; i++)
	{
		sprintf(string, "PAN%d", i);
		htal->tag.set_property(string, pan[i]);
	}

	htal->append_tag();
	
	htal->tag.set_title("FADE");
	htal->tag.set_property("VALUE", fade);
	htal->append_tag();
	htal->append_newline();
	
	htal->tag.set_title("/MODULE");
	htal->append_tag();
	htal->append_newline();
return 0;
}

int AModule::load(FileHTAL *htal, int track_offset)
{
	int result = 0;
	int current_pan = 0;
	int current_plugin = 0;
	int i;
	DB db;

//printf("AModule::load 1\n");
	do{
		result = htal->read_tag();
		
		if(!result)
		{
			if(!strcmp(htal->tag.get_title(), "/MODULE"))
			{
				result = 1;
			}
			else
			if(!strcmp(htal->tag.get_title(), "PLUGIN"))
			{
// title must come before plugins to allow plugins to boot
				if(current_plugin < PLUGINS)
				{
					plugins[current_plugin]->load(htal, track_offset, "/PLUGIN");
					current_plugin++;
				}
			}
			else
			if(!strcmp(htal->tag.get_title(), "TITLE"))
			{
// title must come before plugins to allow plugins to boot properly
				if(gui) 
				{
					strcpy(title, htal->read_text());
					gui->title->update(title);
				}
			}
			else
			if(!strcmp(htal->tag.get_title(), "INV"))
			{
				inv = 1;
				if(gui) gui->inv_toggle->update(inv);
			}
			else
			if(!strcmp(htal->tag.get_title(), "MUTE"))
			{
				mute = 1;
				if(gui) gui->mute_toggle->update(mute);
			}
			else
			if(!strcmp(htal->tag.get_title(), "PAN"))
			{
				pan_x = htal->tag.get_property("X", (long)0);
				pan_y = htal->tag.get_property("Y", (long)0);

				for(i = 0; i < mwindow->output_channels; i++)
				{
					sprintf(string, "PAN%d", i);
					pan[i] = htal->tag.get_property(string, (float)0);
//printf("AModule::load pan%d %f\n", i, pan[i]);
				}

				if(gui)
				{
					gui->pan_stick->update(pan_x, pan_y);
				}
			}
			else
			if(!strcmp(htal->tag.get_title(), "FADE"))
			{
				fade = htal->tag.get_property("VALUE", (float)0);
				if(gui) gui->fade_slider->update(fade);
			}
		}
	}while(!result);
//printf("AModule::load 2\n");
return 0;
}

int AModule::dump()
{
	printf("AModule::dump %x\n", this);
	printf("	pan0 %f pan1 %f\n", pan[0], pan[1]);
return 0;
}

int AModule::update_peak(int peak_number, float value)
{
	peak_history[peak_number] = value;
return 0;
}

int AModule::update_meter(int peak_number, int last_peak, int total_peaks)
{
	if(peak_history && gui)
	{
// zero unused peaks
		for(int i = last_peak; i != peak_number; )
		{
			peak_history[i] = 0;
			i++;
			if(i >= total_peaks) i = 0;
		}

		gui->meter->update(peak_history[peak_number], peak_history[peak_number] > 1);
	}
return 0;
}

int AModule::init_meters(int total_peaks)
{
	peak_history = new float[total_peaks];
	
	for(int i = 0; i < total_peaks; i++)
	{
		peak_history[i] = 0;
	}
return 0;
}

int AModule::stop_meters()
{
	delete [] peak_history;
	peak_history = 0;
	if(gui) gui->meter->reset();
return 0;
}

int AModule::set_title(char *text)
{
	strcpy(title, text);
	if(gui) gui->title->update(text);

	for(int i = 0; i < PLUGINS; i++)
	{
		((APlugin*)plugins[i])->set_string();
	}
return 0;
}

int AModule::change_format() { if(gui) gui->meter->change_format(mwindow->preferences->meter_format); return 0;
}

int AModule::flip_vertical(int pixel) { if(gui) gui->flip_vertical(pixel); return 0;
}

int AModule::change_x(int new_pixel) { gui->change_x(new_pixel); return 0;
}

int AModule::change_y(int new_pixel) { gui->change_y(new_pixel); return 0;
}

int AModule::reset_meter() { gui->meter->reset_over(); return 0;
}

int AModule::change_channels(int new_channels, int *value_positions)
{
	if(gui)
	{
		gui->pan_stick->change_channels(new_channels, value_positions);
		gui->pan_stick->get_values();
		for(int i = 0; i < gui->pan_stick->get_total_values(); i++)
			pan[i] = gui->pan_stick->get_value(i);
	}

//	for(int i = mwindow->output_channels; i < new_channels; i++)
//	{
//		pan[i] = 0;
//	}
return 0;
}

int AModule::set_mute(int value)
{
	mute = value;
	if(gui) gui->mute_toggle->update(value);
return 0;
}








AModuleGUI::AModuleGUI(MainWindow *mwindow, AModule *module, int x, int y, int w, int h)
 : BC_SubWindow(x, y, w, h, MEGREY)
{
	this->mwindow = mwindow;
	this->console = mwindow->console;
	this->module = module;
}

AModuleGUI::~AModuleGUI()
{
	delete title;
	delete inv_toggle;
	delete fade_slider;
	delete pan_stick;
	delete meter;
}

int AModuleGUI::create_objects()
{
	add_border();
	
	if(console->vertical)
	{
		int x, y;
// ================================== title

		x = 3; y = 3;
		add_tool(this->title = new AModuleTitle(module, module->get_patch_of(), x, y));

// =================================== inv

		y += 22;
		add_tool(inv_toggle = new AModuleInv(mwindow->console, module, x + 32, y));
		add_tool(inv_title = new BC_Title(x + 52, y+2, "Inv", SMALLFONT));

		y += 20;
		module->create_plugins(x, y);

// ================================== pans

		add_tool(pan_stick = new AModulePan(module, x, y, 30, 150, 1, mwindow->output_channels, mwindow->channel_positions, module->pan));

// ================================ fader

		add_tool(fade_title = new BC_Title(70, y - 3, "Fade", SMALLFONT));
		y += 10;
		add_tool(fade_slider = new AModuleFade(module, 70, y, 30, 184));

// ================================ meter

		y += 55;
		add_tool(meter = new BC_Meter(5, y, 30, 129, mwindow->preferences->min_meter_db, mwindow->preferences->meter_format));

// =================================== mute

		y += 10;
		add_tool(mute_toggle = new AModuleMute(mwindow->console, module, 40, y));
		add_tool(mute_title = new BC_Title(40, y + 20, "Mute", SMALLFONT));

		y += 80;
		add_tool(reset = new AModuleReset(module, 40, y));
	}
	else
	{
		int x, y;
// ================================== title

		x = 3; y = 3;
		add_tool(this->title = new AModuleTitle(module, module->get_patch_of(), x, y));

// =================================== inv

		y += 22;
		add_tool(inv_toggle = new AModuleInv(mwindow->console, module, x + 32, y));
		add_tool(inv_title = new BC_Title(x + 52, y+2, "Inv", SMALLFONT));
		
		y = 3;
		x += AMODULEWIDTH - 3;
		module->create_plugins(x, y);

// ================================== pans

		add_tool(pan_stick = new AModulePan(module, x, y, 30, 150, 1, mwindow->output_channels, mwindow->channel_positions, module->pan));

// ================================ fader

		x += 70;
		add_tool(fade_title = new BC_Title(x, y + 30, "Fade", SMALLFONT));
		//y += 10;
		add_tool(fade_slider = new AModuleFade(module, x, y, 184, 30));


// =================================== mute

		y += 30;
		add_tool(mute_toggle = new AModuleMute(mwindow->console, module, x + 32, y));
		add_tool(mute_title = new BC_Title(x + 52, y+2, "Mute", SMALLFONT));

// ================================ meter

		add_tool(reset = new AModuleReset(module, x + 150, y));

		y += 23;
		add_tool(meter = new BC_Meter(x, y, 184, 30, mwindow->preferences->min_meter_db, mwindow->preferences->meter_format));
	}

	pan_stick->handle_event();    // update the pan_x and pan_y in amodule
return 0;
}

int AModuleGUI::flip_vertical(int pixel)
{
	module->pixel = pixel;

	if(console->vertical)
	resize_window(pixel, 0, AMODULEWIDTH, console->gui->get_h() - 17);
	else
	resize_window(0, pixel, console->gui->get_w() - 17, AMODULEHEIGHT);
	
	if(console->vertical)
	{
		int x, y;
// ================================== title
		x = 3; y = 3;
		title->resize_tool(x, y);

// =================================== inv
		y += 22;
		inv_toggle->resize_tool(x + 32, y);
		inv_title->resize_tool(x + 52, y+2);

// =================================== plugins
		y += 20;
		module->flip_plugins(x, y);

// ================================== pans
		pan_stick->resize_tool(x, y);

// ================================ fader
		fade_title->resize_tool(70, y - 3);
		y += 10;
		fade_slider->resize_tool(70, y, 30, 184);

// ================================ meter
		y += 55;
		meter->resize_tool(5, y, 30, 129);


// =================================== mute
		y += 10;
		mute_toggle->resize_tool(40, y);
		mute_title->resize_tool(40, y+20);

		y += 80;
		reset->resize_tool(40, y);
	}
	else
	{
		int x, y;
// ================================== title
		x = 3; y = 3;
		title->resize_tool(x, y);

// =================================== inv
		y += 22;
		inv_toggle->resize_tool(x + 32, y);
		inv_title->resize_tool(x + 52, y+2);

		y = 3;
		x += AMODULEWIDTH - 3;
		module->flip_plugins(x, y);

// ================================== pans
		pan_stick->resize_tool(x, y);

// ================================ fader
		x += 70;
		fade_title->resize_tool(x, y + 30);
		fade_slider->resize_tool(x, y, 184, 30);

// =================================== mute
		y += 30;
		mute_toggle->resize_tool(x + 32, y);
		mute_title->resize_tool(x + 52, y+2);

// ================================ meter

		reset->resize_tool(x + 150, y);

		y += 23;
		meter->resize_tool(x, y, 184, 30);
	}
return 0;
}


AModuleTitle::AModuleTitle(AModule *module, Patch *patch, int x, int y)
 : BC_TextBox(x, y, 100, patch->title, 0)
{
	this->module = module;
	this->patch = patch;
}

int AModuleTitle::handle_event()
{
	patch->set_title(get_text());
	strcpy(module->title, get_text());

	for(int i = 0; i < PLUGINS; i++)
	{
		((APlugin*)module->plugins[i])->set_string();
	}
return 0;
}

AModuleInv::AModuleInv(Console *console, AModule *module, int x, int y)
 : BC_CheckBox(x, y, 16, 16, 0)
{
	this->console = console;
	this->module = module;
}

int AModuleInv::handle_event()
{
	// value is changed before this
	console->button_down = 1;
	console->new_status = get_value();
	module->inv = get_value();
return 0;
}

int AModuleInv::cursor_moved_over()
{
	if(console->button_down && console->new_status != get_value())
	{
		update(console->new_status);
		module->inv = get_value();
		return 1;
	}
	return 0;
return 0;
}

int AModuleInv::button_release()
{
	console->button_down = 0;
return 0;
}

AModuleMute::AModuleMute(Console *console, AModule *module, int x, int y)
 : BC_CheckBox(x, y, 16, 16, 0)
{
	this->console = console;
	this->module = module;
}

int AModuleMute::handle_event()
{
	// value is changed before this
	console->button_down = 1;
	console->new_status = get_value();;
	module->mute = get_value();
return 0;
}

int AModuleMute::cursor_moved_over()
{
	if(console->button_down && console->new_status != get_value())
	{
		update(console->new_status);
		module->mute = get_value();
		return 1;
	}
	return 0;
return 0;
}

int AModuleMute::button_release()
{
	console->button_down = 0;
return 0;
}

AModulePan::AModulePan(AModule *module, 
	int x, 
	int y, 
	int r, 
	int virtual_r, 
	float maxvalue, 
	int total_values, 
	int *value_positions, 
	float *values)
 : BC_Pan(x, y, r, virtual_r, maxvalue, total_values, value_positions, values)
{
	this->module = module;
}

AModulePan::~AModulePan()
{
}

int AModulePan::handle_event()
{
		// value is changed before this
	for(int i = 0; i < get_total_values(); i++)
		module->pan[i] = get_value(i);
	
	module->pan_x = get_stick_x();
	module->pan_y = get_stick_y();
return 0;
}


AModuleFade::AModuleFade(AModule *module, int x, int y, int w, int h)
 : BC_FSlider(x, y, w, h, 200, 0, INFINITYGAIN, 6, LTGREY, MEGREY, 1)
{
	this->module = module;
}

AModuleFade::~AModuleFade()
{
}

int AModuleFade::handle_event()
{
		// value is changed before this
	module->fade = get_value();
return 0;
}

AModuleReset::AModuleReset(AModule *module, int x, int y)
 : BC_SmallButton(x, y, "O")
{
	this->module = module;
}

AModuleReset::~AModuleReset()
{
}

int AModuleReset::handle_event()
{
	module->mwindow->reset_meters();
return 0;
}
