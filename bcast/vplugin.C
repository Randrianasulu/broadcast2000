#include <string.h>
#include "modules.h"
#include "pluginpopup.h"
#include "vmodule.h"
#include "vplugin.h"


// plugin

VPlugin::VPlugin(MainWindow *mwindow, VModule *vmodule, int plugin_number)
 : Plugin(mwindow, vmodule, plugin_number)
{
	this->vmodule = vmodule;
}

VPlugin::~VPlugin()
{
}

int VPlugin::create_objects(int x, int y)
{
	if(vmodule->gui)
	{
		vmodule->gui->add_tool(plugin_popup = new PluginPopup(this, x, y));
		vmodule->gui->add_tool(show_toggle = new PluginShowToggle(this, vmodule->modules->console, x + 10, y + 23));
		vmodule->gui->add_tool(show_title = new BC_Title(x + 30, y + 25, "Show", SMALLFONT));
		vmodule->gui->add_tool(on_toggle = new PluginOnToggle(this, vmodule->modules->console, x + 60, y + 23));
		vmodule->gui->add_tool(on_title = new BC_Title(x + 80, y + 25, "On", SMALLFONT));
	}
return 0;
}

int VPlugin::use_gui()
{
	return (vmodule->gui != 0);
return 0;
}

