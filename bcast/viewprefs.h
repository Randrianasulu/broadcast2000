#ifndef VIEWPREFS_H
#define VIEWPREFS_H

class TimeFormatHMS;
class TimeFormatHMSF;
class TimeFormatSamples;
class TimeFormatFrames;
class TimeFormatHex;
class TimeFormatFeet;
class MeterMinDB;
class MeterVUDB;
class MeterVUInt;
class ViewBehaviourText;

#include "preferencesthread.inc"


class ViewPrefs : public PreferencesDialog
{
public:
	ViewPrefs(PreferencesWindow *pwindow);
	~ViewPrefs();
	
	int create_objects();
// must delete each derived class
	int update(int new_value);
	char* behavior_to_text(int mode);
	
	TimeFormatHMS *hms;
	TimeFormatHMSF *hmsf;
	TimeFormatSamples *samples;
	TimeFormatHex *hex;
	TimeFormatFrames *frames;
	TimeFormatFeet *feet;
	MeterMinDB *min_db;
	MeterVUDB *vu_db;
	MeterVUInt *vu_int;
	ViewBehaviourText *button1, *button2, *button3;
};


class TimeFormatHMS : public BC_Radial
{
public:
	TimeFormatHMS(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y);
	int handle_event();
	PreferencesWindow *pwindow;
	ViewPrefs *tfwindow;
};

class TimeFormatHMSF : public BC_Radial
{
public:
	TimeFormatHMSF(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y);
	int handle_event();
	PreferencesWindow *pwindow;
	ViewPrefs *tfwindow;
};

class TimeFormatSamples : public BC_Radial
{
public:
	TimeFormatSamples(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y);
	int handle_event();
	PreferencesWindow *pwindow;
	ViewPrefs *tfwindow;
};

class TimeFormatFrames : public BC_Radial
{
public:
	TimeFormatFrames(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y);
	int handle_event();
	PreferencesWindow *pwindow;
	ViewPrefs *tfwindow;
};

class TimeFormatHex : public BC_Radial
{
public:
	TimeFormatHex(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y);
	int handle_event();
	PreferencesWindow *pwindow;
	ViewPrefs *tfwindow;
};

class TimeFormatFeet : public BC_Radial
{
public:
	TimeFormatFeet(PreferencesWindow *pwindow, ViewPrefs *tfwindow, int value, int y);
	int handle_event();
	PreferencesWindow *pwindow;
	ViewPrefs *tfwindow;
};

class TimeFormatFeetSetting : public BC_TextBox
{
public:
	TimeFormatFeetSetting(PreferencesWindow *pwindow, int y, char *string);
	int handle_event();
	PreferencesWindow *pwindow;
};



class MeterMinDB : public BC_TextBox
{
public:
	MeterMinDB(PreferencesWindow *pwindow, char *text, int y);
	int handle_event();
	PreferencesWindow *pwindow;
};

class MeterVUDB : public BC_Radial
{
public:
	MeterVUDB(PreferencesWindow *pwindow, char *text, int y);
	int handle_event();
	MeterVUInt *vu_int;
	PreferencesWindow *pwindow;
};

class MeterVUInt : public BC_Radial
{
public:
	MeterVUInt(PreferencesWindow *pwindow, char *text, int y);
	int handle_event();
	MeterVUDB *vu_db;
	PreferencesWindow *pwindow;
};

class ViewBehaviourText : public BC_PopupMenu
{
public:
	ViewBehaviourText(int x, int y, char *text, PreferencesWindow *pwindow, int *output);
	~ViewBehaviourText();

	int handle_event();  // user copies text to value here
	int add_items();         // add initial items
	ViewPrefs *tfwindow;
	int *output;
};

class ViewBehaviourItem : public BC_PopupItem
{
public:
	ViewBehaviourItem(ViewBehaviourText *popup, char *text, int behaviour);
	~ViewBehaviourItem();

	int handle_event();
	ViewBehaviourText *popup;
	int behaviour;
};



#endif
