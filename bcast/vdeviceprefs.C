#include <string.h>
#include "vdeviceprefs.h"
#include "videoconfig.h"
#include "videodevice.inc"
#include "preferences.h"
#include "preferencesthread.h"


VDevicePrefs::VDevicePrefs(int x, 
	int y, 
	PreferencesWindow *pwindow, 
	PreferencesDialog *dialog, 
	int mode)
{
	this->pwindow = pwindow;
	this->dialog = dialog;
	this->driver = -1;
	this->mode = mode;
	this->x = x;
	this->y = y;
}

VDevicePrefs::~VDevicePrefs()
{
	delete_objects();
}


int VDevicePrefs::initialize()
{
	delete_objects();

	if(mode == MODEPLAY)
	{
		driver = pwindow->preferences->vconfig->video_out_driver;
		switch(driver)
		{
			case PLAYBACK_X11:
				break;
			case PLAYBACK_LML:
				create_lml_objs();
				break;
			case PLAYBACK_FIREWIRE:
				create_firewire_objs();
				break;
		}
	}
	else
	if(mode == MODERECORD)
	{
		driver = pwindow->preferences->vconfig->video_in_driver;
		switch(driver)
		{
			case VIDEO4LINUX:
				create_v4l_objs();
				break;
			case SCREENCAPTURE:
				create_screencap_objs();
				break;
			case CAPTURE_LML:
				create_lml_objs();
				break;
			case CAPTURE_FIREWIRE:
				create_firewire_objs();
				break;
		}
	}
	return 0;
return 0;
}

int VDevicePrefs::delete_objects()
{
	if(mode == MODEPLAY)
	{
		switch(driver)
		{
			case PLAYBACK_X11:
				break;
			case PLAYBACK_LML:
				delete device_title;
				delete device_text;
				break;
			case PLAYBACK_FIREWIRE:
				break;
		}
	}
	else
	if(mode == MODERECORD)
	{
		switch(driver)
		{
			case VIDEO4LINUX:
			case SCREENCAPTURE:
			case CAPTURE_LML:
				delete device_title;
				delete device_text;
				break;
			case CAPTURE_FIREWIRE:
				delete port_title;
				delete firewire_port;
				delete channel_title;
				delete firewire_channel;
				break;
		}
	}

	driver = -1;
	return 0;
return 0;
}

int VDevicePrefs::create_lml_objs()
{
	char *output_char;

	switch(mode)
	{
		case MODEPLAY: 
			output_char = pwindow->preferences->vconfig->lml_out_device;
			break;
		case MODERECORD:
			output_char = pwindow->preferences->vconfig->lml_in_device;
			break;
	}
	dialog->add_tool(device_title = new BC_Title(x, y, "Device path:", MEDIUMFONT, BLACK));
	dialog->add_tool(device_text = new VDeviceTextBox(x, y + 20, output_char));
	return 0;
return 0;
}

int VDevicePrefs::create_firewire_objs()
{
	int *output_int;
	int x1 = x;

	output_int = &pwindow->preferences->vconfig->vfirewire_in_port;
	dialog->add_tool(port_title = new BC_Title(x1, y, "Port:", MEDIUMFONT, BLACK));
	dialog->add_tool(firewire_port = new VDeviceIntBox(x1, y + 20, output_int));

	x1 += firewire_port->get_w() + 5;
	output_int = &pwindow->preferences->vconfig->vfirewire_in_channel;
	dialog->add_tool(channel_title = new BC_Title(x1, y, "Channel:", MEDIUMFONT, BLACK));
	dialog->add_tool(firewire_channel = new VDeviceIntBox(x1, y + 20, output_int));
	return 0;
return 0;
}

int VDevicePrefs::create_v4l_objs()
{
	char *output_char;
	output_char = pwindow->preferences->vconfig->v4l_in_device;
	dialog->add_tool(device_title = new BC_Title(x, y, "Device path:", MEDIUMFONT, BLACK));
	dialog->add_tool(device_text = new VDeviceTextBox(x, y + 20, output_char));
	return 0;
return 0;
}

int VDevicePrefs::create_screencap_objs()
{
	char *output_char;
	output_char = pwindow->preferences->vconfig->screencapture_display;
	dialog->add_tool(device_title = new BC_Title(x, y, "Display:", MEDIUMFONT, BLACK));
	dialog->add_tool(device_text = new VDeviceTextBox(x, y + 20, output_char));
	return 0;
return 0;
}






VDeviceTextBox::VDeviceTextBox(int x, int y, char *output)
 : BC_TextBox(x, y, 200, output)
{ 
	this->output = output; 
}

int VDeviceTextBox::handle_event() 
{ 
	strcpy(output, get_text()); 
return 0;
}

VDeviceIntBox::VDeviceIntBox(int x, int y, int *output)
 : BC_TextBox(x, y, 60, *output)
{ 
	this->output = output; 
}

int VDeviceIntBox::handle_event() 
{ 
	*output = atol(get_text()); 
return 0;
}
