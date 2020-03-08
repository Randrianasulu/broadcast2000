#include <string.h>
#include "autoconf.h"
#include "defaults.h"

int AutoConf::load_defaults(Defaults* defaults)
{
	char string[1024];
	int i;

	fade = defaults->get("AUTOFADE", 0);
	play = defaults->get("AUTOPLAY", 0);
	mute = defaults->get("AUTOMUTE", 0);
	camera = defaults->get("CAMERA", 0);
	projector = defaults->get("PROJECTOR", 0);
	for(i = 0; i < MAXCHANNELS; i++)
	{
		sprintf(string, "AUTOPAN%d", i);
		pan[i] = defaults->get(string, 0);
	}
	for(i = 0; i < PLUGINS; i++)
	{
		sprintf(string, "AUTOPLUGIN%d", i);
		plugin[i] = defaults->get(string, 0);
	}
return 0;
}

int AutoConf::save_defaults(Defaults* defaults)
{
	char string[1024];
	int i;

	defaults->update("AUTOFADE", fade);
	defaults->update("AUTOPLAY", play);
	defaults->update("AUTOMUTE", mute);
	defaults->update("CAMERA", camera);
	defaults->update("PROJECTOR", projector);
	for(i = 0; i < MAXCHANNELS; i++)
	{
		sprintf(string, "AUTOPAN%d", i);
		defaults->update(string, pan[i]);
	}
	for(i = 0; i < PLUGINS; i++)
	{
		sprintf(string, "AUTOPLUGIN%d", i);
		defaults->update(string, plugin[i]);
	}
return 0;
}

int AutoConf::set_all()
{
	int i;
	fade = 1;
	play = 1;
	mute = 1;
	camera = 1;
	projector = 1;
	for(i = 0; i < MAXCHANNELS; i++)
	{
		pan[i] = 1;
	}
	for(i = 0; i < PLUGINS; i++)
	{
		plugin[i] = 1;
	}
return 0;
}
