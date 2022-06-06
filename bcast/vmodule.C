#include <string.h>
#include "console.h"
#include "filehtal.h"
#include "floatautos.h"
#include "mainwindow.h"
#include "modules.h"
#include "overlayframe.h"
#include "patch.h"
#include "sharedpluginlocation.h"
#include "vframe.h"
#include "vmodule.h"
#include "vplugin.h"

VModule::VModule(MainWindow *mwindow)
 : Module(mwindow)
{
	data_type = TRACK_VIDEO;
	module_height = VMODULEHEIGHT;
	module_width = VMODULEWIDTH;
	gui = 0;
	mode = NORMAL;
}

VModule::~VModule()
{
	for(int i = 0; i < PLUGINS; i++)
	{
		delete (VPlugin*)plugins[i];
	}

// delete subwindow last
	if(gui) delete gui;
}

int VModule::create_objects(int pixel)
{
// get the variables
	int i, x=0, y=0;
	fade = 100;
	mute = 0;
	strcpy(title, get_patch_of()->title);
	this->pixel = pixel;

// build the GUI if necessary
	if(mwindow->gui)
	{
		if(console->vertical)
		console->gui->add_subwindow(gui = new VModuleGUI(mwindow, this, pixel, 0, VMODULEWIDTH, console->gui->get_h() - 17));
		else
		console->gui->add_subwindow(gui = new VModuleGUI(mwindow, this, 0, pixel, console->gui->get_w() - 17, VMODULEHEIGHT));
	}
	else
		create_plugins(x, y);
return 0;
}

int VModule::create_plugins(int &x, int &y)
{
	for(int i = 0; i < PLUGINS; i++)
	{
		plugins[i] = new VPlugin(mwindow, this, i+1);
		((VPlugin*)plugins[i])->create_objects(x, y);
		y += 44;

		if(!console->vertical && y + 44 > VMODULEHEIGHT) 
		{
			y = 3;
			x += VMODULEWIDTH - 3;
		}
	}
	if(!console->vertical && y > 3) { x += VMODULEWIDTH; y = 3; }
return 0;
}

int VModule::flip_plugins(int &x, int &y)
{
	if(mwindow->gui)
	{
		for(int i = 0; i < PLUGINS; i++)
		{
			((VPlugin*)plugins[i])->resize_plugin(x, y);
			y += 44;
			if(!console->vertical && y + 44 > VMODULEHEIGHT)
			{
				y = 3;
				x += VMODULEWIDTH - 3;
			}
		}
		if(!console->vertical && y > 3) { x += VMODULEWIDTH; y = 3; }
	}
return 0;
}

int VModule::set_pixel(int pixel)
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

int VModule::save(FileHTAL *htal)
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
	
	if(mute)
	{
		htal->tag.set_title("MUTE");
		htal->append_tag();
		htal->append_newline();
	}
	
	for(i = 0; i < PLUGINS; i++) plugins[i]->save(htal, "PLUGIN");
	
	htal->tag.set_title("MODE");
	htal->tag.set_property("VALUE", mode);
	htal->append_tag();
	htal->append_newline();

	htal->tag.set_title("FADE");
	htal->tag.set_property("VALUE", fade);
	htal->append_tag();
	htal->append_newline();
	
	htal->tag.set_title("/MODULE");
	htal->append_tag();
	htal->append_newline();
return 0;
}

int VModule::load(FileHTAL *htal, int track_offset)
{
	int result = 0;
	int current_pan = 0;
	int current_plugin = 0;
	int i;
	DB db;

	mode = NORMAL;
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
			if(!strcmp(htal->tag.get_title(), "MUTE"))
			{
				mute = 1;
				if(gui) gui->mute_toggle->update(mute);
			}
			else
			if(!strcmp(htal->tag.get_title(), "TITLE"))
			{
// title must come before plugins to allow plugins to boot properly
				strcpy(title, htal->read_text());
				if(gui) gui->title->update(title);
			}
			else
			if(!strcmp(htal->tag.get_title(), "FADE"))
			{
				fade = htal->tag.get_property("VALUE", (float)100);
				if(gui) gui->fade_slider->update((int)fade);
			}
			else
			if(!strcmp(htal->tag.get_title(), "MODE"))
			{
				mode = htal->tag.get_property("VALUE", NORMAL);
				if(gui) gui->mode_popup->update(gui->mode_popup->mode_to_text(mode));
			}
		}
	}while(!result);
return 0;
}

int VModule::set_title(const char *text)
{
	strcpy(title, text);
	if(gui) gui->title->update(text);
	
	for(int i = 0; i < PLUGINS; i++)
	{
		((VPlugin*)plugins[i])->set_string();
	}
return 0;
}

int VModule::flip_vertical(int pixel) { if(gui) gui->flip_vertical(pixel); return 0;
}

int VModule::change_x(int new_pixel) { if(gui) gui->change_x(new_pixel); return 0;
}

int VModule::change_y(int new_pixel) { if(gui) gui->change_y(new_pixel); return 0;
}

int VModule::set_mute(int value)
{
	mute = value;
	if(gui) gui->mute_toggle->update(value);
return 0;
}




VModuleGUI::VModuleGUI(MainWindow *mwindow, VModule *module, int x, int y, int w, int h)
 : BC_SubWindow(x, y, w, h, MEGREY)
{
	this->mwindow = mwindow;
	this->console = mwindow->console;
	this->module = module;
}

VModuleGUI::~VModuleGUI()
{
	delete title;
	delete fade_slider;
}

int VModuleGUI::create_objects()
{
	add_border();
	
	if(console->vertical)
	{
		int x, y;
// ================================== title

		x = 3; y = 3;
		add_tool(this->title = new VModuleTitle(module, module->get_patch_of(), x, y));

		y += 22;
		module->create_plugins(x, y);

// =================================== mode

		add_tool(mode_popup = new VModuleMode(mwindow->console, module, x, y));
		add_tool(mode_title = new BC_Title(x, y + 25, "Mode", SMALLFONT));

// ================================ fader

		y += 25;
		add_tool(fade_title = new BC_Title(70, y - 3, "Fade", SMALLFONT));
		y += 10;
		add_tool(fade_slider = new VModuleFade(module, 70, y, 30, 184));

// =================================== mute

		y += 65;
		add_tool(mute_toggle = new VModuleMute(mwindow->console, module, 40, y));
		add_tool(mute_title = new BC_Title(40, y + 20, "Mute", SMALLFONT));
	}
	else
	{
		int x, y;
// ================================== title

		x = 3; y = 3;
		add_tool(this->title = new VModuleTitle(module, module->get_patch_of(), x, y));

		y = 3;
		x += VMODULEWIDTH - 3;
		module->create_plugins(x, y);

// ================================ fader

		x += 70;
		add_tool(fade_title = new BC_Title(x, y + 30, "Fade", SMALLFONT));
		//y += 10;
		add_tool(fade_slider = new VModuleFade(module, x, y, 184, 30));

// =================================== mute

		y += 30;
		add_tool(mute_toggle = new VModuleMute(mwindow->console, module, x + 32, y));
		add_tool(mute_title = new BC_Title(x + 52, y+2, "Mute", SMALLFONT));

// =================================== mode

		y += 25;
		add_tool(mode_title = new BC_Title(x, y + 5, "Mode", SMALLFONT));
		x += 32; 
		add_tool(mode_popup = new VModuleMode(mwindow->console, module, x, y));
	}
return 0;
}

int VModuleGUI::flip_vertical(int pixel)
{
	module->pixel = pixel;

	if(console->vertical)
	resize_window(pixel, 0, VMODULEWIDTH, console->gui->get_h() - 17);
	else
	resize_window(0, pixel, console->gui->get_w() - 17, VMODULEHEIGHT);
	
	if(console->vertical)
	{
		int x, y;
// ================================== title
		x = 3; y = 3;
		title->resize_tool(x, y);

// =================================== plugins
		y += 22;
		module->flip_plugins(x, y);

// =================================== mode

		mode_popup->resize_tool(x, y);
		mode_title->resize_tool(x, y + 25);

// ================================ fader
		y += 25;
		fade_title->resize_tool(70, y - 3);
		y += 10;
		fade_slider->resize_tool(70, y, 30, 184);

// =================================== mute
		y += 65;
		mute_toggle->resize_tool(40, y);
		mute_title->resize_tool(40, y+20);
	}
	else
	{
		int x, y;
// ================================== title
		x = 3; y = 3;
		title->resize_tool(x, y);

		y = 3;
		x += VMODULEWIDTH - 3;
		module->flip_plugins(x, y);

// ================================ fader
		x += 70;
		fade_slider->resize_tool(x, y, 184, 30);

// =================================== mute
		y += 30;
		mute_toggle->resize_tool(x + 32, y);
		mute_title->resize_tool(x + 52, y+2);

// =================================== mode
		y += 25;
		mode_title->resize_tool(x, y + 5);
		x += 32;
		mode_popup->resize_tool(x, y);
	}
return 0;
}


VModuleTitle::VModuleTitle(VModule *module, Patch *patch, int x, int y)
 : BC_TextBox(x, y, 100, patch->title, 0)
{
	this->module = module;
	this->patch = patch;
}

int VModuleTitle::handle_event()
{
	patch->set_title(get_text());
	strcpy(module->title, get_text());
	
	for(int i = 0; i < PLUGINS; i++)
	{
		((VPlugin*)module->plugins[i])->set_string();
	}
return 0;
}

VModuleMute::VModuleMute(Console *console, VModule *module, int x, int y)
 : BC_CheckBox(x, y, 16, 16, 0)
{
	this->console = console;
	this->module = module;
}

int VModuleMute::handle_event()
{
	// value is changed before this
	console->button_down = 1;
	console->new_status = get_value();;
	module->mute = get_value();
return 0;
}

int VModuleMute::cursor_moved_over()
{
	if(console->button_down && console->new_status != get_value())
	{
		update(console->new_status);
		module->mute = get_value();
	}
return 0;
}

int VModuleMute::button_release()
{
	console->button_down = 0;
return 0;
}

VModuleFade::VModuleFade(VModule *module, int x, int y, int w, int h)
 : BC_ISlider(x, y, w, h, 200, 100, 0, 100, LTGREY, MEGREY, 1)
{
	this->module = module;
}

VModuleFade::~VModuleFade()
{
}

int VModuleFade::handle_event()
{
		// value is changed before this
	module->fade = (float)get_value();
//printf("VModuleFade::handle_event %f\n", module->fade);
return 0;
}


VModuleMode::VModuleMode(Console *console, VModule *module, int x, int y)
 : BC_PopupMenu(x, y, 80, mode_to_text(module->mode), 1)
{
	this->module = module;
	this->console = console;
}

VModuleMode::~VModuleMode()
{
}

int VModuleMode::handle_event()
{
return 0;
}

int VModuleMode::add_items()         // add initial items
{
	add_item(new VModuleModeItem(this, "Normal", NORMAL));
	add_item(new VModuleModeItem(this, "Addition", ADDITION));
	add_item(new VModuleModeItem(this, "Subtract", SUBTRACT));
	add_item(new VModuleModeItem(this, "Multiply", MULTIPLY));
	add_item(new VModuleModeItem(this, "Divide", DIVIDE));
return 0;
}


const char* VModuleMode::mode_to_text(int mode)
{
	switch(mode)
	{
		case NORMAL:
			return "Normal";
			break;
		
		case ADDITION:
			return "Addition";
			break;
		
		case SUBTRACT:
			return "Subtract";
			break;
		
		case MULTIPLY:
			return "Multiply";
			break;
		
		case DIVIDE:
			return "Divide";
			break;
		
		default:
			return "Normal";
			break;
	}
}

VModuleModeItem::VModuleModeItem(VModuleMode *popup, const char *text, int mode)
 : BC_PopupItem(text)
{
	this->popup = popup;
	this->mode = mode;
}

VModuleModeItem::~VModuleModeItem()
{
}

int VModuleModeItem::handle_event()
{
	popup->update(get_text());
	popup->module->mode = mode;
return 0;
}

