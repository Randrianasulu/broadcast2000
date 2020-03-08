#ifndef VIDEOPREFS_H
#define VIDEOPREFS_H

#include "preferencesthread.h"
#include "vdeviceprefs.inc"

class VideoFastest;
class VideoAlphaChannels;
class VideoFloatingPoint;
class VideoInterpolate;


class VideoPrefs : public PreferencesDialog
{
public:
	VideoPrefs(PreferencesWindow *pwindow);
	~VideoPrefs();

	int create_objects();
	int draw_framerate();
	int set_optimizations(int fastest, int alphachannels, int floatingpoint, int interpolate);

	BC_Title *frame_rate;
	VideoFastest *fastest;
	VideoAlphaChannels *alphachannels;
	VideoFloatingPoint *floatingpoint;
	VideoInterpolate *interpolate;
	VDevicePrefs *out_device, *in_device;
};





class VideoEveryFrame : public BC_CheckBox
{
public:
	VideoEveryFrame(PreferencesWindow *pwindow, int y);
	~VideoEveryFrame();

	int handle_event();
	PreferencesWindow *pwindow;
};



class VideoReadLength : public BC_TextBox
{
public:
	VideoReadLength(PreferencesWindow *pwindow, int y);
	~VideoReadLength();

	int handle_event();
	PreferencesWindow *pwindow;
};



class VideoOutputLength : public BC_TextBox
{
public:
	VideoOutputLength(PreferencesWindow *pwindow, int y);
	~VideoOutputLength();

	int handle_event();
	PreferencesWindow *pwindow;
};

class VideoRateReset : public BC_BigButton
{
public:
	VideoRateReset(PreferencesWindow *pwindow, int y, VideoPrefs *prefs);
	~VideoRateReset();
	int handle_event();
	PreferencesWindow *pwindow;
	VideoPrefs *prefs;
};

class VideoInterpolate : public BC_Radial
{
public:
	VideoInterpolate(PreferencesWindow *pwindow, int y, VideoPrefs *prefs);
	~VideoInterpolate();

	int handle_event();
	PreferencesWindow *pwindow;
	VideoPrefs *prefs;
};

class VideoFloatingPoint : public BC_Radial
{
public:
	VideoFloatingPoint(PreferencesWindow *pwindow, int y, VideoPrefs *prefs);
	~VideoFloatingPoint();

	int handle_event();
	PreferencesWindow *pwindow;
	VideoPrefs *prefs;
};

class VideoAlphaChannels : public BC_Radial
{
public:
	VideoAlphaChannels(PreferencesWindow *pwindow, int y, VideoPrefs *prefs);
	~VideoAlphaChannels();

	int handle_event();
	PreferencesWindow *pwindow;
	VideoPrefs *prefs;
};

class VideoFastest : public BC_Radial
{
public:
	VideoFastest(PreferencesWindow *pwindow, int y, VideoPrefs *prefs);
	~VideoFastest();

	int handle_event();
	PreferencesWindow *pwindow;
	VideoPrefs *prefs;
};

class VideoPlayDriverMenu : public BC_PopupMenu
{
public:
// set wr to 1 for writable file formats
	VideoPlayDriverMenu(int x, int y, VDevicePrefs *device_prefs, int *output);
	~VideoPlayDriverMenu();

	int handle_event();  // user copies text to value here
	int add_items();         // add initial items
	char* driver_to_string(int driver);

	VDevicePrefs *device_prefs;
	int *output;
	
private:
	char string[1024];
};

class VideoPlayDriverItem : public BC_PopupItem
{
public:
	VideoPlayDriverItem(VideoPlayDriverMenu *popup, char *text, int driver);
	~VideoPlayDriverItem();

	int handle_event();
	VideoPlayDriverMenu *popup;
	int driver;
};

class VideoRecDriverMenu : public BC_PopupMenu
{
public:
// set wr to 1 for writable file formats
	VideoRecDriverMenu(int x, int y, VDevicePrefs *device_prefs, int *output);
	~VideoRecDriverMenu();

	int handle_event();  // user copies text to value here
	int add_items();         // add initial items
	char* driver_to_string(int driver);

	VDevicePrefs *device_prefs;
	int *output;
	
private:
	char string[1024];
};

class VideoRecDriverItem : public BC_PopupItem
{
public:
	VideoRecDriverItem(VideoRecDriverMenu *popup, char *text, int driver);
	~VideoRecDriverItem();

	int handle_event();
	VideoRecDriverMenu *popup;
	int driver;
};

class VideoWriteLength : public BC_TextBox
{
public:
	VideoWriteLength(PreferencesWindow *pwindow, char *text, int y);
	int handle_event();
	PreferencesWindow *pwindow;
};

class VideoCaptureLength : public BC_TextBox
{
public:
	VideoCaptureLength(PreferencesWindow *pwindow, char *text, int y);
	int handle_event();
	PreferencesWindow *pwindow;
};



#endif
