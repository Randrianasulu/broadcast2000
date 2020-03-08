#ifndef ADEVICEPREFS_H
#define ADEVICEPREFS_H

// Modes
#ifndef MODEPLAY
#define MODEPLAY 0
#define MODERECORD 1
#define MODEDUPLEX 2
#endif

class OSSPath;
class OSSBits;
class OSSChannels;
class ESoundServer;
class ESoundPort;
class FirewirePort;
class FirewireChannel;

#include "bcbase.h"
#include "preferencesthread.inc"

class ADevicePrefs
{
public:
	ADevicePrefs(int x, 
		int y, 
		PreferencesWindow *pwindow, 
		PreferencesDialog *dialog, 
		int mode);
	~ADevicePrefs();

	int initialize();
	int delete_objects();

	PreferencesWindow *pwindow;

private:
	int create_oss_objs();
	int create_esound_objs();
	int create_firewire_objs();

	PreferencesDialog *dialog;
	int driver, mode;
	int x;
	int y;
	BC_Title *driver_title, *path_title, *bits_title, *channels_title;
	BC_Title *server_title, *port_title, *channel_title;
	OSSPath *oss_path;
	OSSBits *oss_bits;
	OSSChannels *oss_channels;
	ESoundServer *esound_server;
	ESoundPort *esound_port;
	FirewirePort *firewire_port;
	FirewireChannel *firewire_channel;
};

class OSSPath : public BC_TextBox
{
public:
	OSSPath(int x, int y, char *output);
	int handle_event();
	char *output;
};

class OSSBits : public BC_TextBox
{
public:
	OSSBits(int x, int y, int *output);
	int handle_event();
	int *output;
};

class OSSChannels : public BC_TextBox
{
public:
	OSSChannels(int x, int y, int *output);
	int handle_event();
	int *output;
};

class ESoundServer : public BC_TextBox
{
public:
	ESoundServer(int x, int y, char *output);
	int handle_event();
	char *output;
};

class ESoundPort : public BC_TextBox
{
public:
	ESoundPort(int x, int y, int *output);
	int handle_event();
	int *output;
};

class FirewirePort : public BC_TextBox
{
public:
	FirewirePort(int x, int y, int *output);
	int handle_event();
	int *output;
};

class FirewireChannel : public BC_TextBox
{
public:
	FirewireChannel(int x, int y, int *output);
	int handle_event();
	int *output;
};

#endif
