#include <string.h>
#include <stdio.h>


#include "bccolors.h"
#include "bcfont.h"
#include "bcprogress.h"
#include "bcresources.h"

BC_ProgressBar::BC_ProgressBar(int x_, int y_, int w_, int h_, long length_)
 : BC_Tool(x_, y_, w_, h_)
{
	length = length_;
	position = 0;
	percentage = 0;
}

int BC_ProgressBar::create_tool_objects()
{
	create_window(x, y, w, h, MEGREY);
	draw();
return 0;
}

int BC_ProgressBar::draw()
{
	char string[32];
	int real_percentage;
	
	draw_3d_big(0, 0, w, h, 
		get_resources()->button_shadow,
		BLACK,
		get_resources()->button_up,
		get_resources()->button_up,
		get_resources()->button_light);
	real_percentage = percentage;
	if(real_percentage > w - 4) real_percentage = w - 4;

	if(percentage > 0) 
		draw_3d_big(2, 2, real_percentage, h-4, 
			WHITE, LTGREEN, GREEN, DKGREEN, BLACK);

	set_font(MEDIUMFONT);
	set_color(BLACK);     // draw decimal percentage
	sprintf(string, "%d%%", (int)(100 * (float)real_percentage / w));
	draw_text(w / 2, 20, string);
	flash();
return 0;
}

int BC_ProgressBar::update(long position_)
{
	position = position_;
	int newpercentage;
	newpercentage = (int)((float)position / (length ? length : 1) * (w - 4));
	if(newpercentage != percentage)
	{
		percentage = newpercentage;
		draw();
	}
return 0;
}

int BC_ProgressBar::update_length(long length)
{
	this->length = length;
	position = 0;
	percentage = 0;

	draw();
return 0;
}
