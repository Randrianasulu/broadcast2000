#ifndef PLUGINPOPUP_H
#define PLUGINPOPUP_H

class PluginPopupAttach;
class PluginPopupDetach;
class PluginPopupIn;
class PluginPopupOut;
class PluginPopupThru;

#include "bcbase.h"
#include "plugin.inc"
#include "plugindialog.inc"

class PluginPopup : public BC_PopupMenu
{
public:
	PluginPopup(Plugin *plugin, int x, int y);
	~PluginPopup();

	int update(int in, int out, /*int thru, */char* title);
	int add_items();  // called by BC_PopupMenu
	int handle_event();

	PluginPopupAttach *attach;
	PluginPopupDetach *detach;
	PluginPopupIn *in;
	PluginPopupOut *out;
	PluginPopupThru *thru;

	Plugin *plugin;
};

class PluginPopupAttach : public BC_PopupItem
{
public:
	PluginPopupAttach(Plugin *plugin);
	~PluginPopupAttach();

	int handle_event();
	Plugin *plugin;
	PluginPopup *plugin_popup;
	PluginDialogThread *dialog_thread;
};

class PluginPopupDetach : public BC_PopupItem
{
public:
	PluginPopupDetach(Plugin *plugin);
	~PluginPopupDetach();

	int handle_event();
	Plugin *plugin;
};


class PluginPopupIn : public BC_PopupItem
{
public:
	PluginPopupIn(Plugin *plugin);
	~PluginPopupIn();

	int handle_event();
	Plugin *plugin;
};

class PluginPopupOut : public BC_PopupItem
{
public:
	PluginPopupOut(Plugin *plugin);
	~PluginPopupOut();

	int handle_event();
	Plugin *plugin;
};

class PluginPopupThru : public BC_PopupItem
{
public:
	PluginPopupThru(Plugin *plugin);
	~PluginPopupThru();

	int handle_event();
	Plugin *plugin;
};


#endif
