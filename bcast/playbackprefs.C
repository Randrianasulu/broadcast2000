#include <string.h>
#include "adeviceprefs.h"
#include "adrivermenu.h"
#include "audioconfig.h"
#include "audiodevice.inc"
#include "playbackprefs.h"
#include "preferences.h"

PlaybackPrefs::PlaybackPrefs(PreferencesWindow *pwindow)
 : PreferencesDialog(pwindow)
{
}

PlaybackPrefs::~PlaybackPrefs()
{
	delete out_device;
}

int PlaybackPrefs::create_objects()
{
	int x = 10, y = 10;
	char string[1024];

	add_border(get_resources()->get_bg_shadow1(),
		get_resources()->get_bg_shadow2(),
		get_resources()->get_bg_color(),
		get_resources()->get_bg_light2(),
		get_resources()->get_bg_light1());

	add_tool(new BC_Title(x, y, "Audio Out", LARGEFONT, BLACK));
	y += 35;

	out_device = new ADevicePrefs(x + 135, 
		y, 
		pwindow, 
		this, 
		MODEPLAY);
	out_device->initialize();

	add_tool(new BC_Title(x, y, "Playback driver:", MEDIUMFONT, BLACK));
	add_tool(new AudioDriverMenu(x, y + 20, out_device, &(pwindow->preferences->aconfig->audio_out_driver), 0, 1));
	y += 70;
	add_tool(new BC_Title(x, y, "Samples to read from disk at a time:", MEDIUMFONT, BLACK));
	sprintf(string, "%d", pwindow->preferences->audio_read_length);
	add_tool(new PlaybackReadLength(x + 275, y, pwindow, this, string));
	y += 30;
	add_tool(new BC_Title(x, y, "Samples to send to console at a time:", MEDIUMFONT, BLACK));
	sprintf(string, "%d", pwindow->preferences->audio_module_fragment);
	add_tool(new PlaybackModuleFragment(x + 275, y, pwindow, this, string));
	y += 30;
	add_tool(new BC_Title(x, y, "Preload buffer for multiplexed streams:", MEDIUMFONT, BLACK));
	sprintf(string, "%d", pwindow->preferences->playback_preload);
	add_tool(new PlaybackPreload(x + 275, y, pwindow, this, string));
	y += 35;

	add_tool(new PlaybackDisableNoEdits(pwindow, pwindow->preferences->test_playback_edits, y));
	y += 20;
	add_tool(new PlaybackViewFollows(pwindow, pwindow->preferences->view_follows_playback, y));
	y += 20;
	add_tool(new PlaybackSoftwareTimer(pwindow, pwindow->preferences->playback_software_timer, y));
	y += 20;
	add_tool(new PlaybackRealTime(pwindow, pwindow->preferences->real_time_playback, y));
return 0;
}


int PlaybackPrefs::get_buffer_bytes()
{
	return pwindow->preferences->aconfig->oss_out_bits / 8 * pwindow->preferences->aconfig->oss_out_channels * pwindow->preferences->playback_buffer;
return 0;
}










PlaybackReadLength::PlaybackReadLength(int x, int y, PreferencesWindow *pwindow, PlaybackPrefs *playback, char *text)
 : BC_TextBox(x, y, 100, text)
{ this->pwindow = pwindow; this->playback = playback; }

int PlaybackReadLength::handle_event() 
{ 
	pwindow->preferences->audio_read_length = atol(get_text()); 
return 0;
}




PlaybackModuleFragment::PlaybackModuleFragment(int x, int y, PreferencesWindow *pwindow, PlaybackPrefs *playback, char *text)
 : BC_TextBox(x, y, 100, text)
{ this->pwindow = pwindow; }

int PlaybackModuleFragment::handle_event() 
{ 
	pwindow->preferences->audio_module_fragment = atol(get_text()); 
return 0;
}


PlaybackPreload::PlaybackPreload(int x, int y, PreferencesWindow *pwindow, PlaybackPrefs *playback, char *text)
 : BC_TextBox(x, y, 100, text)
{ this->pwindow = pwindow; this->playback = playback; }

int PlaybackPreload::handle_event() 
{ 
	pwindow->preferences->playback_preload = atol(get_text()); 
return 0;
}





PlaybackBufferSize::PlaybackBufferSize(int x, int y, PreferencesWindow *pwindow, PlaybackPrefs *playback, char *text)
 : BC_TextBox(x, y, 100, text)
{ this->pwindow = pwindow; this->playback = playback; }

int PlaybackBufferSize::handle_event() 
{ 
	pwindow->preferences->playback_buffer = atol(get_text()); 

	//playback->buffer_bytes->update_bytes();
return 0;
}

PlaybackBufferBytes::PlaybackBufferBytes(int x, int y, PreferencesWindow *pwindow, PlaybackPrefs *playback, char *text)
 : BC_Title(x, y, text)
{
	this->pwindow = pwindow; this->playback = playback;
}
int PlaybackBufferBytes::update_bytes()
{
	sprintf(string, "%d", playback->get_buffer_bytes());
	update(string);
return 0;
}

PlaybackDisableNoEdits::PlaybackDisableNoEdits(PreferencesWindow *pwindow, int value, int y)
 : BC_CheckBox(10, y, 16, 16, value, "Disable tracks when no edits.")
{ this->pwindow = pwindow; }

int PlaybackDisableNoEdits::handle_event() { pwindow->preferences->test_playback_edits = get_value(); return 0;
}




PlaybackViewFollows::PlaybackViewFollows(PreferencesWindow *pwindow, int value, int y)
 : BC_CheckBox(10, y, 16, 16, value, "View follows playback")
{ this->pwindow = pwindow; }

int PlaybackViewFollows::handle_event() { pwindow->preferences->view_follows_playback = get_value(); return 0;
}




PlaybackSoftwareTimer::PlaybackSoftwareTimer(PreferencesWindow *pwindow, int value, int y)
 : BC_CheckBox(10, y, 16, 16, value, "Use software for positioning information")
{ this->pwindow = pwindow; }

int PlaybackSoftwareTimer::handle_event() { pwindow->preferences->playback_software_timer = get_value(); return 0;
}




PlaybackRealTime::PlaybackRealTime(PreferencesWindow *pwindow, int value, int y)
 : BC_CheckBox(10, y, 16, 16, value, "Audio playback in real time priority (root only)")
{ this->pwindow = pwindow; }

int PlaybackRealTime::handle_event() { pwindow->preferences->real_time_playback = get_value(); return 0;
}
