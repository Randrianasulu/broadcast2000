#ifndef MENUVEFFECTS_H
#define MENUVEFFECTS_H

#include "assets.inc"
#include "mainwindow.inc"
#include "menueffects.h"

class MenuVEffects : public MenuEffects
{
public:
	MenuVEffects(MainWindow *mwindow);
	~MenuVEffects();
};

class MenuVEffectThread : public MenuEffectThread
{
public:
	MenuVEffectThread(MainWindow *mwindow);
	~MenuVEffectThread();

	int get_recordable_tracks(Asset *asset);
	int get_derived_attributes(Asset *asset, Defaults *defaults);
	int save_derived_attributes(Asset *asset, Defaults *defaults);
	PluginArray* create_plugin_array(MainWindow *mwindow);
	int fix_menu(char *title);

	int convert_units(long &start, long &end);
};

class MenuVEffectItem : public MenuEffectItem
{
public:
	MenuVEffectItem(MenuVEffects *menueffect, char *string);
};

#endif
