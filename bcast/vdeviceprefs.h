#ifndef VDEVICEPREFS_H
#define VDEVICEPREFS_H

// Modes
#ifndef MODEPLAY
#define MODEPLAY 0
#define MODERECORD 1
#define MODEDUPLEX 2
#endif

#include "adeviceprefs.inc"
#include "bcbase.h"
#include "preferencesthread.inc"

class VDeviceTextBox;
class VDeviceIntBox;

class VDevicePrefs
{
public:
	VDevicePrefs(int x, 
		int y, 
		PreferencesWindow *pwindow, 
		PreferencesDialog *dialog, 
		int mode);
	~VDevicePrefs();

	int initialize();
	int delete_objects();

	PreferencesWindow *pwindow;

private:
	int create_lml_objs();
	int create_firewire_objs();
	int create_v4l_objs();
	int create_screencap_objs();

	PreferencesDialog *dialog;
	BC_Title *device_title, *port_title, *channel_title;
	VDeviceTextBox *device_text;
	VDeviceIntBox *firewire_port, *firewire_channel;
	int driver, mode;
	int x;
	int y;
};

class VDeviceTextBox : public BC_TextBox
{
public:
	VDeviceTextBox(int x, int y, char *output);

	int handle_event();
	char *output;
};

class VDeviceIntBox : public BC_TextBox
{
public:
	VDeviceIntBox(int x, int y, int *output);

	int handle_event();
	int *output;
};

#endif
