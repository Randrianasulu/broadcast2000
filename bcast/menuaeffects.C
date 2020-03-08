#include <string.h>
#include "apluginarray.h"
#include "assets.h"
#include "file.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "menuaeffects.h"
#include "patchbay.h"

// ============================================= audio effects

MenuAEffects::MenuAEffects(MainWindow *mwindow)
 : MenuEffects(mwindow)
{
	thread = new MenuAEffectThread(mwindow);
}

MenuAEffects::~MenuAEffects()
{
	delete thread;
}

MenuAEffectThread::MenuAEffectThread(MainWindow *mwindow)
 : MenuEffectThread(mwindow)
{
}

MenuAEffectThread::~MenuAEffectThread()
{
}

int MenuAEffectThread::get_recordable_tracks(Asset *asset)
{
//	asset->channels = mwindow->patches->total_recordable_atracks();
	asset->channels = mwindow->patches->total_playable_atracks();
	return asset->channels;
return 0;
}


int MenuAEffectThread::get_derived_attributes(Asset *asset, Defaults *defaults)
{
	char string[1024];
	File file;
	defaults->get("AEFFECTPATH", asset->path);
	sprintf(string, "WAV");
	defaults->get("AEFFECTFORMAT", string);
	if(!file.supports_audio(mwindow->plugindb, string)) sprintf(string, "WAV");
	asset->format = file.strtoformat(mwindow->plugindb, string);
	asset->rate = mwindow->sample_rate;
	asset->bits = defaults->get("AEFFECTBITS", 16);
	dither = defaults->get("AEFFECTDITHER", 0);
	asset->signed_ = defaults->get("AEFFECTSIGNED", 1);
	asset->byte_order = defaults->get("AEFFECTBYTEORDER", 1);
	asset->audio_data = 1;
return 0;
}

int MenuAEffectThread::save_derived_attributes(Asset *asset, Defaults *defaults)
{
	File file;
	defaults->update("AEFFECTPATH", asset->path);
	defaults->update("AEFFECTFORMAT", file.formattostr(mwindow->plugindb, asset->format));
	defaults->update("AEFFECTBITS", asset->bits);
	defaults->update("AEFFECTDITHER", dither);
	defaults->update("AEFFECTSIGNED", asset->signed_);
	defaults->update("AEFFECTBYTEORDER", asset->byte_order);
return 0;
}


PluginArray* MenuAEffectThread::create_plugin_array(MainWindow *mwindow)
{
	return new APluginArray(mwindow);
}

int MenuAEffectThread::convert_units(long &start, long &end)
{
return 0;
}

int MenuAEffectThread::fix_menu(char *title)
{
	mwindow->gui->mainmenu->add_aeffect(title); 
return 0;
}



MenuAEffectItem::MenuAEffectItem(MenuAEffects *menueffect, char *string)
 : MenuEffectItem(menueffect, string)
{
}
