#include <string.h>
#include "adeviceprefs.h"
#include "audioconfig.h"
#include "audiodevice.inc"
#include "preferences.h"
#include "preferencesthread.h"


ADevicePrefs::ADevicePrefs(int x, 
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

ADevicePrefs::~ADevicePrefs()
{
	delete_objects();
}


int ADevicePrefs::initialize()
{
	delete_objects();
	switch(mode)
	{
		case MODEPLAY:
			driver = pwindow->preferences->aconfig->audio_out_driver;
			break;
		case MODERECORD:
			driver = pwindow->preferences->aconfig->audio_in_driver;
			break;
		case MODEDUPLEX:
			driver = pwindow->preferences->aconfig->audio_duplex_driver;
			break;
	}

	switch(driver)
	{
		case AUDIO_OSS:
			create_oss_objs();
			break;
		case AUDIO_ESOUND:
			create_esound_objs();
			break;
		case AUDIO_1394:
			create_firewire_objs();
			break;
	}
	return 0;
return 0;
}

int ADevicePrefs::delete_objects()
{
	switch(driver)
	{
		case AUDIO_OSS:
			delete path_title;
			delete bits_title;
			delete channels_title;
			delete oss_path;
			delete oss_bits;
			delete oss_channels;
			break;
		case AUDIO_ESOUND:
			delete server_title;
			delete port_title;
			delete esound_server;
			delete esound_port;
			break;
		case AUDIO_1394:
			delete port_title;
			delete channel_title;
			delete firewire_port;
			delete firewire_channel;
			break;
	}
	driver = -1;
	return 0;
return 0;
}

int ADevicePrefs::create_oss_objs()
{
	int x1 = x;
	char *output_char;
	int *output_int;

	switch(mode)
	{
		case MODEPLAY: 
			output_char = pwindow->preferences->aconfig->oss_out_device;
			break;
		case MODERECORD:
			output_char = pwindow->preferences->aconfig->oss_in_device;
			break;
		case MODEDUPLEX:
			output_char = pwindow->preferences->aconfig->oss_duplex_device;
			break;
	}
	dialog->add_tool(path_title = new BC_Title(x1, y, "Device path:", MEDIUMFONT, BLACK));
	dialog->add_tool(oss_path = new OSSPath(x1, y + 20, output_char));

	x1 += oss_path->get_w() + 5;
	switch(mode)
	{
		case MODEPLAY: 
			output_int = &pwindow->preferences->aconfig->oss_out_bits;
			break;
		case MODERECORD:
			output_int = &pwindow->preferences->aconfig->oss_in_bits;
			break;
		case MODEDUPLEX:
			output_int = &pwindow->preferences->aconfig->oss_duplex_bits;
			break;
	}
	dialog->add_tool(bits_title = new BC_Title(x1, y, "Bits:", MEDIUMFONT, BLACK));
	dialog->add_tool(oss_bits = new OSSBits(x1, y + 20, output_int));

	x1 += oss_bits->get_w() + 5;
	switch(mode)
	{
		case MODEPLAY: 
			output_int = &pwindow->preferences->aconfig->oss_out_channels;
			break;
		case MODERECORD:
			output_int = &pwindow->preferences->aconfig->oss_in_channels;
			break;
		case MODEDUPLEX:
			output_int = &pwindow->preferences->aconfig->oss_duplex_channels;
			break;
	}
	dialog->add_tool(channels_title = new BC_Title(x1, y, "Channels:", MEDIUMFONT, BLACK));
	dialog->add_tool(oss_channels = new OSSChannels(x1, y + 20, output_int));

	return 0;
return 0;
}

int ADevicePrefs::create_esound_objs()
{
	int x1 = x;
	char *output_char;
	int *output_int;

	switch(mode)
	{
		case MODEPLAY: 
			output_char = pwindow->preferences->aconfig->esound_out_server;
			break;
		case MODERECORD:
			output_char = pwindow->preferences->aconfig->esound_in_server;
			break;
		case MODEDUPLEX:
			output_char = pwindow->preferences->aconfig->esound_duplex_server;
			break;
	}
	dialog->add_tool(server_title = new BC_Title(x1, y, "Server:", MEDIUMFONT, BLACK));
	dialog->add_tool(esound_server = new ESoundServer(x1, y + 20, output_char));

	switch(mode)
	{
		case MODEPLAY: 
			output_int = &pwindow->preferences->aconfig->esound_out_port;
			break;
		case MODERECORD:
			output_int = &pwindow->preferences->aconfig->esound_in_port;
			break;
		case MODEDUPLEX:
			output_int = &pwindow->preferences->aconfig->esound_duplex_port;
			break;
	}
	x1 += esound_server->get_w() + 5;
	dialog->add_tool(port_title = new BC_Title(x1, y, "Port:", MEDIUMFONT, BLACK));
	dialog->add_tool(esound_port = new ESoundPort(x1, y + 20, output_int));
	return 0;
return 0;
}

int ADevicePrefs::create_firewire_objs()
{
	int x1 = x;
	int *output_int;

	switch(mode)
	{
		case MODEPLAY: 
			output_int = &pwindow->preferences->aconfig->afirewire_in_port;
			break;
		case MODERECORD:
			output_int = &pwindow->preferences->aconfig->afirewire_in_port;
			break;
		case MODEDUPLEX:
			output_int = &pwindow->preferences->aconfig->afirewire_in_port;
			break;
	}
	dialog->add_tool(port_title = new BC_Title(x1, y, "Port:", MEDIUMFONT, BLACK));
	dialog->add_tool(firewire_port = new FirewirePort(x1, y + 20, output_int));

	x1 += firewire_port->get_w() + 5;
	switch(mode)
	{
		case MODEPLAY: 
			output_int = &pwindow->preferences->aconfig->afirewire_in_channel;
			break;
		case MODERECORD:
			output_int = &pwindow->preferences->aconfig->afirewire_in_channel;
			break;
		case MODEDUPLEX:
			output_int = &pwindow->preferences->aconfig->afirewire_in_channel;
			break;
	}
	dialog->add_tool(channel_title = new BC_Title(x1, y, "Channel:", MEDIUMFONT, BLACK));
	dialog->add_tool(firewire_channel = new FirewireChannel(x1, y + 20, output_int));
	return 0;
return 0;
}



OSSPath::OSSPath(int x, int y, char *output)
 : BC_TextBox(x, y, 150, output)
{ 
	this->output = output; 
}

int OSSPath::handle_event() 
{ 
	strcpy(output, get_text()); 
return 0;
}

OSSBits::OSSBits(int x, int y, int *output)
 : BC_TextBox(x, y, 50, *output)
{ 
	this->output = output; 
}

int OSSBits::handle_event() 
{ 
	*output = atol(get_text()); 
return 0;
}

OSSChannels::OSSChannels(int x, int y, int *output)
 : BC_TextBox(x, y, 50, *output)
{ 
	this->output = output;
}

int OSSChannels::handle_event() 
{ 
	*output = atol(get_text()); 
return 0;
}

ESoundServer::ESoundServer(int x, int y, char *output)
 : BC_TextBox(x, y, 150, output)
{ 
	this->output = output; 
}

int ESoundServer::handle_event() 
{ 
	strcpy(output, get_text()); 
return 0;
}

ESoundPort::ESoundPort(int x, int y, int *output)
 : BC_TextBox(x, y, 50, *output)
{ 
	this->output = output;
}

int ESoundPort::handle_event() 
{ 
	*output = atol(get_text()); 
return 0;
}

FirewirePort::FirewirePort(int x, int y, int *output)
 : BC_TextBox(x, y, 50, *output)
{ 
	this->output = output;
}

int FirewirePort::handle_event() 
{ 
	*output = atol(get_text()); 
return 0;
}

FirewireChannel::FirewireChannel(int x, int y, int *output)
 : BC_TextBox(x, y, 50, *output)
{ 
	this->output = output;
}

int FirewireChannel::handle_event() 
{ 
	*output = atol(get_text()); 
return 0;
}

