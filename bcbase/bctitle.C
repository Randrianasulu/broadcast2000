#include <string.h>
#include "bctitle.h"
#include "bcwindowbase.h"

BC_Title::BC_Title(int x, int y, char *text, int font, int color)
 : BC_Tool(x, y, w, h)
{
	this->font = font;
	this->color = color;
	strcpy(this->text, text);
}

int BC_Title::create_tool_objects()
{
	w = get_text_width(font, text) + 5;
	h = get_text_height(font);
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_Title::set_color(int color)
{
	this->color = color;
	draw();
return 0;
}

int BC_Title::resize(int w, int h)
{
	draw();
return 0;
}

int BC_Title::resize_tool(int x, int y)
{
	resize_window(x, y, w, h);
	draw();
return 0;
}


int BC_Title::update(char *text)
{
	strcpy(this->text, text);
	int new_w;
	
	new_w = get_text_width(font, text) + 5;
	if(new_w > w)
	{
		resize_window(x, y, new_w, h);
	}
	else
	{
		BC_Tool::set_color(BC_Tool::color);
		draw_box(0, 0, w, h);      // wipe clear
	}
	draw();
return 0;
}

int BC_Title::draw()
{
	set_font(font);
	BC_Tool::set_color(color);
	draw_text(0, get_text_ascent(font), text);
	set_font(MEDIUMFONT);    // reset
	flash();
return 0;
}
