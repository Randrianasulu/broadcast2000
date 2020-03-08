#include <string.h>
#include "preferences.h"
#include "preferencesthread.h"
#include "viewprefs.h"

#define MOVE_ALL_EDITS_TITLE "Drag all following edits"
#define MOVE_ONE_EDIT_TITLE "Drag only one edit"
#define MOVE_NO_EDITS_TITLE "Drag source only"
#define MOVE_EDITS_DISABLED_TITLE "No effect"

ViewPrefs::ViewPrefs(PreferencesWindow *pwindow)
 : PreferencesDialog(pwindow)
{
}

int ViewPrefs::create_objects()
{
	int y = 10, value;
	char string[1024];
	add_border(get_resources()->get_bg_shadow1(),
		get_resources()->get_bg_shadow2(),
		get_resources()->get_bg_color(),
		get_resources()->get_bg_light2(),
		get_resources()->get_bg_light1());
	add_tool(new BC_Title(10, y, "Interface", LARGEFONT, BLACK));
	y += 35;
	add_tool(hms = new TimeFormatHMS(pwindow, this, pwindow->preferences->time_format == 0, y));
	y += 20;
	add_tool(hmsf = new TimeFormatHMSF(pwindow, this, pwindow->preferences->time_format == 1, y));
	y += 20;
	add_tool(samples = new TimeFormatSamples(pwindow, this, pwindow->preferences->time_format == 2, y));
	y += 20;
	add_tool(hex = new TimeFormatHex(pwindow, this, pwindow->preferences->time_format == 3, y));
	y += 20;
	add_tool(frames = new TimeFormatFrames(pwindow, this, pwindow->preferences->time_format == 4, y));
	y += 20;
	add_tool(feet = new TimeFormatFeet(pwindow, this, pwindow->preferences->time_format == 5, y));
	add_tool(new BC_Title(260, y, "frames per foot"));
	sprintf(string, "%0.2f", pwindow->preferences->frames_per_foot);
	add_tool(new TimeFormatFeetSetting(pwindow, y - 5, string));

	y += 35;
	add_tool(new BC_Title(10, y, "Clicking on in/out points does what:"));
	y += 25;
	add_tool(new BC_Title(10, y, "Button 1:"));
	add_tool(new ViewBehaviourText(80, y - 5, behavior_to_text(pwindow->preferences->edit_handle_mode[0]), pwindow, &(pwindow->preferences->edit_handle_mode[0])));
	y += 25;
	add_tool(new BC_Title(10, y, "Button 2:"));
	add_tool(new ViewBehaviourText(80, y - 5, behavior_to_text(pwindow->preferences->edit_handle_mode[1]), pwindow, &(pwindow->preferences->edit_handle_mode[1])));
	y += 25;
	add_tool(new BC_Title(10, y, "Button 3:"));
	add_tool(new ViewBehaviourText(80, y - 5, behavior_to_text(pwindow->preferences->edit_handle_mode[2]), pwindow, &(pwindow->preferences->edit_handle_mode[2])));

	y += 40;
	add_tool(new BC_Title(10, y, "Min DB for meter"));
	sprintf(string, "%.0f", pwindow->preferences->min_meter_db);
	add_tool(min_db = new MeterMinDB(pwindow, string, y));
	y += 30;
	add_tool(new BC_Title(10, y, "Format for meter"));
	add_tool(vu_db = new MeterVUDB(pwindow, "DB", y));
	add_tool(vu_int = new MeterVUInt(pwindow, "Percent of maximum", y));
	vu_db->vu_int = vu_int;
	vu_int->vu_db = vu_db;
return 0;
}

char* ViewPrefs::behavior_to_text(int mode)
{
	switch(mode)
	{
		case MOVE_ALL_EDITS:
			return MOVE_ALL_EDITS_TITLE;
			break;
		case MOVE_ONE_EDIT:
			return MOVE_ONE_EDIT_TITLE;
			break;
		case MOVE_NO_EDITS:
			return MOVE_NO_EDITS_TITLE;
			break;
		case MOVE_EDITS_DISABLED:
			return MOVE_EDITS_DISABLED_TITLE;
			break;
		default:
			return "";
			break;
	}
}

int ViewPrefs::update(int new_value)
{
	pwindow->preferences->time_format = new_value;
	hms->update(new_value == 0);
	hmsf->update(new_value == 1);
	samples->update(new_value == 2);
	hex->update(new_value == 3);
	frames->update(new_value == 4);
	feet->update(new_value == 5);
return 0;
}

ViewPrefs::~ViewPrefs()
{
	delete hms;
	delete hmsf;
	delete samples;
	delete frames;
	delete hex;
	delete feet;
	delete min_db;
	delete vu_db;
	delete vu_int;
}

TimeFormatHMS::TimeFormatHMS(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y)
 : BC_Radial(10, y, 16, 16, value, "Use Hours:Minutes:Seconds.xxx")
{ this->pwindow = pwindow; this->tfwindow = tfwindow; }

int TimeFormatHMS::handle_event()
{
	tfwindow->update(0);
return 0;
}

TimeFormatHMSF::TimeFormatHMSF(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y)
 : BC_Radial(10, y, 16, 16, value, "Use Hours:Minutes:Seconds:Frames")
{ this->pwindow = pwindow; this->tfwindow = tfwindow; }

int TimeFormatHMSF::handle_event()
{
	tfwindow->update(1);
return 0;
}

TimeFormatSamples::TimeFormatSamples(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y)
 : BC_Radial(10, y, 16, 16, value, "Use Samples")
{ this->pwindow = pwindow; this->tfwindow = tfwindow; }

int TimeFormatSamples::handle_event()
{
	tfwindow->update(2);
return 0;
}

TimeFormatFrames::TimeFormatFrames(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y)
 : BC_Radial(10, y, 16, 16, value, "Use Frames")
{ this->pwindow = pwindow; this->tfwindow = tfwindow; }

int TimeFormatFrames::handle_event()
{
	tfwindow->update(4);
return 0;
}

TimeFormatHex::TimeFormatHex(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y)
 : BC_Radial(10, y, 16, 16, value, "Use Hex Samples")
{ this->pwindow = pwindow; this->tfwindow = tfwindow; }

int TimeFormatHex::handle_event()
{
	tfwindow->update(3);
return 0;
}

TimeFormatFeet::TimeFormatFeet(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y)
 : BC_Radial(10, y, 16, 16, value, "Use Feet-frames")
{ this->pwindow = pwindow; this->tfwindow = tfwindow; }

int TimeFormatFeet::handle_event()
{
	tfwindow->update(5);
return 0;
}

TimeFormatFeetSetting::TimeFormatFeetSetting(PreferencesWindow *pwindow, int y, char *string)
 : BC_TextBox(160, y, 90, string)
{ this->pwindow = pwindow; }

int TimeFormatFeetSetting::handle_event()
{
	pwindow->preferences->frames_per_foot = atof(get_text());
	if(pwindow->preferences->frames_per_foot < 1) pwindow->preferences->frames_per_foot = 1;
return 0;
}




ViewBehaviourText::ViewBehaviourText(int x, int y, char *text, PreferencesWindow *pwindow, int *output)
 : BC_PopupMenu(x, y, 200, text)
{
	this->output = output;
}

ViewBehaviourText::~ViewBehaviourText()
{
}

int ViewBehaviourText::handle_event()
{
return 0;
}

int ViewBehaviourText::add_items()
{
// Video4linux versions are automatically detected
	add_item(new ViewBehaviourItem(this, MOVE_ALL_EDITS_TITLE, MOVE_ALL_EDITS));
	add_item(new ViewBehaviourItem(this, MOVE_ONE_EDIT_TITLE, MOVE_ONE_EDIT));
	add_item(new ViewBehaviourItem(this, MOVE_NO_EDITS_TITLE, MOVE_NO_EDITS));
	add_item(new ViewBehaviourItem(this, MOVE_EDITS_DISABLED_TITLE, MOVE_EDITS_DISABLED));
return 0;
}


ViewBehaviourItem::ViewBehaviourItem(ViewBehaviourText *popup, char *text, int behaviour)
 : BC_PopupItem(text)
{
	this->popup = popup;
	this->behaviour = behaviour;
}

ViewBehaviourItem::~ViewBehaviourItem()
{
}

int ViewBehaviourItem::handle_event()
{
	popup->update(get_text());
	*(popup->output) = behaviour;
return 0;
}




MeterMinDB::MeterMinDB(PreferencesWindow *pwindow, char *text, int y)
 : BC_TextBox(145, y, 50, text)
{ this->pwindow = pwindow; }

int MeterMinDB::handle_event()
{ pwindow->preferences->min_meter_db = atol(get_text()); return 0;
}






MeterVUDB::MeterVUDB(PreferencesWindow *pwindow, char *text, int y)
 : BC_Radial(145, y, 16, 16, pwindow->preferences->meter_format == METER_DB, text)
{ this->pwindow = pwindow; }

int MeterVUDB::handle_event() { vu_int->update(0); pwindow->preferences->meter_format = METER_DB; return 0;
}

MeterVUInt::MeterVUInt(PreferencesWindow *pwindow, char *text, int y)
 : BC_Radial(205, y, 16, 16, pwindow->preferences->meter_format == METER_INT, text)
{ this->pwindow = pwindow; }

int MeterVUInt::handle_event() { vu_db->update(0); pwindow->preferences->meter_format = METER_INT; return 0;
}
