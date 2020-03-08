#ifndef BLUEWINDOW_H
#define BLUEWINDOW_H

#include "bcbase.h"

class BlueThread;
class BlueWindow;

#include "filehtal.h"
#include "mutex.h"
#include "bluescreen.h"

class BlueThread : public Thread
{
public:
	BlueThread(BluescreenMain *client);
	~BlueThread();

	void run();

	Mutex gui_started; // prevent loading data until the GUI is started
	BluescreenMain *client;
	BlueWindow *window;
};

class PaletteWheel;
class PaletteWheelValue;
class PaletteOutput;
class PaletteHue;
class PaletteSaturation;
class PaletteValue;
class PaletteRed;
class PaletteGreen;
class PaletteBlue;
class Threshold;
class Feather;

class BlueWindow : public BC_Window
{
public:
	BlueWindow(BluescreenMain *client);
	~BlueWindow();

	int create_objects();
	int close_event();
	
	BluescreenMain *client;
	PaletteWheel *wheel;
	PaletteWheelValue *wheel_value;
	PaletteOutput *output;
	
	PaletteHue *hue;
	PaletteSaturation *saturation;
	PaletteValue *value;
	PaletteRed *red;
	PaletteGreen *green;
	PaletteBlue *blue;
	Threshold *threshold;
	Feather *feather;
	VFrame *value_bitmap;
};

class Threshold : public BC_ISlider
{
public:
	Threshold(BluescreenMain *client, int x, int y);
	~Threshold();
	int handle_event();
	BluescreenMain *client;
};

class Feather : public BC_ISlider
{
public:
	Feather(BluescreenMain *client, int x, int y);
	~Feather();
	int handle_event();
	BluescreenMain *client;
};

class PaletteWheel : public BC_Canvas
{
public:
	PaletteWheel(BluescreenMain *client, int x, int y);
	~PaletteWheel();
	int button_press();
	int cursor_motion();
	int button_release();

	int initialize();
	int draw(float hue, float saturation);
	int get_angle(float x1, float y1, float x2, float y2);
	float torads(int angle);
	BluescreenMain *client;
	float oldhue;
	float oldsaturation;
	int button_down;
};

class PaletteWheelValue : public BC_Canvas
{
public:
	PaletteWheelValue(BluescreenMain *client, int x, int y);
	~PaletteWheelValue();
	int initialize();
	int button_press();
	int cursor_motion();
	int button_release();
	int draw(float hue, float saturation, float value);
	BluescreenMain *client;
	int button_down;
	VFrame *frame;
};

class PaletteOutput : public BC_Canvas
{
public:
	PaletteOutput(BluescreenMain *client, int x, int y);
	~PaletteOutput();
	int initialize();
	int handle_event();
	int draw();
	BluescreenMain *client;
};

class PaletteHue : public BC_ISlider
{
public:
	PaletteHue(BluescreenMain *client, int x, int y);
	~PaletteHue();
	int handle_event();
	BluescreenMain *client;
};

class PaletteSaturation : public BC_FSlider
{
public:
	PaletteSaturation(BluescreenMain *client, int x, int y);
	~PaletteSaturation();
	int handle_event();
	BluescreenMain *client;
};

class PaletteValue : public BC_FSlider
{
public:
	PaletteValue(BluescreenMain *client, int x, int y);
	~PaletteValue();
	int handle_event();
	BluescreenMain *client;
};

class PaletteRed : public BC_FSlider
{
public:
	PaletteRed(BluescreenMain *client, int x, int y);
	~PaletteRed();
	int handle_event();
	BluescreenMain *client;
};

class PaletteGreen : public BC_FSlider
{
public:
	PaletteGreen(BluescreenMain *client, int x, int y);
	~PaletteGreen();
	int handle_event();
	BluescreenMain *client;
};

class PaletteBlue : public BC_FSlider
{
public:
	PaletteBlue(BluescreenMain *client, int x, int y);
	~PaletteBlue();
	int handle_event();
	BluescreenMain *client;
};



#endif
