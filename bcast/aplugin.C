#include <string.h>
#include "amodule.h"
#include "aplugin.h"
#include "modules.h"
#include "pluginpopup.h"


// plugin

APlugin::APlugin(MainWindow *mwindow, AModule *amodule, int plugin_number)
 : Plugin(mwindow, amodule, plugin_number)
{
	this->amodule = amodule;
}

APlugin::~APlugin()
{
}

int APlugin::create_objects(int x, int y)
{
	if(amodule->gui)
	{
		amodule->gui->add_tool(plugin_popup = new PluginPopup(this, x, y));
		amodule->gui->add_tool(show_toggle = new PluginShowToggle(this, amodule->modules->console, x + 10, y + 23));
		amodule->gui->add_tool(show_title = new BC_Title(x + 30, y + 25, "Show", SMALLFONT));
		amodule->gui->add_tool(on_toggle = new PluginOnToggle(this, amodule->modules->console, x + 60, y + 23));
		amodule->gui->add_tool(on_title = new BC_Title(x + 80, y + 25, "On", SMALLFONT));
	}
return 0;
}

int APlugin::use_gui()
{
	return (amodule->gui != 0);
return 0;
}

