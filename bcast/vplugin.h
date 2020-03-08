#ifndef VPLUGIN_H
#define VPLUGIN_H

#include "mainwindow.inc"
#include "plugin.h"
#include "vmodule.inc"

class VPlugin : public Plugin
{
public:
	VPlugin(MainWindow *mwindow, VModule *vmodule, int plugin_number);
	~VPlugin();

	int create_objects(int x, int y);
	int use_gui();       // whether or not the module has a gui

	VModule *vmodule;
};




#endif
