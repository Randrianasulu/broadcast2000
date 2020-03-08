#include <string.h>
#include "assets.h"
#include "defaults.h"
#include "file.h"
#include "mainmenu.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "menuveffects.h"
#include "patchbay.h"
#include "quicktime.h"
#include "vpluginarray.h"



MenuVEffects::MenuVEffects(MainWindow *mwindow)
 : MenuEffects(mwindow)
{
	thread = new MenuVEffectThread(mwindow);
}

MenuVEffects::~MenuVEffects()
{
	delete thread;
}

MenuVEffectThread::MenuVEffectThread(MainWindow *mwindow)
 : MenuEffectThread(mwindow)
{
}

MenuVEffectThread::~MenuVEffectThread()
{
}

int MenuVEffectThread::get_recordable_tracks(Asset *asset)
{
//	asset->layers = mwindow->patches->total_recordable_vtracks();
	asset->layers = mwindow->patches->total_playable_vtracks();
	return asset->layers;
return 0;
}

int MenuVEffectThread::get_derived_attributes(Asset *asset, Defaults *defaults)
{
	char string[1024];
	File file;
	defaults->get("VEFFECTPATH", asset->path);
	sprintf(string, MOV_NAME);
	defaults->get("VEFFECTFORMAT", string);
	if(!file.supports_video(mwindow->plugindb, string)) sprintf(string, MOV_NAME);
	asset->format = file.strtoformat(mwindow->plugindb, string);
	sprintf(asset->compression, QUICKTIME_YUV2);
	defaults->get("VEFFECTCOMPRESSION", asset->compression);
	asset->quality = defaults->get("VEFFECTQUALITY", 100);
	asset->width = mwindow->track_w;
	asset->height = mwindow->track_h;
	asset->video_data = 1;
return 0;
}

int MenuVEffectThread::save_derived_attributes(Asset *asset, Defaults *defaults)
{
	File file;
	defaults->update("VEFFECTPATH", asset->path);
	defaults->update("VEFFECTFORMAT", file.formattostr(mwindow->plugindb, asset->format));
	defaults->update("VEFFECTCOMPRESSION", asset->compression);
	defaults->update("VEFFECTQUALITY", asset->quality);
return 0;
}

PluginArray* MenuVEffectThread::create_plugin_array(MainWindow *mwindow)
{
	return new VPluginArray(mwindow);
}

int MenuVEffectThread::convert_units(long &start, long &end)
{
	start = toframes_round(start, mwindow->sample_rate, mwindow->frame_rate);
	end = toframes_round(end, mwindow->sample_rate, mwindow->frame_rate);
return 0;
}

int MenuVEffectThread::fix_menu(char *title)
{
	mwindow->gui->mainmenu->add_veffect(title); 
return 0;
}

MenuVEffectItem::MenuVEffectItem(MenuVEffects *menueffect, char *string)
 : MenuEffectItem(menueffect, string)
{
}
