#ifndef COMPRESSWINDOW_H
#define COMPRESSWINDOW_H

#define TOTAL_LOADS 5

#include "bcbase.h"

class CompressThread;
class CompressWindow;

#include "compress.h"

#include "filehtal.h"
#include "mutex.h"

class CompressThread : public Thread
{
public:
	CompressThread(Compress *compress);
	~CompressThread();
	
	void run();
	
	Mutex gui_started; // prevent loading data until the GUI is started
	Compress *compress;
	CompressWindow *window;
};

class CompressCanvas;
class CompressReadAhead;
class CompressAttack;
class CompressChannel;
class CompressMenu;
class CompressClear;

class CompressWindow : public BC_Window
{
public:
	CompressWindow(Compress *compress);
	~CompressWindow();
	int create_objects();
	int get_divisions(int pixels, char **titles, int *title_x);
	
	int close_event();
	
	int draw_envelope();
	
	Compress *compress;
	CompressCanvas *canvas;
	CompressReadAhead *readahead;
	CompressAttack *attack;
	CompressChannel *channel;
	CompressMenu *menu;
	CompressClear *clear;
};

class CompressCanvas : public BC_Canvas
{
public:
	CompressCanvas(Compress *compress, int x, int y, int w, int h);
	~CompressCanvas();

	int handle_event();
	int button_release(); 
	int button_press();
	int cursor_motion();
	
	int draw_floating(int flash);
	int test_points();
	int test_point(int x, int y);
	int test_lines();
	int test_line(int x1, int y1, int x2, int y2, float &input, float &output);
	int insert_point(int number, float input, float output);
	int delete_point(int selected_point);
	
	int draw_point(int x, int y);
	int selected_point;
	Compress *compress;
};

class CompressReadAhead : public BC_IPot
{
public:
	CompressReadAhead(Compress *compress, int x, int y);
	~CompressReadAhead();
	int handle_event();
	Compress *compress;
};

class CompressAttack : public BC_IPot
{
public:
	CompressAttack(Compress *compress, int x, int y);
	~CompressAttack();
	int handle_event();
	Compress *compress;
};

class CompressChannel : public BC_TextBox
{
public:
	CompressChannel(Compress *compress, int x, int y);
	~CompressChannel();
	int handle_event();
	Compress *compress;
};

class CompressClear : public BC_BigButton
{
public:
	CompressClear(Compress *compress, int x, int y);
	~CompressClear();
	int handle_event();
	Compress *compress;
};

class CompressLoad;
class CompressSave;
class CompressSetDefault;
class CompressLoadPrev;
class CompressLoadPrevThread;

class CompressMenu : public BC_MenuBar
{
public:
	CompressMenu(Compress *compress, CompressWindow *window);
	~CompressMenu();
	
	int create_objects(Defaults *defaults);
	int load_defaults(Defaults *defaults);
	int save_defaults(Defaults *defaults);
// most recent loads
	int add_load(char *path);
	CompressLoadPrevThread *prev_load_thread;
	
	int total_loads;
	BC_Menu *filemenu;
	CompressWindow *window;
	Compress *compress;
	CompressLoad *load;
	CompressSave *save;
	CompressSetDefault *set_default;
	CompressLoadPrev *prev_load[TOTAL_LOADS];
};

class CompressSaveThread;
class CompressLoadThread;

class CompressLoad : public BC_MenuItem
{
public:
	CompressLoad(Compress *compress, CompressMenu *menu);
	~CompressLoad();
	int handle_event();
	Compress *compress;
	CompressLoadThread *thread;
	CompressMenu *menu;
};

class CompressSave : public BC_MenuItem
{
public:
	CompressSave(Compress *compress, CompressMenu *menu);
	~CompressSave();
	int handle_event();
	Compress *compress;
	CompressSaveThread *thread;
	CompressMenu *menu;
};

class CompressSetDefault : public BC_MenuItem
{
public:
	CompressSetDefault();
	int handle_event();
};

class CompressLoadPrev : public BC_MenuItem
{
public:
	CompressLoadPrev(Compress *compress, CompressMenu *menu, char *filename, char *path);
	CompressLoadPrev(Compress *compress, CompressMenu *menu);
	int handle_event();
	int set_path(char *path);
	char path[1024];
	Compress *compress;
	CompressMenu *menu;
};


class CompressLoadPrevThread : public Thread
{
public:
	CompressLoadPrevThread(Compress *compress, CompressMenu *menu);
	~CompressLoadPrevThread();
	void run();
	int set_path(char *path);
	char path[1024];
	Compress *compress;
	CompressMenu *menu;
};



class CompressSaveThread : public Thread
{
public:
	CompressSaveThread(Compress *compress, CompressMenu *menu);
	~CompressSaveThread();
	void run();
	Compress *compress;
	CompressMenu *menu;
};

class CompressSaveDialog : public BC_FileBox
{
public:
	CompressSaveDialog(Compress *compress);
	~CompressSaveDialog();
	
	int ok_event();
	int cancel_event();
	Compress *compress;
};


class CompressLoadThread : public Thread
{
public:
	CompressLoadThread(Compress *compress, CompressMenu *menu);
	~CompressLoadThread();
	void run();
	Compress *compress;
	CompressMenu *menu;
};

class CompressLoadDialog : public BC_FileBox
{
public:
	CompressLoadDialog(Compress *compress);
	~CompressLoadDialog();
	
	int ok_event();
	int cancel_event();
	Compress *compress;
};





#endif
