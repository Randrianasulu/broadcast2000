#ifndef APLUGIN_H
#define APLUGIN_H

#include "amodule.inc"
#include "mainwindow.inc"
#include "plugin.h"

class APlugin : public Plugin
{
public:
	APlugin(MainWindow *mwindow, AModule *amodule, int plugin_number);
	~APlugin();

	int create_objects(int x, int y);
	int use_gui();

	AModule *amodule;
};

#endif
