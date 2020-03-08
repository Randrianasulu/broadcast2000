#include <string.h>
#include "pluginprefs.h"
#include "preferences.h"

PluginPrefs::PluginPrefs(PreferencesWindow *pwindow)
 : PreferencesDialog(pwindow)
{
}

PluginPrefs::~PluginPrefs()
{
	delete ipath;
	delete ipathtext;
	delete lpath;
	delete lpathtext;
}

int PluginPrefs::create_objects()
{
	char string[1024];
	int x = 10, y = 10;

	add_border(get_resources()->get_bg_shadow1(),
		get_resources()->get_bg_shadow2(),
		get_resources()->get_bg_color(),
		get_resources()->get_bg_light2(),
		get_resources()->get_bg_light1());

	add_tool(new BC_Title(10, y, "Plugins", LARGEFONT, BLACK));
	y += 35;
	add_tool(new BC_Title(10, y, "Look for global plugins here", MEDIUMFONT, BLACK));
	y += 20;
	add_tool(ipathtext = new PluginGlobalPathText(10, y, pwindow, pwindow->preferences->global_plugin_dir));
	add_tool(ipath = new PluginGlobalPath(215, y, pwindow, ipathtext, pwindow->preferences->global_plugin_dir));
	y += 35;
	add_tool(new BC_Title(10, y, "Look for personal plugins here", MEDIUMFONT, BLACK));
	y += 20;
	add_tool(lpathtext = new PluginLocalPathText(10, y, pwindow, pwindow->preferences->local_plugin_dir));
	add_tool(lpath = new PluginLocalPath(215, y, pwindow, lpathtext, pwindow->preferences->local_plugin_dir));
	return 0;
return 0;
}






PluginGlobalPath::PluginGlobalPath(int x, int y, PreferencesWindow *pwindow, BC_TextBox *textbox, char *text)
 : BrowseButton(x, y, textbox, text, "Global Plugin Path", "Select the directory for plugins", 1)
{ this->pwindow = pwindow; }

PluginGlobalPath::~PluginGlobalPath() {}

PluginGlobalPathText::PluginGlobalPathText(int x, int y, PreferencesWindow *pwindow, char *text)
 : BC_TextBox(x, y, 200, text)
{ this->pwindow = pwindow; }

PluginGlobalPathText::~PluginGlobalPathText() {}

int PluginGlobalPathText::handle_event()
{
	strcpy(pwindow->preferences->global_plugin_dir, get_text());
return 0;
}




PluginLocalPath::PluginLocalPath(int x, int y, PreferencesWindow *pwindow, BC_TextBox *textbox, char *text)
 : BrowseButton(x, y, textbox, text, "Personal Plugin Path", "Select the directory for plugins", 1)
{ this->pwindow = pwindow; }

PluginLocalPath::~PluginLocalPath() {}

PluginLocalPathText::PluginLocalPathText(int x, int y, PreferencesWindow *pwindow, char *text)
 : BC_TextBox(x, y, 200, text)
{ this->pwindow = pwindow; }

PluginLocalPathText::~PluginLocalPathText() {}

int PluginLocalPathText::handle_event()
{
	strcpy(pwindow->preferences->local_plugin_dir, get_text());
return 0;
}
