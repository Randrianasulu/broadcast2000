#include <string.h>
#include "levelwindow.h"
#include "levelwindowgui.h"
#include "mainwindow.h"
#include "preferences.h"



LevelWindowGUI::LevelWindowGUI(LevelWindow *thread, int w, int h)
 : BC_Window("", MEGREY, ICONNAME ": Levels", w, h, 10, 10, 0, !thread->meter_visible)
{
	METER_H = 205;
	this->thread = thread;
	total_meters = 0;
	db_scale = 0;
}

LevelWindowGUI::~LevelWindowGUI()
{
	for(int i = 0; i < total_meters; i++) delete meters[i];
	delete_db_scale();
}

int LevelWindowGUI::create_objects()
{
	add_tool(reset_button = new LevelWindowReset(this, 5, get_h() - 30));
return 0;
}


int LevelWindowGUI::resize_event(int w, int h)
{
//printf("LevelWindowGUI::resize_event 1 %d %d\n", w, h);
	if(thread->horizontal)
	{
		if(w < METER_H - METER_H / 3)
		{
// change to vertical
			thread->horizontal = 0;
			resize_window(w, 260);
			int new_meters = total_meters;
			for(int i = 0; i < new_meters; i++) delete_meter();
			for(int i = 0; i < new_meters; i++) new_meter();
		}
		else
		if(w != 230) resize_window(230, h);
	}
	else
	{
		if(h < METER_H - METER_H / 3)
		{
// change to horizontal
			thread->horizontal = 1;
			resize_window(230, h);
			int new_meters = total_meters;
			for(int i = 0; i < new_meters; i++) delete_meter();
			for(int i = 0; i < new_meters; i++) new_meter();
		}
		else
		if(h != 260) resize_window(w, 260);
	}
	
	reset_button->resize(5, this->get_h() - 30);
//printf("LevelWindowGUI::resize_event 2 %d %d\n", w, h);
return 0;
}

int LevelWindowGUI::new_meter()
{
	char string[16];
	int format = thread->mwindow->preferences->meter_format;
	int x, y, w, h, title_x, title_y;
	
	if(format == METER_DB)
	x = y = total_meters * 30 + 5 + 25;
	else
	x = y = total_meters * 30 + 5;

	if(thread->horizontal)
	{
		x = 20;
		if(format == METER_DB) y -= 5;
		title_x = 5;
		title_y = y;
		w = METER_H;
		h = 25;
	}
	else
	{
		y = 20;
		title_x = x;
		title_y = 2;
		w = 25;
		h = METER_H;
	}

	sprintf(string, "%d", total_meters + 1);
	add_tool(meters[total_meters] = new BC_Meter(x, y, w, h, thread->mwindow->preferences->min_meter_db, thread->mwindow->preferences->meter_format));
	add_tool(titles[total_meters] = new BC_Title(title_x, title_y, string));
	total_meters++;

	if(total_meters == 1 && format == METER_DB)
	{
		draw_db_scale();
	}
return 0;
}

int LevelWindowGUI::delete_meter()
{
	total_meters--;
	delete meters[total_meters];
	delete titles[total_meters];
	if(total_meters == 0)
	{
		delete_db_scale();
	}
return 0;
}

int LevelWindowGUI::draw_db_scale()
{
//printf("LevelWindowGUI::draw_db_scale 1\n");
// print calibrated DB guage
	int x, y;
	if(thread->horizontal)
		y = 5;
	else
		x = 5;
	
	for(int i = 0; i < 6; i++)
	{
		if(thread->horizontal)
			x = 20 + (int)meters[0]->title_x[i];
		else
			y = 10 + METER_H - (int)meters[0]->title_x[i];
			
		add_tool(db_title[i] = new BC_Title(x, y, meters[0]->db_titles[i], SMALLFONT));
	}
	
//printf("LevelWindowGUI::draw_db_scale 2\n");
	db_scale = 1;
return 0;
}

int LevelWindowGUI::delete_db_scale()
{
//printf("LevelWindowGUI::delete_db_scale 1\n");
	if(db_scale)
	for(int i = 0; i < 6; i++)
	{
		delete db_title[i];
	}
//printf("LevelWindowGUI::delete_db_scale 2\n");
	db_scale = 0;
return 0;
}

int LevelWindowGUI::close_event()
{
	thread->hide_window();
return 0;
}

int LevelWindowGUI::keypress_event()
{
	if(get_keypress() == 'w')
	{
		thread->hide_window();
		return 1;
	}
	return 0;
return 0;
}


int LevelWindowGUI::reset_over()
{
	for(int i = 0; i < total_meters; i++)
	{
		meters[i]->reset_over();
	}
return 0;
}


LevelWindowReset::LevelWindowReset(LevelWindowGUI *gui, int x, int y)
 : BC_SmallButton(x, y, "Reset")
{
	this->gui = gui;
}

LevelWindowReset::~LevelWindowReset()
{
}

int LevelWindowReset::handle_event()
{
	gui->thread->mwindow->reset_meters();
return 0;
}
