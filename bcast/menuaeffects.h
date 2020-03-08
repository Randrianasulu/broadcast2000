#ifndef MENUAEFFECTS_H
#define MENUAEFFECTS_H

#include "assets.inc"
#include "bcbase.h"
#include "mainwindow.inc"
#include "menueffects.h"

class MenuAEffects : public MenuEffects
{
public:
	MenuAEffects(MainWindow *mwindow);
	~MenuAEffects();
};

class MenuAEffectThread : public MenuEffectThread
{
public:
	MenuAEffectThread(MainWindow *mwindow);
	~MenuAEffectThread();

	int get_recordable_tracks(Asset *asset);
	int get_derived_attributes(Asset *asset, Defaults *defaults);
	int save_derived_attributes(Asset *asset, Defaults *defaults);
	PluginArray* create_plugin_array(MainWindow *mwindow);
	int convert_units(long &start, long &end);
	int fix_menu(char *title);
};


class MenuAEffectItem : public MenuEffectItem
{
public:
	MenuAEffectItem(MenuAEffects *menueffect, char *string);
};




#endif
