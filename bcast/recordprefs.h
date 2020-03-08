#ifndef RECORDPREFS_H
#define RECORDPREFS_H

class DuplexEnable;
class RecordMinDB;
class RecordVUDB;
class RecordVUInt;
class RecordWriteLength;
class RecordRealTime;

#include "preferencesthread.h"


class RecordPrefs : public PreferencesDialog
{
public:
	RecordPrefs(PreferencesWindow *pwindow);
	~RecordPrefs();

	int create_objects();

	ADevicePrefs *in_device, *duplex_device;
};


class RecordWriteLength : public BC_TextBox
{
public:
	RecordWriteLength(int x, int y, PreferencesWindow *pwindow, char *text);
	int handle_event();
	PreferencesWindow *pwindow;
};

class DuplexEnable : public BC_CheckBox
{
public:
	DuplexEnable(int x, int y, PreferencesWindow *pwindow, int value);
	int handle_event();
	PreferencesWindow *pwindow;
};

class RecordRealTime : public BC_CheckBox
{
public:
	RecordRealTime(int x, int y, PreferencesWindow *pwindow, int value);
	int handle_event();
	PreferencesWindow *pwindow;
};

#endif
