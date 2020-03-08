#ifndef INDEXPREFS_H
#define INDEXPREFS_H

class IndexPath;
class IndexSize;
class IndexCount;
class IndexPathText;
class CacheSize;
class PrefsSMP;

#include "browsebutton.h"
#include "deleteallindexes.inc"
#include "preferencesthread.h"

class IndexPrefs : public PreferencesDialog
{
public:
	IndexPrefs(PreferencesWindow *pwindow);
	~IndexPrefs();
	
	int create_objects();
// must delete each derived class
	IndexPath *ipath;
	IndexSize *isize;
	IndexCount *icount;
	IndexPathText *ipathtext;
	CacheSize *csize;
	DeleteAllIndexes *deleteall;
	PrefsSMP *smp;
};





class IndexPath : public BrowseButton
{
public:
	IndexPath(int x, int y, PreferencesWindow *pwindow, BC_TextBox *textbox, char *text);
	~IndexPath();
	
	PreferencesWindow *pwindow;
};





class PrefsSMP : public BC_TextBox
{
public:
	PrefsSMP(int x, int y, PreferencesWindow *pwindow);
	~PrefsSMP();

	int handle_event();
	PreferencesWindow *pwindow;
};


class IndexPathText : public BC_TextBox
{
public:
	IndexPathText(int x, int y, PreferencesWindow *pwindow, char *text);
	~IndexPathText();
	int handle_event();
	PreferencesWindow *pwindow;
};

class CacheSize : public BC_TextBox
{
public:
	CacheSize(int x, int y, PreferencesWindow *pwindow, char *text);
	int handle_event();
	PreferencesWindow *pwindow;
};



class IndexSize : public BC_TextBox
{
public:
	IndexSize(int x, int y, PreferencesWindow *pwindow, char *text);
	int handle_event();
	PreferencesWindow *pwindow;
};


class IndexCount : public BC_TextBox
{
public:
	IndexCount(int x, int y, PreferencesWindow *pwindow, char *text);
	int handle_event();
	PreferencesWindow *pwindow;
};


#endif
