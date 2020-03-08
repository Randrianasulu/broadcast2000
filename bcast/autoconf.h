#ifndef AUTOCONF_H
#define AUTOCONF_H

#include "defaults.inc"
#include "maxchannels.h"
#include "module.inc"

class AutoConf
{
public:
	AutoConf() { };
	~AutoConf() { };

	int load_defaults(Defaults* defaults);
	int save_defaults(Defaults* defaults);
	int set_all();  // set all parameters to 1

	int fade;
	int pan[MAXCHANNELS];
	int plugin[PLUGINS];
	int play;
	int mute;
	int camera;
	int projector;
};

#endif
