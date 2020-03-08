#include "compresswindow.h"


CompressThread::CompressThread(Compress *compress)
 : Thread()
{
	this->compress = compress;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

CompressThread::~CompressThread()
{
}
	
void CompressThread::run()
{
	window = new CompressWindow(compress);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	window->menu->save_defaults(compress->defaults);
	delete window;
}






CompressWindow::CompressWindow(Compress *compress)
 : BC_Window("", MEGREY, compress->gui_string, 380, 300, 380, 300, 0, !compress->show_initially)
{ this->compress = compress; }

CompressWindow::~CompressWindow()
{
	delete canvas;
	delete readahead;
	delete attack;
	delete clear;
	delete channel;
	delete menu;
}

int CompressWindow::create_objects()
{
	add_tool(menu = new CompressMenu(compress, this));
	menu->create_objects(compress->defaults);

	int x = 5, y = 30, i;
	add_tool(new BC_Title(x, y, "Output"));
	add_tool(new BC_Title(100, y + 245, "Input"));
	add_tool(canvas = new CompressCanvas(compress, x + 30, y + 20, 200, 200));

// calibrate the db titles
	int title_x[6];
	char *db_titles[6];
	for(i = 0; i < 6; i++) db_titles[i] = new char[6];
	get_divisions(200, db_titles, title_x);

	for(i = 0; i < 6; i++)
	{
		add_tool(new BC_Title(title_x[i] + 30, y + 230, db_titles[i], SMALLFONT));
		add_tool(new BC_Title(5, y + 10 + canvas->h - title_x[i], db_titles[i], SMALLFONT));
	}

	for(i = 0; i < 6; i++) delete db_titles[i];
	x += 240; y += 5;

	add_tool(new BC_Title(x, y + 10, "Read ahead"));
	add_tool(readahead = new CompressReadAhead(compress, x + 90, y)); y += 40;

	add_tool(new BC_Title(x, y + 10, "Attack"));
	add_tool(attack = new CompressAttack(compress, x + 60, y)); y += 40;

	add_tool(new BC_Title(x, y + 10, "Control channel"));
	add_tool(channel = new CompressChannel(compress, x, y + 35)); y += 100;
	
	add_tool(clear = new CompressClear(compress, x, y));
	draw_envelope();
}

int CompressWindow::get_divisions(int pixels, char **titles, int *title_x)
{
	int i;
	float j, j_step;
	int division, division_step;
	
	division = 0;
	division_step = pixels / 4;
	j = INFINITYGAIN;     // number for title
	j_step = INFINITYGAIN / 4;

	for(i = 0; i < 4;)
	{
		sprintf(titles[i], "%.0f", j);

		if(i == 0) title_x[i] = division; else title_x[i] = division - 10;
		division += division_step;
		j -= j_step;
		i++;
	}
	
	sprintf(titles[4], "%.0f", j_step / 2);
	title_x[4] = (division + title_x[3]) / 2;
	
	sprintf(titles[5], "0");
	title_x[5] = division - 10;
}

int CompressWindow::close_event()
{
	hide_window();
	compress->send_hide_gui();
}

int CompressWindow::draw_envelope()
{
	int x1, y1, x2, y2;
	
	canvas->set_color(WHITE);
	canvas->draw_box(0, 0, canvas->w, canvas->h);
	canvas->set_color(BLACK);

	x1 = 0; y1 = canvas->h;
	for(int i = 0; i < compress->total_points; i++)
	{
		x2 = canvas->w - (int)(compress->input_level[i] / INFINITYGAIN * canvas->w);
		y2 = (int)(compress->output_level[i] / INFINITYGAIN * canvas->h);
		
		canvas->draw_line(x1, y1, x2, y2);
		canvas->draw_point(x2, y2);
		x1 = x2;
		y1 = y2;
	}

	x2 = canvas->w;
	y2 = 0;
	canvas->draw_line(x1, y1, x2, y2);
	canvas->flash();
}






CompressCanvas::CompressCanvas(Compress *compress, int x, int y, int w, int h)
 : BC_Canvas(x, y, w, h, WHITE)
{
	this->compress = compress;
	selected_point = -1;
}

CompressCanvas::~CompressCanvas()
{
}

int CompressCanvas::handle_event()
{
}

int CompressCanvas::button_release()
{
	int result = 0;
	
	if(selected_point > -1)
	{
		if(selected_point < compress->total_points - 1 && 
			compress->input_level[selected_point] >= compress->input_level[selected_point + 1]) result = 1;

		if(selected_point > 0 && 
			compress->input_level[selected_point] <= compress->input_level[selected_point - 1]) result = 1;

		if(result) delete_point(selected_point);

		selected_point = -1;
		compress->update_gui();
	}
}

int CompressCanvas::button_press()
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

int CompressCanvas::cursor_motion()
{
	if(selected_point > -1)
	{
		draw_floating(0);
		compress->input_level[selected_point] = INFINITYGAIN - ((float)cursor_x / w) * INFINITYGAIN;
		compress->output_level[selected_point] = ((float)cursor_y / h) * INFINITYGAIN;

		if(compress->input_level[selected_point] < INFINITYGAIN) compress->input_level[selected_point] = INFINITYGAIN;
		if(compress->input_level[selected_point] > 0) compress->input_level[selected_point] = 0;
		if(compress->output_level[selected_point] < INFINITYGAIN) compress->output_level[selected_point] = INFINITYGAIN;
		if(compress->output_level[selected_point] > 0) compress->output_level[selected_point] = 0;

		//compress->update_gui();
		draw_floating(1);
		compress->send_configure_change();
	}
}

int CompressCanvas::test_points()
{
	int x, y, result = -1;
	for(int i = 0; i < compress->total_points && result == -1; i++)
	{
		x = w - (int)(compress->input_level[i] / INFINITYGAIN * w);
		y = (int)(compress->output_level[i] / INFINITYGAIN * h);
		
		if(test_point(x, y)) result = i;
	}
	return result;
}

int CompressCanvas::test_lines()
{
	int x1, y1, x2, y2;
	int result = -1;
	float new_input, new_output;
	
	x1 = 0; y1 = h;
	for(int i = 0; i < compress->total_points && result == -1; i++)
	{
		x2 = w - (int)(compress->input_level[i] / INFINITYGAIN * w);
		y2 = (int)(compress->output_level[i] / INFINITYGAIN * h);
		
		if(test_line(x1, y1, x2, y2, new_input, new_output)) result = i;
		x1 = x2;
		y1 = y2;
	}

	if(result == -1)
	{
		x2 = w;
		y2 = 0;
		if(test_line(x1, y1, x2, y2, new_input, new_output)) result = compress->total_points;
	}
	
	if(result > -1)
	{
// insert a point
		insert_point(result, new_input, new_output);
	}
	return result;
}

int CompressCanvas::test_line(int x1, int y1, int x2, int y2, float &new_input, float &new_output)
{
	static float slope;
	static int miny, maxy, y;
	const int HEIGHT = 5;
	
	
	if(cursor_x >= x1 && cursor_x <= x2)
	{
		slope = (float)(y2 - y1) / (x2 - x1);
		y = y1 + (int)(slope * (cursor_x - x1));
		miny = y - HEIGHT;
		maxy = y + HEIGHT;
//printf("y %d cursor_y %d\n", y, cursor_y);

		if(cursor_y >= miny && cursor_y <= maxy)
		{
			new_input = INFINITYGAIN - ((float)cursor_x / w) * INFINITYGAIN;
			new_output = ((float)cursor_y / h) * INFINITYGAIN;
//printf("new_input %f new_output %f\n", new_input, new_output);
			return 1;
		}
	}
	return 0;
}

int CompressCanvas::test_point(int x, int y)
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
	
	if(cursor_x >= x1 && cursor_x <= x2 && cursor_y >= y1 && cursor_y <= y2) return 1;
	
	return 0;
}

int CompressCanvas::insert_point(int number, float input, float output)
{
//printf("CompressCanvas::insert_point %d\n", number);
	for(int j = compress->total_points, i = compress->total_points - 1; j > number; j--, i--)
	{
		compress->input_level[j] = compress->input_level[i];
		compress->output_level[j] = compress->output_level[i];
	}
	compress->input_level[number] = input;
	compress->output_level[number] = output;
	compress->total_points++;
}

int CompressCanvas::delete_point(int selected_point)
{
	for(int j = selected_point, i = selected_point + 1; i < compress->total_points; i++, j++)
	{
		compress->input_level[j] = compress->input_level[i];
		compress->output_level[j] = compress->output_level[i];
	}
	compress->total_points--;
}

int CompressCanvas::draw_point(int x, int y)
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

int CompressCanvas::draw_floating(int flash)
{
	static int x1, y1, x2, y2, x3, y3, tx, ty;
	static char string[32];
	
	x2 = w - (int)(compress->input_level[selected_point] / INFINITYGAIN * w);
	y2 = (int)(compress->output_level[selected_point] / INFINITYGAIN * h);
	
	if(selected_point > 0)
	{
		x1 = w - (int)(compress->input_level[selected_point - 1] / INFINITYGAIN * w);
		y1 = (int)(compress->output_level[selected_point - 1] / INFINITYGAIN * h);
	}
	else
	{
		x1 = 0; y1 = h;
	}
	
	if(selected_point < compress->total_points - 1)
	{
		x3 = w - (int)(compress->input_level[selected_point + 1] / INFINITYGAIN * w);
		y3 = (int)(compress->output_level[selected_point + 1] / INFINITYGAIN * h);
	}
	else
	{
		x3 = w;
		y3 = 0;
	}
	
	tx = x2; ty = y2;
	if(tx < w / 2) tx += 20;
	else tx -= 100;

	if(ty < h / 2) ty += 50;
	else ty -= 20;
	
	
	
	set_inverse();
	set_color(WHITE);
	draw_line(x1, y1, x2, y2);
	draw_line(x2, y2, x3, y3);
	draw_point(x2, y2);	
	sprintf(string, "%.1f %.1f", compress->input_level[selected_point], compress->output_level[selected_point]);
	draw_text(tx, ty, string);
	set_opaque();
	if(flash) CompressCanvas::flash();
}

CompressReadAhead::CompressReadAhead(Compress *compress, int x, int y)
 : BC_IPot(x, y, 40, 40, compress->readahead, 0, 5000, LTGREY, MEGREY)
{
	this->compress = compress;
}
CompressReadAhead::~CompressReadAhead()
{
}
int CompressReadAhead::handle_event()
{
	compress->readahead = atol(text);
	compress->send_configure_change();
}


CompressAttack::CompressAttack(Compress *compress, int x, int y)
 : BC_IPot(x, y, 40, 40, compress->attack, 1, 5000, LTGREY, MEGREY)
{
	this->compress = compress;
}
CompressAttack::~CompressAttack()
{
}
int CompressAttack::handle_event()
{
	compress->attack = atol(text);
	compress->send_configure_change();
}


CompressChannel::CompressChannel(Compress *compress, int x, int y)
 : BC_TextBox(x, y, 50, compress->channel)
{
	this->compress = compress;
}
CompressChannel::~CompressChannel()
{
}
int CompressChannel::handle_event()
{
	compress->channel = atol(get_text());
	compress->send_configure_change();
}

CompressClear::CompressClear(Compress *compress, int x, int y)
 : BC_BigButton(x, y, "Clear")
{
	this->compress = compress;
}
CompressClear::~CompressClear()
{
}
int CompressClear::handle_event()
{
	compress->total_points = 0;
	compress->update_gui();
	compress->send_configure_change();
}






CompressMenu::CompressMenu(Compress *compress, CompressWindow *window)
 : BC_MenuBar(0, 0, window->get_w())
{
	this->window = window;
	this->compress = compress;
}

CompressMenu::~CompressMenu()
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

int CompressMenu::create_objects(Defaults *defaults)
{
	add_menu(filemenu = new BC_Menu("File"));
	filemenu->add_menuitem(load = new CompressLoad(compress, this));
	filemenu->add_menuitem(save = new CompressSave(compress, this));
	//filemenu->add_menuitem(set_default = new CompressSetDefault);
	load_defaults(defaults);
	prev_load_thread = new CompressLoadPrevThread(compress, this);
}

int CompressMenu::load_defaults(Defaults *defaults)
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
//printf("CompressMenu::load_defaults %s\n", path);
			filemenu->add_menuitem(prev_load[i] = new CompressLoadPrev(compress, this, filename, path));
		}
	}
}

int CompressMenu::save_defaults(Defaults *defaults)
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

int CompressMenu::add_load(char *path)
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
		filemenu->add_menuitem(prev_load[total_loads] = new CompressLoadPrev(compress, this));
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

CompressLoad::CompressLoad(Compress *compress, CompressMenu *menu)
 : BC_MenuItem("Load...")
{
	this->compress = compress;
	this->menu = menu;
	thread = new CompressLoadThread(compress, menu);
}
CompressLoad::~CompressLoad()
{
	delete thread;
}
int CompressLoad::handle_event()
{
	thread->start();
}

CompressSave::CompressSave(Compress *compress, CompressMenu *menu)
 : BC_MenuItem("Save...")
{
	this->compress = compress;
	this->menu = menu;
	thread = new CompressSaveThread(compress, menu);
}
CompressSave::~CompressSave()
{
	delete thread;
}
int CompressSave::handle_event()
{
	thread->start();
}

CompressSetDefault::CompressSetDefault()
 : BC_MenuItem("Set default")
{
}
int CompressSetDefault::handle_event()
{
}

CompressLoadPrev::CompressLoadPrev(Compress *compress, CompressMenu *menu, char *filename, char *path)
 : BC_MenuItem(filename)
{
	this->compress = compress;
	this->menu = menu;
	strcpy(this->path, path);
}
CompressLoadPrev::CompressLoadPrev(Compress *compress, CompressMenu *menu)
 : BC_MenuItem("")
{
	this->compress = compress;
	this->menu = menu;
}
int CompressLoadPrev::handle_event()
{
	menu->prev_load_thread->set_path(path);
	menu->prev_load_thread->start();
}
int CompressLoadPrev::set_path(char *path)
{
	strcpy(this->path, path);
}


CompressSaveThread::CompressSaveThread(Compress *compress, CompressMenu *menu)
 : Thread()
{
	this->compress = compress;
	this->menu = menu;
}
CompressSaveThread::~CompressSaveThread()
{
}
void CompressSaveThread::run()
{
	int result = 0;
	{
		CompressSaveDialog dialog(compress);
		dialog.create_objects();
		result = dialog.run_window();
		if(!result) strcpy(compress->config_directory, dialog.get_filename());
	}
	if(!result) 
	{
		result = compress->save_to_file(compress->config_directory);
		menu->add_load(compress->config_directory);
	}
}

CompressSaveDialog::CompressSaveDialog(Compress *compress)
 : BC_FileBox("", 
 			compress->config_directory, 
 			"Save compress", 
 			"Select the compress file to save as", 0, 0)
{
	this->compress = compress;
}
CompressSaveDialog::~CompressSaveDialog()
{
}
int CompressSaveDialog::ok_event()
{
	set_done(0);
}
int CompressSaveDialog::cancel_event()
{
	set_done(1);
}



CompressLoadThread::CompressLoadThread(Compress *compress, CompressMenu *menu)
 : Thread()
{
	this->compress = compress;
	this->menu = menu;
}
CompressLoadThread::~CompressLoadThread()
{
}
void CompressLoadThread::run()
{
	int result = 0;
	{
		CompressLoadDialog dialog(compress);
		dialog.create_objects();
		result = dialog.run_window();
		if(!result) strcpy(compress->config_directory, dialog.get_filename());
	}
	if(!result) 
	{
		result = compress->load_from_file(compress->config_directory);
		if(!result)
		{
			menu->add_load(compress->config_directory);
			compress->send_configure_change();
		}
	}
}

CompressLoadPrevThread::CompressLoadPrevThread(Compress *compress, CompressMenu *menu) : Thread()
{
	this->compress = compress;
	this->menu = menu;
}
CompressLoadPrevThread::~CompressLoadPrevThread()
{
}
void CompressLoadPrevThread::run()
{
	int result = 0;
	strcpy(compress->config_directory, path);
	result = compress->load_from_file(path);
	if(!result)
	{
		menu->add_load(path);
		compress->send_configure_change();
	}
}
int CompressLoadPrevThread::set_path(char *path)
{
	strcpy(this->path, path);
}





CompressLoadDialog::CompressLoadDialog(Compress *compress)
 : BC_FileBox("", 
 			compress->config_directory, 
 			"Load compress", 
 			"Select the compress file to load from", 0, 0)
{
	this->compress = compress;
}
CompressLoadDialog::~CompressLoadDialog()
{
}
int CompressLoadDialog::ok_event()
{
	set_done(0);
}
int CompressLoadDialog::cancel_event()
{
	set_done(1);
}

