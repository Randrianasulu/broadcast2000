#include "bluewindow.h"


BlueThread::BlueThread(BluescreenMain *client)
 : Thread()
{
	this->client = client;
	synchronous = 1; // make thread wait for join
	gui_started.lock();
}

BlueThread::~BlueThread()
{
}
	
void BlueThread::run()
{
	window = new BlueWindow(client);
	window->create_objects();
	gui_started.unlock();
	window->run_window();
	delete window;
}






BlueWindow::BlueWindow(BluescreenMain *client)
 : BC_Window("", MEGREY, client->gui_string, 410, 290, 0, 0, 0, !client->show_initially)
{
	this->client = client; 
}

BlueWindow::~BlueWindow()
{
	delete value_bitmap;
}

int BlueWindow::create_objects()
{
	int x = 10, init_x = 10, y = 10, init_y = 10;
	add_tool(wheel = new PaletteWheel(client, x, y));
	wheel->initialize();
	
	x += 140;
	add_tool(wheel_value = new PaletteWheelValue(client, x, y));
	wheel_value->initialize();
	
	x = init_x; y += 140;
	add_tool(output = new PaletteOutput(client, x, y));
	output->initialize();
	
	x += 240; y = init_y;
	add_tool(new BC_Title(x, y, "Hue", SMALLFONT));
	y += 15;
	add_tool(hue = new PaletteHue(client, x, y));
	y += 30;
	add_tool(new BC_Title(x, y, "Saturation", SMALLFONT));
	y += 15;
	add_tool(saturation = new PaletteSaturation(client, x, y));
	y += 30;
	add_tool(new BC_Title(x, y, "Value", SMALLFONT));
	y += 15;
	add_tool(value = new PaletteValue(client, x, y));
	y += 30;
	add_tool(new BC_Title(x, y, "Red", SMALLFONT));
	y += 15;
	add_tool(red = new PaletteRed(client, x, y));
	y += 30;
	add_tool(new BC_Title(x, y, "Green", SMALLFONT));
	add_tool(new BC_Title(init_x, y, "Threshold", SMALLFONT));
	y += 15;
	add_tool(green = new PaletteGreen(client, x, y));
	add_tool(threshold = new Threshold(client, init_x, y));
	y += 30;
	add_tool(new BC_Title(x, y, "Blue", SMALLFONT));
//	add_tool(new BC_Title(init_x, y, "Feather", SMALLFONT));
	y += 15;
	add_tool(blue = new PaletteBlue(client, x, y));
//	add_tool(feather = new Feather(client, init_x, y));
	value_bitmap = new VFrame(0, 40, 130);
return 0;
}

int BlueWindow::close_event()
{
	hide_window();
	client->send_hide_gui();
return 0;
}

Threshold::Threshold(BluescreenMain *client, int x, int y)
 : BC_ISlider(x, y, 150, 25, 200, (int)client->threshold, 0, 255, 0)
{
	this->client = client;
}
Threshold::~Threshold()
{
}

int Threshold::handle_event()
{
	client->threshold = (int)get_value();
	client->send_configure_change();
return 0;
}


Feather::Feather(BluescreenMain *client, int x, int y)
 : BC_ISlider(x, y, 150, 25, 200, (int)client->feather, 0, 255, 0)
{
	this->client = client;
}
Feather::~Feather()
{
}

int Feather::handle_event()
{
	client->feather = (int)get_value();
	client->send_configure_change();
return 0;
}


PaletteWheel::PaletteWheel(BluescreenMain *client, int x, int y)
 : BC_Canvas(x, y, 130, 130)
{
	this->client = client;
	oldhue = 0;
	oldsaturation = 0;
	button_down = 0;
}
PaletteWheel::~PaletteWheel()
{
}

int PaletteWheel::button_press()
{
	if(get_cursor_x() >= 0 && get_cursor_x() < get_w() &&
		get_cursor_y() >= 0 && get_cursor_y() < get_h())
	{
		button_down = 1;
		cursor_motion();
		return 1;
	}
	return 0;
}

int PaletteWheel::cursor_motion()
{
	int x1, y1, distance;
	if(button_down)
	{
		client->h = get_angle(get_w() / 2, get_h() / 2, get_cursor_x(), get_cursor_y());
		x1 = get_w() / 2 - get_cursor_x();
		y1 = get_h() / 2 - get_cursor_y();
		distance = (int)sqrt(x1 * x1 + y1 * y1);
		if(distance > get_w() / 2) distance = get_w() / 2;
		client->s = (float)distance / (get_w() / 2);
		client->update_display();
		client->send_configure_change();
		return 1;
	}
	return 0;
}

int PaletteWheel::button_release()
{
	button_down = 0;
	return 0;
}

int PaletteWheel::initialize()
{
// Upper right
	float h;
	float s;
	float v = 1;
	float r, g, b;
	float x1, y1, x2, y2;
	float distance;
	int default_r, default_g, default_b;
	VFrame frame(0, get_w(), get_h());
	x1 = get_w() / 2;
	y1 = get_h() / 2;
	default_r = (get_resources()->get_bg_color() & 0xff0000) >> 16;
	default_g = (get_resources()->get_bg_color() & 0xff00) >> 8;
	default_b = (get_resources()->get_bg_color() & 0xff);

	for(y2 = 0; y2 < get_h(); y2++)
	{
		for(x2 = 0; x2 < get_w(); x2++)
		{
			((VPixel**)frame.get_rows())[(int)y2][(int)x2].a = VMAX;
			distance = sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1));
			if(distance > x1)
			{
				((VPixel**)frame.get_rows())[(int)y2][(int)x2].r = default_r;
				((VPixel**)frame.get_rows())[(int)y2][(int)x2].g = default_g;
				((VPixel**)frame.get_rows())[(int)y2][(int)x2].b = default_b;
			}
			else
			{
				h = get_angle(x1, y1, x2, y2);
				s = distance / x1;
				client->hsv_to_rgb(r, g, b, h, s, v);
				((VPixel**)frame.get_rows())[(int)y2][(int)x2].r = (int)(r * VMAX);
				((VPixel**)frame.get_rows())[(int)y2][(int)x2].g = (int)(g * VMAX);
				((VPixel**)frame.get_rows())[(int)y2][(int)x2].b = (int)(b * VMAX);
			}
		}
	}
	draw_bitmap(&frame, 0, 0, get_w(), get_h(), 0, 0, get_w(), get_h(), 0);
	oldhue = client->h;
	oldsaturation = client->s;
	draw(oldhue, oldsaturation);
	flash();
return 0;
}

float PaletteWheel::torads(int angle)
{
	return (float)angle / 360 * 2 * M_PI;
}


int PaletteWheel::draw(float hue, float saturation)
{
	int x, y, w, h;
	w = get_w() / 2;
	h = get_h() / 2;

	if(hue > 0 && hue < 90)
	{
		x = (int)(w + w * cos(torads(90 - hue)) * saturation);
		y = (int)(h - h * sin(torads(90 - hue)) * saturation);
	}
	else
	if(hue > 90 && hue < 180)
	{
		x = (int)(w + w * cos(torads(hue - 90)) * saturation);
		y = (int)(h + h * sin(torads(hue - 90)) * saturation);
	}
	else
	if(hue > 180 && hue < 270)
	{
		x = (int)(w - w * cos(torads(270 - hue)) * saturation);
		y = (int)(h + h * sin(torads(270 - hue)) * saturation);
	}
	else
	if(hue > 270 && hue < 360)
	{
		x = (int)(w - w * cos(torads(hue - 270)) * saturation);
		y = (int)(h - w * sin(torads(hue - 270)) * saturation);
	}
	else
	if(hue == 0) 
	{
		x = w;
		y = (int)(h - h * saturation);
	}
	else
	if(hue == 90)
	{
		x = (int)(w + w * saturation);
		y = h;
	}
	else
	if(hue == 180)
	{
		x = w;
		y = (int)(h + h * saturation);
	}
	else
	if(hue == 270)
	{
		x = (int)(w - w * saturation);
		y = h;
	}
	set_inverse();
	set_color(WHITE);
	draw_circle(x - 5, y - 5, 10, 10);
	set_opaque();
return 0;
}

int PaletteWheel::get_angle(float x1, float y1, float x2, float y2)
{
	float result;
	
	if(x2 > x1 && y2 < y1)
// Top right
		result = 90 - atan((y1 - y2) / (x2 - x1)) / M_PI / 2 * 360;
	else
	if(x2 < x1 && y2 < y1)
// Top left
		result = 270 + atan((y1 - y2) / (x1 - x2)) / M_PI / 2 * 360;
	else
	if(x2 > x1 && y2 > y1)
// Bottom right
		result = 90 + atan((y2 - y1) / (x2 - x1)) / M_PI / 2 * 360;
	else
	if(x2 < x1 && y2 > y1)
// Bottom left
		result = 270 - atan((y2 - y1) / (x1 - x2)) / M_PI / 2 * 360;
	else
		if(x2 == x1 && y2 < y1) result = 0;
	else
		if(x2 == x1 && y2 > y1) result = 180;
	else
		if(x2 > x1 && y2 == y1) result = 90;
	else
		if(x2 < x1 && y2 == y1) result = 270;
	else 
		result = 0;
	return (int)result;
}

PaletteWheelValue::PaletteWheelValue(BluescreenMain *client, int x, int y)
 : BC_Canvas(x, y, 40, 130, BLACK)
{
	this->client = client;
	button_down = 0;
}
PaletteWheelValue::~PaletteWheelValue()
{
	delete frame;
}

int PaletteWheelValue::initialize()
{
	frame = new VFrame(0, get_w(), get_h());
	draw(client->h, client->s, client->v);
	flash();
return 0;
}

int PaletteWheelValue::button_press()
{
	if(get_cursor_x() >= 0 && get_cursor_x() < get_w() &&
		get_cursor_y() >= 0 && get_cursor_y() < get_h())
	{
		button_down = 1;
		cursor_motion();
		return 1;
	}
	return 0;
}

int PaletteWheelValue::cursor_motion()
{
	int x1, y1, distance;
	if(button_down)
	{
		client->v = (float)(get_h() - get_cursor_y()) / get_h();
		client->update_display();
		client->send_configure_change();
		return 1;
	}
	return 0;
}

int PaletteWheelValue::button_release()
{
	button_down = 0;
	return 0;
}

int PaletteWheelValue::draw(float hue, float saturation, float value)
{
	float r_f, g_f, b_f;
	int i, j, r, g, b;
	for(i = get_h() - 1; i >= 0; i--)
	{
		client->hsv_to_rgb(r_f, g_f, b_f, hue, saturation, (float)(get_h() - 1 - i) / get_h());
		r = (int)(r_f * 255);
		g = (int)(g_f * 255);
		b = (int)(b_f * 255);
		for(j = 0; j < get_w(); j++)
		{
 			((VPixel**)frame->get_rows())[i][j].r = r;
 			((VPixel**)frame->get_rows())[i][j].g = g;
 			((VPixel**)frame->get_rows())[i][j].b = b;
		}
	}
	draw_bitmap(frame, 0, 0, get_w(), get_h(), 0, 0, get_w(), get_h(), 0);
	set_color(BLACK);
	draw_line(0, get_h() - value * get_h(), get_w(), get_h() - value * get_h());
return 0;
}

PaletteOutput::PaletteOutput(BluescreenMain *client, int x, int y)
 : BC_Canvas(x, y, 180, 30, BLACK)
{
	this->client = client;
}
PaletteOutput::~PaletteOutput()
{
}


int PaletteOutput::initialize()
{
	draw();
	flash();
return 0;
}

int PaletteOutput::handle_event()
{
return 0;
}

int PaletteOutput::draw()
{
	float r_f, g_f, b_f;
	
	client->hsv_to_rgb(r_f, g_f, b_f, client->h, client->s, client->v);
	set_color(((int)(r_f * 255) << 16) | ((int)(g_f * 255) << 8) | ((int)(b_f * 255)));
	draw_box(0, 0, get_w(), get_h());
return 0;
}

PaletteHue::PaletteHue(BluescreenMain *client, int x, int y)
 : BC_ISlider(x, y, 150, 25, 200, (int)(client->h), 0, 359, 0)
{
	this->client = client;
}
PaletteHue::~PaletteHue()
{
}

int PaletteHue::handle_event()
{
	client->h = get_value();
	client->update_display();
	client->send_configure_change();
return 0;
}

PaletteSaturation::PaletteSaturation(BluescreenMain *client, int x, int y)
 : BC_FSlider(x, y, 150, 25, 200, client->s, 0, 1, 0)
{
	this->client = client;
}
PaletteSaturation::~PaletteSaturation()
{
}

int PaletteSaturation::handle_event()
{
	client->s = get_value();
	client->update_display();
	client->send_configure_change();
return 0;
}

PaletteValue::PaletteValue(BluescreenMain *client, int x, int y)
 : BC_FSlider(x, y, 150, 25, 200, client->v, 0, 1, 0)
{
	this->client = client;
}
PaletteValue::~PaletteValue()
{
}

int PaletteValue::handle_event()
{
	client->v = get_value();
	client->update_display();
	client->send_configure_change();
return 0;
}


PaletteRed::PaletteRed(BluescreenMain *client, int x, int y)
 : BC_FSlider(x, y, 150, 25, 200, client->r, 0, 1, 0)
{
	this->client = client;
}
PaletteRed::~PaletteRed()
{
}

int PaletteRed::handle_event()
{
	client->update_rgb();
	client->send_configure_change();
return 0;
}

PaletteGreen::PaletteGreen(BluescreenMain *client, int x, int y)
 : BC_FSlider(x, y, 150, 25, 200, client->g, 0, 1, 0)
{
	this->client = client;
}
PaletteGreen::~PaletteGreen()
{
}

int PaletteGreen::handle_event()
{
	client->update_rgb();
	client->send_configure_change();
return 0;
}

PaletteBlue::PaletteBlue(BluescreenMain *client, int x, int y)
 : BC_FSlider(x, y, 150, 25, 200, client->b, 0, 1, 0)
{
	this->client = client;
}
PaletteBlue::~PaletteBlue()
{
}

int PaletteBlue::handle_event()
{
	client->update_rgb();
	client->send_configure_change();
return 0;
}
