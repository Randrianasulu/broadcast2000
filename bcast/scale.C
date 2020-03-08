#include <string.h>
#include "mainundo.h"
#include "mainwindow.h"
#include "scale.h"
#include "tracks.h"
#include "videowindow.h"

Scale::Scale(MainWindow *mwindow)
 : BC_MenuItem("Resize...")
{ 
	this->mwindow = mwindow; 
	thread = new ScaleThread(mwindow);
}

Scale::~Scale() 
{
	delete thread;
}

int Scale::handle_event()
{
	thread->start();
return 0;
}

ScaleThread::ScaleThread(MainWindow *mwindow)
 : Thread()
{ 
	this->mwindow = mwindow; 
	already_running = 0;
}

ScaleThread::~ScaleThread() {}

void ScaleThread::run()
{
	if(already_running) return;
	already_running = 1;
	constrain_ratio = mwindow->defaults->get("SCALECONSTRAIN", 0);
	scale_data = mwindow->defaults->get("SCALEDATA", 0);
	auto_aspect = mwindow->defaults->get("AUDIOASPECT", 0);
	offsets[0] = offsets[1] = offsets[2] = offsets[3] = 0;

	orig_dimension[0] = dimension[0] = mwindow->track_w;
	orig_dimension[1] = dimension[1] = mwindow->track_h;
	orig_dimension[2] = dimension[2] = mwindow->output_w;
	orig_dimension[3] = dimension[3] = mwindow->output_h;
	ratio[0] = ratio[1] = ratio[2] = ratio[3] = 1;
	aspect_w = mwindow->aspect_w;
	aspect_h = mwindow->aspect_h;

	window = new ScaleWindow(this);
	window->create_objects();
	int result = window->run_window();
	if(!result)
	{
		int dummy_offsets[4];
		dummy_offsets[0] = dummy_offsets[1] = dummy_offsets[2] = dummy_offsets[3] = 0;
// Fake the offsets if data is scaled.

// fix tracks
		//mwindow->stop_playback(1);
// save the before undo
		mwindow->undo->update_undo_edits("Resize", 0);
		mwindow->tracks->scale_video(dimension, scale_data ? dummy_offsets : offsets, scale_data);
		mwindow->track_w = dimension[0];
		mwindow->track_h = dimension[1];
		mwindow->output_w = dimension[2];
		mwindow->output_h = dimension[3];
		mwindow->aspect_w = aspect_w;
		mwindow->aspect_h = aspect_h;
		mwindow->video_window->resize_window();
		mwindow->draw();
		mwindow->undo->update_undo_edits();
		mwindow->changes_made = 1;
		mwindow->defaults->update("ASPECTW", aspect_w);
		mwindow->defaults->update("ASPECTH", aspect_h);
		mwindow->defaults->update("AUTOASPECT", auto_aspect);
	}
	delete window;

	mwindow->defaults->update("SCALECONSTRAIN", constrain_ratio);
	mwindow->defaults->update("SCALEDATA", scale_data);
	already_running = 0;
}

int ScaleThread::update_window(int offset_updated)
{
	int pair_start = 0;
	int i, result, modified_item, dimension_modified = 0, ratio_modified = 0;

	for(i = 0, result = 0; i < 4 && !result; i++)
	{
		if(i == 2) pair_start = 2;
		if(dimension[i] < 0)
		{
			dimension[i] *= -1;
			result = 1;
			modified_item = i;
			dimension_modified = 1;
		}
		if(ratio[i] < 0)
		{
			ratio[i] *= -1;
			result = 1;
			modified_item = i;
			ratio_modified = 1;
		}
	}

	if(result)
	{
		if(dimension_modified)
			ratio[modified_item] = (float)dimension[modified_item] / orig_dimension[modified_item];

		if(ratio_modified && !constrain_ratio)
		{
			dimension[modified_item] = (int)(orig_dimension[modified_item] * ratio[modified_item]);
			window->dimension[modified_item]->update(dimension[modified_item]);
		}

		for(i = pair_start; i < pair_start + 2 && constrain_ratio; i++)
		{
			if(dimension_modified ||
				(i != modified_item && ratio_modified))
			{
				ratio[i] = ratio[modified_item];
				window->ratio[i]->update(ratio[i]);
			}

			if(ratio_modified ||
				(i != modified_item && dimension_modified))
			{
				dimension[i] = (int)(orig_dimension[i] * ratio[modified_item]);
				window->dimension[i]->update(dimension[i]);
			}
		}
	}

//	window->position1->draw();
//	window->position2->draw();
//printf("%d\n", offsets[0]);
//	if(!offset_updated)
//	{
//		window->offsets[0]->update(offsets[0]);
//		window->offsets[1]->update(offsets[1]);
//		window->offsets[2]->update(offsets[2]);
//		window->offsets[3]->update(offsets[3]);
//	}
	
	update_aspect(window);
	return 0;
return 0;
}

int ScaleThread::update_aspect(ScaleWindow *window)
{
	if(auto_aspect)
	{
		char string[1024];
		mwindow->create_aspect_ratio(aspect_w, aspect_h, dimension[2], dimension[3]);
		sprintf(string, "%.0f", aspect_w);
		window->aspect_w->update(string);
		sprintf(string, "%.0f", aspect_h);
		window->aspect_h->update(string);
	}
return 0;
}



ScaleWindow::ScaleWindow(ScaleThread *thread)
 : BC_Window(ICONNAME ": Scale", 370, 260, 0, 0)
{ this->thread = thread; }

ScaleWindow::~ScaleWindow()
{
}

int ScaleWindow::create_objects()
{
	int x = 10, y = 10;
	add_tool(new BC_Title(x, y, "New camera size:"));
	add_tool(new BC_Title(x + 200, y, "New projector size:"));
	y += 30;
	add_tool(new BC_Title(x, y, "Width:"));
	x += 70;
	add_tool(dimension[0] = new ScaleSizeText(x, y, thread, &(thread->dimension[0])));
	x += 110;
	add_tool(new BC_Title(x, y, "Width:"));
	x += 70;
	add_tool(dimension[2] = new ScaleSizeText(x, y, thread, &(thread->dimension[2])));

	y += 30;
	x = 10;
	add_tool(new BC_Title(x, y, "Height:"));
	x += 70;
	add_tool(dimension[1] = new ScaleSizeText(x, y, thread, &(thread->dimension[1])));
	x += 110;
	add_tool(new BC_Title(x, y, "Height:"));
	x += 70;
	add_tool(dimension[3] = new ScaleSizeText(x, y, thread, &(thread->dimension[3])));

	y += 30;
	x = 10;
	add_tool(new BC_Title(x, y, "W Ratio:"));
	x += 70;
	add_tool(ratio[0] = new ScaleRatioText(x, y, thread, &(thread->ratio[0])));
	x += 110;
	add_tool(new BC_Title(x, y, "W Ratio:"));
	x += 70;
	add_tool(ratio[2] = new ScaleRatioText(x, y, thread, &(thread->ratio[2])));

	y += 30;
	x = 10;
	add_tool(new BC_Title(x, y, "H Ratio:"));
	x += 70;
	add_tool(ratio[1] = new ScaleRatioText(x, y, thread, &(thread->ratio[1])));
	x += 110;
	add_tool(new BC_Title(x, y, "H Ratio:"));
	x += 70;
	add_tool(ratio[3] = new ScaleRatioText(x, y, thread, &(thread->ratio[3])));

//	y += 30;
//	x = 10;
//	add_tool(new BC_Title(x, y, "X Offset:"));
//	x += 70;
//	add_tool(offsets[0] = new ScaleOffsetText(x, y, thread, &(thread->offsets[0])));
//	x += 110;
//	add_tool(new BC_Title(x, y, "X Offset:"));
//	x += 70;
//	add_tool(offsets[2] = new ScaleOffsetText(x, y, thread, &(thread->offsets[2])));
//
//	y += 30;
//	x = 10;
//	add_tool(new BC_Title(x, y, "Y Offset:"));
//	x += 70;
//	add_tool(offsets[1] = new ScaleOffsetText(x, y, thread, &(thread->offsets[1])));
//	x += 110;
//	add_tool(new BC_Title(x, y, "Y Offset:"));
//	x += 70;
//	add_tool(offsets[3] = new ScaleOffsetText(x, y, thread, &(thread->offsets[3])));

	x = 10;
	y += 30;
	add_tool(new BC_Title(x, y, "Aspect ratio:"));
	x += 100;
	char string[1024];
	sprintf(string, "%.0f", thread->aspect_w);
	add_tool(aspect_w = new ScaleAspectW(x, y, thread, &(thread->aspect_w), string));
	x += 55;
	add_tool(new BC_Title(x, y, ":"));
	x += 10;
	sprintf(string, "%.0f", thread->aspect_h);
	add_tool(aspect_h = new ScaleAspectH(x, y, thread, &(thread->aspect_h), string));
	x += 60;
	add_tool(new ScaleAspectAuto(x, y + 5, thread));

	y += 30;
//	x = 40;
//	add_tool(new BC_Title(x, y, "Camera position:"));
//	x += 200;
//	add_tool(new BC_Title(x, y, "Projector position:"));

//	ScalePosition *position;
//	x = 60;
//	y += 25;
//	add_tool(position1 = new ScalePosition(x, y, thread, this, 
//		&(thread->orig_dimension[0]), &(thread->dimension[0]), &(thread->offsets[0])));
//	position1->draw();

//	x += 200;
//	add_tool(position2 = new ScalePosition(x, y, thread, this, 
//		&(thread->orig_dimension[2]), &(thread->dimension[2]), &(thread->offsets[2])));
//	position2->draw();

//	y += 110;
	x = 10;
	add_tool(new ScaleConstrain(x, y, thread));	
	x += 200;
	add_tool(new ScaleData(x, y, thread));	

	y += 30;
	x = 50;
	add_tool(new ScaleOK(x, y, this));
	x += 200;
	add_tool(new ScaleCancel(x, y, this));
return 0;
}

ScaleSizeText::ScaleSizeText(int x, int y, ScaleThread *thread, int *output)
 : BC_TextBox(x, y, 100, *output)
{ this->thread = thread; this->output = output; }
ScaleSizeText::~ScaleSizeText() {}
int ScaleSizeText::handle_event()
{
	*output = atol(get_text());
	*output /= 2;
	*output *= 2;
	if(*output <= 0) *output = 2;
	if(*output > 10000) *output = 10000;
	*output *= -1;
	thread->update_window();
return 0;
}

ScaleOffsetText::ScaleOffsetText(int x, int y, ScaleThread *thread, int *output)
 : BC_TextBox(x, y, 100, *output)
{ this->thread = thread; this->output = output; }
ScaleOffsetText::~ScaleOffsetText() {}
int ScaleOffsetText::handle_event()
{
	*output = atol(get_text());
	//if(*output <= 0) *output = 0;
	if(*output > 10000) *output = 10000;
	if(*output < -10000) *output = -10000;
	thread->update_window(1);
return 0;
}

ScaleRatioText::ScaleRatioText(int x, int y, ScaleThread *thread, float *output)
 : BC_TextBox(x, y, 100, *output)
{ this->thread = thread; this->output = output; }
ScaleRatioText::~ScaleRatioText() {}
int ScaleRatioText::handle_event()
{
	*output = atof(get_text());
	//if(*output <= 0) *output = 1;
	if(*output > 10000) *output = 10000;
	if(*output < -10000) *output = -10000;
	*output *= -1;
	thread->update_window();
return 0;
}




ScaleConstrain::ScaleConstrain(int x, int y, ScaleThread *thread)
 : BC_CheckBox(x, y, 17, 17, thread->constrain_ratio, "Constrain ratio")
{ this->thread = thread; }
ScaleConstrain::~ScaleConstrain() {}
int ScaleConstrain::handle_event()
{
	thread->constrain_ratio = get_value();
return 0;
}

ScaleData::ScaleData(int x, int y, ScaleThread *thread)
 : BC_CheckBox(x, y, 17, 17, thread->scale_data, "Scale data")
{ this->thread = thread; }
ScaleData::~ScaleData() {}
int ScaleData::handle_event()
{
	thread->scale_data = get_value();
	thread->update_window();
return 0;
}



ScalePosition::ScalePosition(int x, int y, ScaleThread *thread, ScaleWindow *window, 
	int *orig_dimension, int *scale_dimension, int *offsets)
 : BC_Canvas(x, y, 100, 100)
{
	this->thread = thread; 
	this->window = window;
	this->orig_dimension = orig_dimension; 
	this->scale_dimension = scale_dimension;
	this->offsets = offsets;
	button_down = 0;
}

ScalePosition::~ScalePosition() {}

int ScalePosition::get_scale()
{
	hscale = (float)get_w() / (orig_dimension[0] + scale_dimension[0] * 2);
	vscale = (float)get_h() / (orig_dimension[1] + scale_dimension[1] * 2);
return 0;
}

int ScalePosition::draw()
{
	clear_box(0, 0, get_w(), get_h());

// Disable offsets when not scaling data
	if(thread->scale_data)
	{
		set_color(RED);
		draw_box(1, 1, get_w() - 3, get_h() - 3);
		set_color(YELLOW);
		draw_rectangle(0, 0, get_w() - 1, get_h() - 1);
	}
	else
	{
		int in_x, in_y, in_w, in_h;
		int out_x, out_y, out_w, out_h;

		get_scale();

		in_x = (int)(scale_dimension[0] * hscale);
		in_w = (int)(orig_dimension[0] * hscale);
		in_y = (int)(scale_dimension[1] * vscale);
		in_h = (int)(orig_dimension[1] * vscale);
		out_x = (int)(in_x + ((float)orig_dimension[0] / 2 + offsets[0] - (float)scale_dimension[0] / 2) * hscale);
		out_w = (int)(scale_dimension[0] * hscale);
		out_y = (int)(in_y + ((float)orig_dimension[1] / 2 + offsets[1] - (float)scale_dimension[1] / 2) * vscale);
		out_h = (int)(scale_dimension[1] * vscale);

//printf("ScalePosition::draw %d %d %d %d\n", out_x, out_y, out_w, out_h);
		set_color(RED);
		draw_box(out_x, out_y, out_w, out_h);
		set_color(YELLOW);
		draw_rectangle(in_x, in_y, in_w, in_h);
	}
	flash();
return 0;
}

int ScalePosition::button_press()
{
	int result = 0;
	if(get_cursorx() > 0 && get_cursory() > 0 && 
		get_cursorx() < get_w() && get_cursory() < get_h())
	{
		button_down = 1;
		center_x = get_cursorx();
		center_y = get_cursory();
		get_scale();
		offset_x = (int)(offsets[0] * hscale);
		offset_y = (int)(offsets[1] * vscale);
		result = 1;
	}

	return result;
return 0;
}

int ScalePosition::cursor_motion()
{
	int result = 0;
	get_scale();
	if(button_down)
	{
		offsets[0] = (int)((offset_x + get_cursor_x() - center_x) / hscale);
		offsets[1] = (int)((offset_y + get_cursor_y() - center_y) / vscale);
		thread->update_window();
		result = 1;
	}
	return result;
return 0;
}

int ScalePosition::button_release()
{
	button_down = 0;
	return 0;
return 0;
}

ScaleAspectAuto::ScaleAspectAuto(int x, int y, ScaleThread *thread)
 : BC_CheckBox(x, y, 16, 16, thread->auto_aspect, "Auto")
{ this->thread = thread; }

ScaleAspectAuto::~ScaleAspectAuto()
{
}

int ScaleAspectAuto::handle_event()
{
	thread->auto_aspect = get_value();
	thread->update_aspect(thread->window);
return 0;
}




ScaleAspectW::ScaleAspectW(int x, int y, ScaleThread *thread, float *output, char *string)
 : BC_TextBox(x, y, 50, string)
{
	this->output = output;
	this->thread = thread;
}
ScaleAspectW::~ScaleAspectW()
{
}

int ScaleAspectW::handle_event()
{
	*output = atof(get_text());
return 0;
}


ScaleAspectH::ScaleAspectH(int x, int y, ScaleThread *thread, float *output, char *string)
 : BC_TextBox(x, y, 50, string)
{
	this->output = output;
	this->thread = thread;
}
ScaleAspectH::~ScaleAspectH()
{
}

int ScaleAspectH::handle_event()
{
	*output = atof(get_text());
return 0;
}



ScaleOK::ScaleOK(int x, int y, ScaleWindow *window)
 : BC_BigButton(x, y, "OK")
{ this->window = window; }
ScaleOK::~ScaleOK() {}
int ScaleOK::handle_event()
{
	window->set_done(0);
return 0;
}

int ScaleOK::keypress_event()
{
	if(get_keypress() == 13) { handle_event(); return 1; }
	return 0;
return 0;
}




ScaleCancel::ScaleCancel(int x, int y, ScaleWindow *window)
 : BC_BigButton(x, y, "Cancel")
{ this->window = window; }

ScaleCancel::~ScaleCancel() {}
int ScaleCancel::handle_event()
{
	window->set_done(1);
return 0;
}

int ScaleCancel::keypress_event()
{
	if(get_keypress() == ESC) { handle_event(); return 1; }
	return 0;
return 0;
}
