#include "guicast.h"
#include "keys.h"
#include "vframe.h"
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <unistd.h>

class TestWindow : public BC_Window
{
public:
	TestWindow() : BC_Window("Test", 0, 0, 640, 480, 640, 480)
	{
	};
	
	void create_objects()
	{
		add_subwindow(meter = new BC_Meter(10, 10, METER_VERT, get_h() - 20, INFINITYGAIN, METER_DB, 1, 150, 15));
	};
	
	int close_event()
	{
		set_done(0);
		return 1;
	};
	
	int keypress_event()
	{
		if(get_keypress() == ESC || tolower(get_keypress()) == 'q')
		{
			set_done(0);
			return 1;
		}
	};

	BC_Meter *meter;
};

int main()
{
	TestWindow window;
	BC_Title *title;

	window.create_objects();
	window.meter->update(DB::fromdb(0), 1);
	window.run_window();
}
