#ifndef PLAYBACKPREFS_H
#define PLAYBACKPREFS_H

class PlaybackOutPath;
class PlaybackOutBits;
class PlaybackBufferSize;
class PlaybackBufferBytes;
class PlaybackOutChannels;
class PlaybackViewFollows;
class PlaybackRealTime;
class PlaybackSoftwareTimer;
class PlaybackModuleFragment;
class PlaybackReadLength;
class PlaybackDisableNoEdits;
class PlaybackPreload;

#include "adeviceprefs.inc"
#include "adrivermenu.inc"
#include "bcbase.h"
#include "preferencesthread.h"

class PlaybackPrefs : public PreferencesDialog
{
public:
	PlaybackPrefs(PreferencesWindow *pwindow);
	~PlaybackPrefs();

	int create_objects();

	int get_buffer_bytes();

	ADevicePrefs *out_device;
};

class PlaybackBufferSize : public BC_TextBox
{
public:
	PlaybackBufferSize(int x, int y, PreferencesWindow *pwindow, PlaybackPrefs *playback, char *text);
	int handle_event();
	PreferencesWindow *pwindow;
	PlaybackPrefs *playback;
};

class PlaybackPreload : public BC_TextBox
{
public:
	PlaybackPreload(int x, int y, PreferencesWindow *pwindow, PlaybackPrefs *playback, char *text);
	int handle_event();
	PreferencesWindow *pwindow;
	PlaybackPrefs *playback;
};

class PlaybackReadLength : public BC_TextBox
{
public:
	PlaybackReadLength(int x, int y, PreferencesWindow *pwindow, PlaybackPrefs *playback, char *text);
	int handle_event();
	PreferencesWindow *pwindow;
	PlaybackPrefs *playback;
};

class PlaybackModuleFragment : public BC_TextBox
{
public:
	PlaybackModuleFragment(int x, int y, PreferencesWindow *pwindow, PlaybackPrefs *playback, char *text);
	int handle_event();
	PreferencesWindow *pwindow;
	PlaybackPrefs *playback;
};

class PlaybackBufferBytes : public BC_Title
{
public:
	PlaybackBufferBytes(int x, int y, PreferencesWindow *pwindow, PlaybackPrefs *playback, char *text);
	
	int update_bytes();
	
	PreferencesWindow *pwindow;
	PlaybackPrefs *playback;
	char string[1024];
};


class PlaybackDisableNoEdits : public BC_CheckBox
{
public:
	PlaybackDisableNoEdits(PreferencesWindow *pwindow, int value, int y);
	int handle_event();
	PreferencesWindow *pwindow;
};

class PlaybackViewFollows : public BC_CheckBox
{
public:
	PlaybackViewFollows(PreferencesWindow *pwindow, int value, int y);
	int handle_event();
	PreferencesWindow *pwindow;
};

class PlaybackSoftwareTimer : public BC_CheckBox
{
public:
	PlaybackSoftwareTimer(PreferencesWindow *pwindow, int value, int y);
	int handle_event();
	PreferencesWindow *pwindow;
};

class PlaybackRealTime : public BC_CheckBox
{
public:
	PlaybackRealTime(PreferencesWindow *pwindow, int value, int y);
	int handle_event();
	PreferencesWindow *pwindow;
};


#endif
