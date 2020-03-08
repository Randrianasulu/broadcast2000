#include "graphicwindow.h"


GraphicThread::GraphicThread(Graphic *plugin)
 : Thread()
{
	this->plugin = plugin;
	synchronous = 1; // make thread wait for join
	gui_started.lock(); // make plugin wait for startup
}

GraphicThread::~GraphicThread()
{
}
	
void GraphicThread::run()
{
	window = new GraphicWindow(plugin);
	window->create_objects();
	gui_started.unlock(); // make plugin wait for startup
	window->run_window();
// defaults are saved by plugin before this
	delete window;
}






GraphicWindow::GraphicWindow(Graphic *plugin)
 : BC_Window("", MEGREY, plugin->gui_string, 460, 240, 0, 0, 0, !plugin->show_initially)
{ this->plugin = plugin; }

GraphicWindow::~GraphicWindow()
{
}

int GraphicWindow::create_objects()
{
	add_tool(menu = new GraphicMenu(plugin, this));
	menu->create_objects(plugin->defaults);

	int x = 5, y = 30, i;
	add_tool(new BC_Title(x, y, "Amount"));
	y += 20;
	add_tool(new BC_Title(170, y + 170, "Freq"));
	add_tool(canvas = new GraphicCanvas(plugin, x + 30, y, 300, 150));

// calibrate the titles
	int title_x[TOTAL_DIVISIONS];
	char *titles[TOTAL_DIVISIONS];
	for(i = 0; i < TOTAL_DIVISIONS; i++) titles[i] = new char[16];
	get_divisions(canvas->get_w(), titles, title_x);

	for(i = 0; i < TOTAL_DIVISIONS; i++)
	{
		add_tool(new BC_Title(title_x[i] + 30, y + 155, titles[i], SMALLFONT));
	}

	add_tool(new BC_Title(10, y + canvas->get_h() / 2, "0", SMALLFONT));
	add_tool(new BC_Title(10, y, "+15", SMALLFONT));
	add_tool(new BC_Title(10, y + canvas->get_h() - 15, "-15", SMALLFONT));
	
	for(i = 0; i < TOTAL_DIVISIONS; i++) delete titles[i];

	x += 340; y += 5;
	add_tool(new BC_Title(x, y + 10, "Window Size:"));
	add_tool(windowsize = new GraphicWindowSize(plugin, x, y + 30));

	y += 60;
	add_tool(new GraphicClear(plugin, x, y));
	canvas->draw_envelope();
}

int GraphicWindow::get_divisions(int pixels, char **titles, int *title_x)
{
	int i;
	int division_freqs = TOTALFREQS / (TOTAL_DIVISIONS - 1);
	int division_pixels = pixels / (TOTAL_DIVISIONS - 1);

	for(i = 0; i < TOTAL_DIVISIONS; i++)
	{
		sprintf(titles[i], "%d", plugin->freq.tofreq(division_freqs * i));

printf("GraphicWindow::get_divisions %d %d\n", division_freqs * i, plugin->freq.tofreq(division_freqs * i));
		if(i < TOTAL_DIVISIONS - 1)
			title_x[i] = division_pixels * i;
		else 
			title_x[i] = division_pixels * i - 20;
	}
}

int GraphicWindow::close_event()
{
	hide_window();
	plugin->send_hide_gui();
}




GraphicCanvas::GraphicCanvas(Graphic *plugin, int x, int y, int w, int h)
 : BC_Canvas(x, y, w, h, WHITE)
{
	this->plugin = plugin;
	selected_point = -1;
}

GraphicCanvas::~GraphicCanvas()
{
}

int GraphicCanvas::handle_event()
{
}

int GraphicCanvas::button_release()
{
	int result = 0;
	
	if(selected_point > -1)
	{
		if(selected_point < plugin->total_points - 1 && 
			plugin->freqs[selected_point] >= plugin->freqs[selected_point + 1]) result = 1;

		if(selected_point > 0 && 
			plugin->freqs[selected_point] <= plugin->freqs[selected_point - 1]) result = 1;

		if(result) delete_point(selected_point);

		selected_point = -1;
		plugin->update_gui();
	}
}

int GraphicCanvas::button_press()
{
// test points
	selected_point = test_points();
	if(selected_point == -1) selected_point = test_lines();

	if(selected_point == -1) 
	return 0;
	else
	{
		draw_floating(1);
		return 1;
	}
}

int GraphicCanvas::cursor_motion()
{
	if(selected_point > -1)
	{
		long new_freq;
		float amount;

		draw_floating(0);
		get_values(get_cursor_x(), get_cursor_y(), amount, new_freq);

		plugin->freqs[selected_point] = new_freq;
		plugin->amounts[selected_point] = amount;

		draw_floating(1);
		plugin->send_configure_change();
	}
}

int GraphicCanvas::get_point(int point, int &x, int &y)
{
	x = (int)((float)plugin->freq.fromfreq(plugin->freqs[point]) / TOTALFREQS * get_w());
	y = (int)((float)get_h() / 2 - plugin->amounts[point] / MAXLEVEL * get_h() / 2);
}

int GraphicCanvas::get_values(int x, int y, float &amount, long &freq)
{
	freq = (int)((float)x / get_w() * TOTALFREQS);
	if(freq > TOTALFREQS) freq = TOTALFREQS;
	if(freq < 0) freq = 0;
	freq = plugin->freq.tofreq(freq);

	amount = -((float)(get_cursor_y() - get_h() / 2) / (get_h() / 2)) * MAXLEVEL;
	if(amount <= -MAXLEVEL) amount = -MAXLEVEL;
	if(amount > MAXLEVEL) amount = MAXLEVEL;

	if(shift_down()) amount = 0;
}

int GraphicCanvas::test_points()
{
	int x, y, result = -1;
	for(int i = 0; i < plugin->total_points && result == -1; i++)
	{
		get_point(i, x, y);
		if(test_point(x, y)) result = i;
	}
	return result;
}

int GraphicCanvas::test_lines()
{
	int x1, y1, x2, y2;
	int result = -1;
	float amount;
	long new_freq;

	x1 = 0;
	y1 = get_h() / 2;
	for(int i = 0; i < plugin->total_points && result == -1; i++)
	{
		get_point(i, x2, y2);
		
		if(test_line(x1, y1, x2, y2)) result = i;
		x1 = x2;
		y1 = y2;
	}

	if(result == -1)
	{
		x2 = get_w();
		y2 = get_h() / 2;
		if(test_line(x1, y1, x2, y2)) result = plugin->total_points;
	}
	if(result > -1)
	{
// insert a point
		get_values(get_cursor_x(), get_cursor_y(), amount, new_freq);
		insert_point(result, amount, new_freq);
	}
	return result;
}

int GraphicCanvas::test_line(int x1, int y1, int x2, int y2)
{
    float slope;
    int miny, maxy, y;
	const int HEIGHT = 5;
	float amount;
	long new_freq;

	if(get_cursor_x() >= x1 && get_cursor_x() <= x2)
	{
		slope = (float)(y2 - y1) / (x2 - x1);
		y = y1 + (int)(slope * (cursor_x - x1));
		miny = y - HEIGHT;
		maxy = y + HEIGHT;

		if(get_cursor_y() >= miny && get_cursor_y() <= maxy)
		{
			return 1;
		}
	}
	return 0;
}

int GraphicCanvas::test_point(int x, int y)
{
	const int WIDTH = 5;
	const int HEIGHT = 5;
	int x1, y1, x2, y2;
	
	x1 = x - WIDTH;     x2 = x + WIDTH;
	y1 = y - HEIGHT;    y2 = y + HEIGHT;
	
	if(x1 < 0) x1 = 0;
	if(x2 > w) x2 = w;
	if(y1 < 0) y1 = 0;
	if(y2 > h) y2 = h;
	
	if(get_cursor_x() >= x1 && get_cursor_x() <= x2 && get_cursor_y() >= y1 && get_cursor_y() <= y2) return 1;

	return 0;
}

int GraphicCanvas::insert_point(int number, float new_amount, long new_freq)
{
	for(int j = plugin->total_points, i = plugin->total_points - 1; j > number; j--, i--)
	{
		plugin->amounts[j] = plugin->amounts[i];
		plugin->freqs[j] = plugin->freqs[i];
	}
	plugin->amounts[number] = new_amount;
	plugin->freqs[number] = new_freq;
	plugin->total_points++;
}

int GraphicCanvas::delete_point(int selected_point)
{
	for(int j = selected_point, i = selected_point + 1; i < plugin->total_points; i++, j++)
	{
		plugin->amounts[j] = plugin->amounts[i];
		plugin->freqs[j] = plugin->freqs[i];
	}
	plugin->total_points--;
}

int GraphicCanvas::draw_point(int x, int y)
{
	const int WIDTH = 5;
	const int HEIGHT = 5;
	static int x1, y1, x2, y2;
	
	x1 = x - WIDTH;     x2 = x + WIDTH;
	y1 = y - HEIGHT;    y2 = y + HEIGHT;
	
	if(x1 < 0) x1 = 0;
	if(x2 > w) x2 = w;
	if(y1 < 0) y1 = 0;
	if(y2 > h) y2 = h;
	
	draw_box(x1, y1, x2 - x1, y2 - y1);
}

int GraphicCanvas::draw_floating(int flash)
{
	static int x1, y1, x2, y2, x3, y3, tx, ty;
	static char string[32];

	get_point(selected_point, x2, y2);
	
	if(selected_point > 0)
	{
		get_point(selected_point - 1, x1, y1);
	}
	else
	{
		x1 = 0;
		y1 = get_h() / 2;
	}

	if(selected_point < plugin->total_points - 1)
	{
		get_point(selected_point + 1, x3, y3);
	}
	else
	{
		x3 = w;
		y3 = get_h() / 2;
	}
	
	tx = x2;
	ty = y2;

	if(tx < w / 2) 
		tx += 20;
	else 
		tx -= 100;

	if(ty < h / 2) 
		ty += 50;
	else 
		ty -= 20;

	set_inverse();
	set_color(WHITE);
	draw_line(x1, y1, x2, y2);
	draw_line(x2, y2, x3, y3);
	draw_point(x2, y2);	
	sprintf(string, "%.2fx%d", plugin->amounts[selected_point], plugin->freqs[selected_point]);
	draw_text(tx, ty, string);
	set_opaque();
	if(flash) GraphicCanvas::flash();
}

int GraphicCanvas::draw_envelope()
{
	int x1, y1, x2, y2;

	set_color(WHITE);
	draw_box(0, 0, get_w(), get_h());
	set_color(BLACK);

	x1 = 0; 
	y1 = get_h() / 2;
	for(int i = 0; i < plugin->total_points; i++)
	{
		get_point(i, x2, y2);
		
		draw_line(x1, y1, x2, y2);
		draw_point(x2, y2);
		x1 = x2;
		y1 = y2;
	}

	x2 = get_w();
	y2 = get_h() / 2;
	draw_line(x1, y1, x2, y2);
	flash();
}








GraphicWindowSize::GraphicWindowSize(Graphic *plugin, int x, int y)
 : BC_TextBox(x, y, 100, (int)plugin->fourier.window_size)
{
	this->plugin = plugin;
}

GraphicWindowSize::~GraphicWindowSize()
{
}

int GraphicWindowSize::handle_event()
{
	if(atol(get_text()) <= 1024)
	plugin->fourier.window_size = 1024;
	else
	plugin->fourier.window_size = atol(get_text());

	plugin->send_configure_change();
}




GraphicClear::GraphicClear(Graphic *plugin, int x, int y)
 : BC_BigButton(x, y, "Clear")
{
	this->plugin = plugin;
}

GraphicClear::~GraphicClear()
{
}

int GraphicClear::handle_event()
{
	plugin->reset();
	plugin->send_configure_change();
}





GraphicMenu::GraphicMenu(Graphic *plugin, GraphicWindow *window)
 : BC_MenuBar(0, 0, window->get_w())
{
	this->window = window;
	this->plugin = plugin;
}

GraphicMenu::~GraphicMenu()
{
	delete load;
	delete save;
	//delete set_default;
	for(int i = 0; i < total_loads; i++)
	{
		delete prev_load[i];
	}
	delete prev_load_thread;
}

int GraphicMenu::create_objects(Defaults *defaults)
{
	add_menu(filemenu = new BC_Menu("File"));
	filemenu->add_menuitem(load = new GraphicLoad(plugin, this));
	filemenu->add_menuitem(save = new GraphicSave(plugin, this));
	//filemenu->add_menuitem(set_default = new GraphicSetDefault);
	load_defaults(defaults);
	prev_load_thread = new GraphicLoadPrevThread(plugin, this);
}

int GraphicMenu::load_defaults(Defaults *defaults)
{
	FileSystem fs;
	total_loads = defaults->get("TOTAL_LOADS", 0);
	if(total_loads > 0)
	{
		filemenu->add_menuitem(new BC_MenuItem("-"));
		char string[1024], path[1024], filename[1024];
	
		for(int i = 0; i < total_loads; i++)
		{
			sprintf(string, "LOADPREVIOUS%d", i);
			defaults->get(string, path);
			fs.extract_name(filename, path);
//printf("GraphicMenu::load_defaults %s\n", path);
			filemenu->add_menuitem(prev_load[i] = new GraphicLoadPrev(plugin, this, filename, path));
		}
	}
}

int GraphicMenu::save_defaults(Defaults *defaults)
{
	if(total_loads > 0)
	{
		defaults->update("TOTAL_LOADS",  total_loads);
		char string[1024];
		
		for(int i = 0; i < total_loads; i++)
		{
			sprintf(string, "LOADPREVIOUS%d", i);
			defaults->update(string, prev_load[i]->path);
		}
	}
}

int GraphicMenu::add_load(char *path)
{
	if(total_loads == 0)
	{
		filemenu->add_menuitem(new BC_MenuItem("-"));
	}
	
// test for existing copy
	FileSystem fs;
	char text[1024], new_path[1024];      // get text and path
	fs.extract_name(text, path);
	strcpy(new_path, path);
	
	for(int i = 0; i < total_loads; i++)
	{
		if(!strcmp(prev_load[i]->text, text))     // already exists
		{                                // swap for top load
			for(int j = i; j > 0; j--)   // move preceeding loads down
			{
				prev_load[j]->set_text(prev_load[j - 1]->text);
				prev_load[j]->set_path(prev_load[j - 1]->path);
			}
			prev_load[0]->set_text(text);
			prev_load[0]->set_path(new_path);
			return 1;
		}
	}
	
// add another load
	if(total_loads < TOTAL_LOADS)
	{
		filemenu->add_menuitem(prev_load[total_loads] = new GraphicLoadPrev(plugin, this));
		total_loads++;
	}
	
// cycle loads down
	for(int i = total_loads - 1; i > 0; i--)
	{         
	// set menu item text
		prev_load[i]->set_text(prev_load[i - 1]->text);
	// set filename
		prev_load[i]->set_path(prev_load[i - 1]->path);
	}

// set up the new load
	prev_load[0]->set_text(text);
	prev_load[0]->set_path(new_path);
	return 0;
}

GraphicLoad::GraphicLoad(Graphic *plugin, GraphicMenu *menu)
 : BC_MenuItem("Load...")
{
	this->plugin = plugin;
	this->menu = menu;
	thread = new GraphicLoadThread(plugin, menu);
}
GraphicLoad::~GraphicLoad()
{
	delete thread;
}
int GraphicLoad::handle_event()
{
	thread->start();
}

GraphicSave::GraphicSave(Graphic *plugin, GraphicMenu *menu)
 : BC_MenuItem("Save...")
{
	this->plugin = plugin;
	this->menu = menu;
	thread = new GraphicSaveThread(plugin, menu);
}
GraphicSave::~GraphicSave()
{
	delete thread;
}
int GraphicSave::handle_event()
{
	thread->start();
}

GraphicSetDefault::GraphicSetDefault()
 : BC_MenuItem("Set default")
{
}
int GraphicSetDefault::handle_event()
{
}

GraphicLoadPrev::GraphicLoadPrev(Graphic *plugin, GraphicMenu *menu, char *filename, char *path)
 : BC_MenuItem(filename)
{
	this->plugin = plugin;
	this->menu = menu;
	strcpy(this->path, path);
}
GraphicLoadPrev::GraphicLoadPrev(Graphic *plugin, GraphicMenu *menu)
 : BC_MenuItem("")
{
	this->plugin = plugin;
	this->menu = menu;
}
int GraphicLoadPrev::handle_event()
{
	menu->prev_load_thread->set_path(path);
	menu->prev_load_thread->start();
}
int GraphicLoadPrev::set_path(char *path)
{
	strcpy(this->path, path);
}


GraphicSaveThread::GraphicSaveThread(Graphic *plugin, GraphicMenu *menu)
 : Thread()
{
	this->plugin = plugin;
	this->menu = menu;
}
GraphicSaveThread::~GraphicSaveThread()
{
}
void GraphicSaveThread::run()
{
	int result = 0;
	{
		GraphicSaveDialog dialog(plugin);
		dialog.create_objects();
		result = dialog.run_window();
		if(!result) strcpy(plugin->config_directory, dialog.get_filename());
	}
	if(!result) 
	{
		result = plugin->save_to_file(plugin->config_directory);
		menu->add_load(plugin->config_directory);
	}
}

GraphicSaveDialog::GraphicSaveDialog(Graphic *plugin)
 : BC_FileBox("", 
 			plugin->config_directory, 
 			"Save plugin", 
 			"Select the plugin file to save as", 0, 0)
{
	this->plugin = plugin;
}
GraphicSaveDialog::~GraphicSaveDialog()
{
}
int GraphicSaveDialog::ok_event()
{
	set_done(0);
}
int GraphicSaveDialog::cancel_event()
{
	set_done(1);
}



GraphicLoadThread::GraphicLoadThread(Graphic *plugin, GraphicMenu *menu)
 : Thread()
{
	this->plugin = plugin;
	this->menu = menu;
}
GraphicLoadThread::~GraphicLoadThread()
{
}
void GraphicLoadThread::run()
{
	int result = 0;
	{
		GraphicLoadDialog dialog(plugin);
		dialog.create_objects();
		result = dialog.run_window();
		if(!result) strcpy(plugin->config_directory, dialog.get_filename());
	}
	if(!result) 
	{
		result = plugin->load_from_file(plugin->config_directory);
		if(!result)
		{
			menu->add_load(plugin->config_directory);
			plugin->send_configure_change();
		}
	}
}

GraphicLoadPrevThread::GraphicLoadPrevThread(Graphic *plugin, GraphicMenu *menu) : Thread()
{
	this->plugin = plugin;
	this->menu = menu;
}
GraphicLoadPrevThread::~GraphicLoadPrevThread()
{
}
void GraphicLoadPrevThread::run()
{
	int result = 0;
	strcpy(plugin->config_directory, path);
	result = plugin->load_from_file(path);
	if(!result)
	{
		menu->add_load(path);
		plugin->send_configure_change();
	}
}
int GraphicLoadPrevThread::set_path(char *path)
{
	strcpy(this->path, path);
}





GraphicLoadDialog::GraphicLoadDialog(Graphic *plugin)
 : BC_FileBox("", 
 			plugin->config_directory, 
 			"Load plugin", 
 			"Select the plugin file to load from", 0, 0)
{
	this->plugin = plugin;
}
GraphicLoadDialog::~GraphicLoadDialog()
{
}
int GraphicLoadDialog::ok_event()
{
	set_done(0);
}
int GraphicLoadDialog::cancel_event()
{
	set_done(1);
}

