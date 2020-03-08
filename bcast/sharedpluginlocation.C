#include <string.h>
#include "filehtal.h"
#include "sharedpluginlocation.h"


// plugin locations
SharedPluginLocation::SharedPluginLocation()
{
}

SharedPluginLocation::SharedPluginLocation(int module, int plugin)
{
	this->module = module;
	this->plugin = plugin;
}

int SharedPluginLocation::save(FileHTAL *htal)
{
	htal->tag.set_title("SHARED");
	htal->tag.set_property("MODULE", (long)module);
	htal->tag.set_property("PLUGIN", (long)plugin);
	htal->append_tag();
return 0;
}

int SharedPluginLocation::load(FileHTAL *htal, int track_offset)
{
	module = htal->tag.get_property("MODULE", (long)0) + track_offset;
	plugin = htal->tag.get_property("PLUGIN", (long)0);
return 0;
}

int SharedPluginLocation::operator==(const SharedPluginLocation &that)
{
	if(
		module == that.module &&
		plugin == that.plugin
	) return 1;
	else
	return 0;
return 0;
}




SharedModuleLocation::SharedModuleLocation()
{
}

SharedModuleLocation::SharedModuleLocation(int module)
{
	this->module = module;
}

int SharedModuleLocation::save(FileHTAL *htal)
{
//printf("SharedModuleLocation::save module %d\n", module);
	htal->tag.set_title("MODULE");
	htal->tag.set_property("MODULE", (long)module);
	htal->append_tag();
return 0;
}

int SharedModuleLocation::load(FileHTAL *htal, int track_offset)
{
	module = htal->tag.get_property("MODULE", (long)0) + track_offset;
return 0;
}

int SharedModuleLocation::operator==(const SharedModuleLocation &that)
{
	if(
		module == that.module
	) return 1;
	else
	return 0;
return 0;
}
