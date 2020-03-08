#ifndef GRAPHICWINDOW_H
#define GRAPHICWINDOW_H

class GraphicWindow;
class GraphicThread;

#include "bcbase.h"
#include "graphic.h"
#include "mutex.h"

#define TOTAL_DIVISIONS 6
#define TOTAL_LOADS 5

class GraphicThread : public Thread
{
public:
	GraphicThread(Graphic *plugin);
	~GraphicThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	Graphic *plugin;
	GraphicWindow *window;
};

class GraphicCanvas;
class GraphicWindowSize;
class GraphicMenu;

class GraphicWindow : public BC_Window
{
public:
	GraphicWindow(Graphic *plugin);
	~GraphicWindow();
	
	int create_objects();
	int get_divisions(int pixels, char **titles, int *title_x);
	int close_event();

	Graphic *plugin;
	GraphicCanvas *canvas;
	GraphicWindowSize *windowsize;
	GraphicMenu *menu;
};

class GraphicCanvas : public BC_Canvas
{
public:
	GraphicCanvas(Graphic *plugin, int x, int y, int w, int h);
	~GraphicCanvas();

	int handle_event();
	int button_release(); 
	int button_press();
	int cursor_motion();
	
	int draw_floating(int flash);
	int draw_envelope();
	int get_point(int point, int &x, int &y);
	int get_values(int x, int y, float &amount, long &freq);
	int test_points();
	int test_point(int x, int y);
	int test_lines();
	int test_line(int x1, int y1, int x2, int y2);
	int insert_point(int number, float new_amount, long new_freq);
	int delete_point(int selected_point);
	
	int draw_point(int x, int y);
	int selected_point;
	Graphic *plugin;
};

class GraphicWindowSize : public BC_TextBox
{
public:
	GraphicWindowSize(Graphic *plugin, int x, int y);
	~GraphicWindowSize();
	int handle_event();
	Graphic *plugin;
};

class GraphicClear : public BC_BigButton
{
public:
	GraphicClear(Graphic *plugin, int x, int y);
	~GraphicClear();
	int handle_event();
	Graphic *plugin;
};


class GraphicLoad;
class GraphicSave;
class GraphicSetDefault;
class GraphicLoadPrev;
class GraphicLoadPrevThread;

class GraphicMenu : public BC_MenuBar
{
public:
	GraphicMenu(Graphic *plugin, GraphicWindow *window);
	~GraphicMenu();
	
	int create_objects(Defaults *defaults);
	int load_defaults(Defaults *defaults);
	int save_defaults(Defaults *defaults);
// most recent loads
	int add_load(char *path);
	GraphicLoadPrevThread *prev_load_thread;
	
	int total_loads;
	BC_Menu *filemenu;
	GraphicWindow *window;
	Graphic *plugin;
	GraphicLoad *load;
	GraphicSave *save;
	GraphicSetDefault *set_default;
	GraphicLoadPrev *prev_load[TOTAL_LOADS];
};

class GraphicSaveThread;
class GraphicLoadThread;

class GraphicLoad : public BC_MenuItem
{
public:
	GraphicLoad(Graphic *plugin, GraphicMenu *menu);
	~GraphicLoad();
	int handle_event();
	Graphic *plugin;
	GraphicLoadThread *thread;
	GraphicMenu *menu;
};

class GraphicSave : public BC_MenuItem
{
public:
	GraphicSave(Graphic *plugin, GraphicMenu *menu);
	~GraphicSave();
	int handle_event();
	Graphic *plugin;
	GraphicSaveThread *thread;
	GraphicMenu *menu;
};

class GraphicSetDefault : public BC_MenuItem
{
public:
	GraphicSetDefault();
	int handle_event();
};

class GraphicLoadPrev : public BC_MenuItem
{
public:
	GraphicLoadPrev(Graphic *plugin, GraphicMenu *menu, char *filename, char *path);
	GraphicLoadPrev(Graphic *plugin, GraphicMenu *menu);
	int handle_event();
	int set_path(char *path);
	char path[1024];
	Graphic *plugin;
	GraphicMenu *menu;
};


class GraphicLoadPrevThread : public Thread
{
public:
	GraphicLoadPrevThread(Graphic *plugin, GraphicMenu *menu);
	~GraphicLoadPrevThread();
	void run();
	int set_path(char *path);
	char path[1024];
	Graphic *plugin;
	GraphicMenu *menu;
};



class GraphicSaveThread : public Thread
{
public:
	GraphicSaveThread(Graphic *plugin, GraphicMenu *menu);
	~GraphicSaveThread();
	void run();
	Graphic *plugin;
	GraphicMenu *menu;
};

class GraphicSaveDialog : public BC_FileBox
{
public:
	GraphicSaveDialog(Graphic *plugin);
	~GraphicSaveDialog();
	
	int ok_event();
	int cancel_event();
	Graphic *plugin;
};


class GraphicLoadThread : public Thread
{
public:
	GraphicLoadThread(Graphic *plugin, GraphicMenu *menu);
	~GraphicLoadThread();
	void run();
	Graphic *plugin;
	GraphicMenu *menu;
};

class GraphicLoadDialog : public BC_FileBox
{
public:
	GraphicLoadDialog(Graphic *plugin);
	~GraphicLoadDialog();
	
	int ok_event();
	int cancel_event();
	Graphic *plugin;
};




#endif
