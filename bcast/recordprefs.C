#include <string.h>
#include "adeviceprefs.h"
#include "adrivermenu.h"
#include "audioconfig.h"
#include "preferences.h"
#include "recordprefs.h"

RecordPrefs::RecordPrefs(PreferencesWindow *pwindow)
 : PreferencesDialog(pwindow)
{
}

RecordPrefs::~RecordPrefs()
{
	delete in_device;
	delete duplex_device;
}

int RecordPrefs::create_objects()
{
	int x = 10, y = 10;
	char string[1024];

	add_border(get_resources()->get_bg_shadow1(),
		get_resources()->get_bg_shadow2(),
		get_resources()->get_bg_color(),
		get_resources()->get_bg_light2(),
		get_resources()->get_bg_light1());
	add_tool(new BC_Title(x, y, "Audio In", LARGEFONT, BLACK));
	y += 35;

	in_device = new ADevicePrefs(x + 135, 
		y, 
		pwindow, 
		this, 
		MODERECORD);
	in_device->initialize();

	add_tool(new BC_Title(x, y, "Record Driver", MEDIUMFONT, BLACK));
	add_tool(new AudioDriverMenu(x, y + 20, in_device, &(pwindow->preferences->aconfig->audio_in_driver), 1, 0));

	y += 70;
	duplex_device = new ADevicePrefs(x + 135, 
		y, 
		pwindow, 
		this, 
		MODEDUPLEX);
	duplex_device->initialize();

	add_tool(new BC_Title(x, y, "Duplex Driver", MEDIUMFONT, BLACK));
	add_tool(new AudioDriverMenu(x, y + 20, duplex_device, &(pwindow->preferences->aconfig->audio_duplex_driver), 0, 1));

	y += 70;
	add_tool(new BC_Title(x, y, "Samples to write to disk at a time"));
	y += 20;
	sprintf(string, "%ld", pwindow->preferences->record_write_length);
	add_tool(new RecordWriteLength(x, y, pwindow, string));

	y += 35;
	add_tool(new DuplexEnable(x, y, pwindow, pwindow->preferences->enable_duplex));
	y += 20;
	add_tool(new RecordRealTime(x, y, pwindow, pwindow->preferences->real_time_record));
	return 0;
return 0;
}

RecordWriteLength::RecordWriteLength(int x, int y, PreferencesWindow *pwindow, char *text)
 : BC_TextBox(x, y, 100, text)
{ this->pwindow = pwindow; }

int RecordWriteLength::handle_event()
{ pwindow->preferences->record_write_length = atol(get_text()); return 0;
}




RecordRealTime::RecordRealTime(int x, int y, PreferencesWindow *pwindow, int value)
 : BC_CheckBox(x, y, 16, 16, value, "Record in realtime priority (root only)")
{ this->pwindow = pwindow; }

int RecordRealTime::handle_event()
{
	pwindow->preferences->real_time_record = get_value();
return 0;
}

DuplexEnable::DuplexEnable(int x, int y, PreferencesWindow *pwindow, int value)
 : BC_CheckBox(x, y, 16, 16, value, "Enable full duplex")
{ this->pwindow = pwindow; }

int DuplexEnable::handle_event()
{
	pwindow->preferences->enable_duplex = get_value();
return 0;
}
