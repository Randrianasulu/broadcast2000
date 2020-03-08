#ifndef SHAREDPLUGINLOCATION_H
#define SHAREDPLUGINLOCATION_H

#include "filehtal.inc"

class SharedPluginLocation
{
public:
	SharedPluginLocation();
	SharedPluginLocation(int module, int plugin);

	int save(FileHTAL *htal);
	int load(FileHTAL *htal, int track_offset);
	int operator==(const SharedPluginLocation &that);

	int module, plugin;
};

class SharedModuleLocation
{
public:
	SharedModuleLocation();
	SharedModuleLocation(int module);

	int save(FileHTAL *htal);
	int load(FileHTAL *htal, int track_offset);
	int operator==(const SharedModuleLocation &that);

	int module;
};

#endif
