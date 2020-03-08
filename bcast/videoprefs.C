#include <string.h>
#include "mainwindow.h"
#include "preferences.h"
#include "vdeviceprefs.h"
#include "videoconfig.h"
#include "videodevice.inc"
#include "videoprefs.h"

VideoPrefs::VideoPrefs(PreferencesWindow *pwindow)
 : PreferencesDialog(pwindow)
{
}

VideoPrefs::~VideoPrefs()
{
	delete in_device;
	delete out_device;
}

int VideoPrefs::create_objects()
{
	char string[1024];
	int y = 10, x = 10;

	add_border(get_resources()->get_bg_shadow1(),
		get_resources()->get_bg_shadow2(),
		get_resources()->get_bg_color(),
		get_resources()->get_bg_light2(),
		get_resources()->get_bg_light1());
	add_tool(new BC_Title(10, y, "Video", LARGEFONT, BLACK));
	y += 30;
	add_tool(new BC_Title(10, y, "Actual framerate being achieved:"));
	add_tool(frame_rate = new BC_Title(250, y, "-", MEDIUMFONT, YELLOW));
	draw_framerate();
	y += 25;
	add_tool(new VideoEveryFrame(pwindow, y));
	y += 20;
	add_tool(fastest = new VideoFastest(pwindow, y, this));
	y += 20;
	add_tool(alphachannels = new VideoAlphaChannels(pwindow, y, this));
	y += 20;
	add_tool(floatingpoint = new VideoFloatingPoint(pwindow, y, this));
	y += 20;
	add_tool(interpolate = new VideoInterpolate(pwindow, y, this));
	y += 30;

	out_device = new VDevicePrefs(x + 135, 
		y, 
		pwindow, 
		this, 
		MODEPLAY);
	out_device->initialize();

	add_tool(new BC_Title(x, y, "Playback Driver:"));
	add_tool(new VideoPlayDriverMenu(x, y + 20, out_device, &(pwindow->preferences->vconfig->video_out_driver)));

	y += 50;
	in_device = new VDevicePrefs(x + 135, 
		y, 
		pwindow, 
		this, 
		MODERECORD);
	in_device->initialize();

	add_tool(new BC_Title(x, y, "Record driver:"));
	add_tool(new VideoRecDriverMenu(x, y + 20, in_device, &(pwindow->preferences->vconfig->video_in_driver)));

	y += 60;
	sprintf(string, "%d", pwindow->preferences->video_write_length);
	add_tool(new VideoWriteLength(pwindow, string, y));
	add_tool(new BC_Title(120, y, "Frames to record to disk at a time."));
	y += 30;
	sprintf(string, "%d", pwindow->preferences->vconfig->capture_length);
	add_tool(new VideoCaptureLength(pwindow, string, y));
	add_tool(new BC_Title(120, y, "Frames to buffer in device."));
return 0;
}

int VideoPrefs::set_optimizations(int fastest, int alphachannels, int floatingpoint, int interpolate)
{
	pwindow->preferences->video_interpolate = interpolate;
	pwindow->preferences->video_floatingpoint = floatingpoint | interpolate;
	pwindow->preferences->video_use_alpha = alphachannels | floatingpoint | interpolate;
	this->fastest->update(fastest);
	this->alphachannels->update(alphachannels);
	this->floatingpoint->update(floatingpoint);
	this->interpolate->update(interpolate);
return 0;
}

int VideoPrefs::draw_framerate()
{
	char string[1024];
	if(pwindow->preferences->actual_frame_rate <= 0)
		sprintf(string, "%.2f fps", pwindow->mwindow->frame_rate);
	else
		sprintf(string, "%.2f fps", pwindow->preferences->actual_frame_rate);
	frame_rate->update(string);
return 0;
}


VideoEveryFrame::VideoEveryFrame(PreferencesWindow *pwindow, int y)
 : BC_CheckBox(10, y, 16, 16, pwindow->preferences->video_every_frame, "Play every frame")
{ this->pwindow = pwindow; }

VideoEveryFrame::~VideoEveryFrame()
{
}

int VideoEveryFrame::handle_event()
{
	pwindow->preferences->video_every_frame = get_value();
return 0;
}




VideoReadLength::VideoReadLength(PreferencesWindow *pwindow, int y)
 : BC_TextBox(5, y, 100, pwindow->preferences->video_read_length)
{ this->pwindow = pwindow; }

VideoReadLength::~VideoReadLength()
{
}

int VideoReadLength::handle_event()
{
	pwindow->preferences->video_read_length = atol(get_text());
return 0;
}




VideoOutputLength::VideoOutputLength(PreferencesWindow *pwindow, int y)
 : BC_TextBox(5, y, 100, pwindow->preferences->video_output_length)
{ this->pwindow = pwindow; }

VideoOutputLength::~VideoOutputLength()
{
}

int VideoOutputLength::handle_event()
{
	pwindow->preferences->video_output_length = atol(get_text());
return 0;
}


VideoRateReset::VideoRateReset(PreferencesWindow *pwindow, int y, VideoPrefs *prefs)
 : BC_BigButton(100, y, "Reset")
{
	this->pwindow = pwindow; this->prefs = prefs;
}

VideoRateReset::~VideoRateReset()
{
}

int VideoRateReset::handle_event()
{
	pwindow->preferences->actual_frame_rate = -1;
	prefs->draw_framerate();
return 0;
}

VideoInterpolate::VideoInterpolate(PreferencesWindow *pwindow, int y, VideoPrefs *prefs)
 : BC_Radial(10, y, 16, 16, pwindow->preferences->video_interpolate, "Bilinear interpolation")
{ this->pwindow = pwindow; this->prefs = prefs; }

VideoInterpolate::~VideoInterpolate()
{
}

int VideoInterpolate::handle_event()
{	
	update(1);
	prefs->set_optimizations(0, 0, 0, 1);
return 0;
}


VideoFloatingPoint::VideoFloatingPoint(PreferencesWindow *pwindow, int y, VideoPrefs *prefs)
 : BC_Radial(10, y, 16, 16, 
 	pwindow->preferences->video_floatingpoint && !(pwindow->preferences->video_interpolate), "Floating point alpha channels")
{ this->pwindow = pwindow; this->prefs = prefs; }

VideoFloatingPoint::~VideoFloatingPoint()
{
}

int VideoFloatingPoint::handle_event()
{
	update(1);
	prefs->set_optimizations(0, 0, 1, 0);
return 0;
}


VideoAlphaChannels::VideoAlphaChannels(PreferencesWindow *pwindow, int y, VideoPrefs *prefs)
 : BC_Radial(10, y, 16, 16, 
 	pwindow->preferences->video_use_alpha && !(pwindow->preferences->video_floatingpoint | pwindow->preferences->video_interpolate), 
 	"Alpha channels")
{ this->pwindow = pwindow; this->prefs = prefs; }

VideoAlphaChannels::~VideoAlphaChannels()
{
}

int VideoAlphaChannels::handle_event()
{
	update(1);
	prefs->set_optimizations(0, 1, 0, 0);
return 0;
}

VideoFastest::VideoFastest(PreferencesWindow *pwindow, int y, VideoPrefs *prefs)
 : BC_Radial(10, y, 16, 16, 
 	!(pwindow->preferences->video_use_alpha | pwindow->preferences->video_floatingpoint | pwindow->preferences->video_interpolate), 
 	"Fastest rendering")
{ this->pwindow = pwindow; this->prefs = prefs; }

VideoFastest::~VideoFastest()
{
}

int VideoFastest::handle_event()
{
	update(1);
	prefs->set_optimizations(1, 0, 0, 0);
return 0;
}

VideoPlayDriverMenu::VideoPlayDriverMenu(int x, int y, VDevicePrefs *device_prefs, int *output)
 : BC_PopupMenu(x, y, 125, driver_to_string(*output))
{
	this->output = output;
	this->device_prefs = device_prefs;
}

VideoPlayDriverMenu::~VideoPlayDriverMenu()
{
}

int VideoPlayDriverMenu::handle_event()
{
return 0;
}

char* VideoPlayDriverMenu::driver_to_string(int driver)
{
	switch(driver)
	{
		case PLAYBACK_X11:
			sprintf(string, PLAYBACK_X11_TITLE);
			break;
		case PLAYBACK_LML:
			sprintf(string, PLAYBACK_LML_TITLE);
			break;
		case PLAYBACK_FIREWIRE:
			sprintf(string, PLAYBACK_FIREWIRE_TITLE);
			break;
		default:
			sprintf(string, "");
	}
	return string;
}

int VideoPlayDriverMenu::add_items()
{
// Video4linux versions are automatically detected
	add_item(new VideoPlayDriverItem(this, PLAYBACK_X11_TITLE, PLAYBACK_X11));
	add_item(new VideoPlayDriverItem(this, PLAYBACK_LML_TITLE, PLAYBACK_LML));
//	add_item(new VideoPlayDriverItem(this, PLAYBACK_FIREWIRE_TITLE, PLAYBACK_FIREWIRE));
return 0;
}


VideoPlayDriverItem::VideoPlayDriverItem(VideoPlayDriverMenu *popup, char *text, int driver)
 : BC_PopupItem(text)
{
	this->popup = popup;
	this->driver = driver;
}

VideoPlayDriverItem::~VideoPlayDriverItem()
{
}

int VideoPlayDriverItem::handle_event()
{
	popup->update(get_text());
	*(popup->output) = driver;
	popup->device_prefs->initialize();
	return 1;
return 0;
}


VideoRecDriverMenu::VideoRecDriverMenu(int x, int y, VDevicePrefs *device_prefs, int *output)
 : BC_PopupMenu(x, y, 125, driver_to_string(*output))
{
	this->output = output;
	this->device_prefs = device_prefs;
}

VideoRecDriverMenu::~VideoRecDriverMenu()
{
}

int VideoRecDriverMenu::handle_event()
{
return 0;
}

char* VideoRecDriverMenu::driver_to_string(int driver)
{
	switch(driver)
	{
		case VIDEO4LINUX:
			sprintf(string, VIDEO4LINUX_TITLE);
			break;
		case SCREENCAPTURE:
			sprintf(string, SCREENCAPTURE_TITLE);
			break;
		case CAPTURE_LML:
			sprintf(string, CAPTURE_LML_TITLE);
			break;
		case CAPTURE_FIREWIRE:
			sprintf(string, CAPTURE_FIREWIRE_TITLE);
			break;
		default:
			sprintf(string, "");
	}
	return string;
}

int VideoRecDriverMenu::add_items()
{
// Video4linux versions are automatically detected
	add_item(new VideoRecDriverItem(this, VIDEO4LINUX_TITLE, VIDEO4LINUX));
	add_item(new VideoRecDriverItem(this, SCREENCAPTURE_TITLE, SCREENCAPTURE));
	add_item(new VideoRecDriverItem(this, CAPTURE_LML_TITLE, CAPTURE_LML));
	add_item(new VideoRecDriverItem(this, CAPTURE_FIREWIRE_TITLE, CAPTURE_FIREWIRE));
return 0;
}


VideoRecDriverItem::VideoRecDriverItem(VideoRecDriverMenu *popup, char *text, int driver)
 : BC_PopupItem(text)
{
	this->popup = popup;
	this->driver = driver;
}

VideoRecDriverItem::~VideoRecDriverItem()
{
}

int VideoRecDriverItem::handle_event()
{
	popup->update(get_text());
	*(popup->output) = driver;
	popup->device_prefs->initialize();
	return 1;
return 0;
}



VideoWriteLength::VideoWriteLength(PreferencesWindow *pwindow, char *text, int y)
 : BC_TextBox(10, y, 100, text)
{ this->pwindow = pwindow; }

int VideoWriteLength::handle_event()
{ pwindow->preferences->video_write_length = atol(get_text()); return 0;
}

VideoCaptureLength::VideoCaptureLength(PreferencesWindow *pwindow, char *text, int y)
 : BC_TextBox(10, y, 100, text)
{ this->pwindow = pwindow; }

int VideoCaptureLength::handle_event()
{ pwindow->preferences->vconfig->capture_length = atol(get_text()); return 0;
}
