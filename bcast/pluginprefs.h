#ifndef PLUGINPREFS_H
#define PLUGINPREFS_H

class PluginGlobalPath;
class PluginGlobalPathText;
class PluginLocalPath;
class PluginLocalPathText;

#include "browsebutton.h"
#include "preferencesthread.h"

class PluginPrefs : public PreferencesDialog
{
public:
	PluginPrefs(PreferencesWindow *pwindow);
	~PluginPrefs();
	
	int create_objects();
// must delete each derived class
	PluginGlobalPath *ipath;
	PluginGlobalPathText *ipathtext;
	PluginLocalPath *lpath;
	PluginLocalPathText *lpathtext;
};





class PluginGlobalPath : public BrowseButton
{
public:
	PluginGlobalPath(int x, int y, PreferencesWindow *pwindow, BC_TextBox *textbox, char *text);
	~PluginGlobalPath();
	
	PreferencesWindow *pwindow;
};



class PluginGlobalPathText : public BC_TextBox
{
public:
	PluginGlobalPathText(int x, int y, PreferencesWindow *pwindow, char *text);
	~PluginGlobalPathText();
	int handle_event();
	PreferencesWindow *pwindow;
};






class PluginLocalPath : public BrowseButton
{
public:
	PluginLocalPath(int x, int y, PreferencesWindow *pwindow, BC_TextBox *textbox, char *text);
	~PluginLocalPath();
	
	PreferencesWindow *pwindow;
};



class PluginLocalPathText : public BC_TextBox
{
public:
	PluginLocalPathText(int x, int y, PreferencesWindow *pwindow, char *text);
	~PluginLocalPathText();
	int handle_event();
	PreferencesWindow *pwindow;
};

#endif
