#include <stdio.h>


#define PROGRESS_LEFT 0
#define PROGRESS_MID 1
#define PROGRESS_RIGHT 2
#define PROGRESS_LEFT_HI 3
#define PROGRESS_MID_HI 4
#define PROGRESS_RIGHT_HI 5
#define PROGRESS_STATES 6

#include "colors.h"
#include "fonts.h"
#include "bcprogress.h"
#include "bcpixmap.h"
#include "bcresources.h"

BC_ProgressBar::BC_ProgressBar(int x, int y, int w, long length)
 : BC_SubWindow(x, y, w, 0, -1)
{
	this->length = length;
	position = 0;
	pixel = 0;
	for(int i = 0; i < PROGRESS_STATES; i++) images[i] = 0;
}

int BC_ProgressBar::initialize()
{
	set_images();
	h = images[PROGRESS_MID]->get_h();

	BC_SubWindow::initialize();
	draw(1);
	return 0;
}

int BC_ProgressBar::set_images()
{
	for(int i = 0; i < PROGRESS_STATES; i++)
		if(images[i]) delete images[i];

	for(int i = 0; i < PROGRESS_STATES; i++)
		images[i] = new BC_Pixmap(parent_window, get_resources()->progress_images[i], PIXMAP_ALPHA);
	return 0;
}


int BC_ProgressBar::draw(int force)
{
	char string[32];
	int new_pixel;
	int left_division = images[PROGRESS_LEFT]->get_w_fixed();
	int right_division = get_w() - images[PROGRESS_RIGHT]->get_w_fixed();

	new_pixel = (int)(((float)position / length) * get_w());

	if(new_pixel != pixel || force)
	{
		pixel = new_pixel;
// Clear background
		draw_top_background(parent_window, 0, 0, get_w(), get_h());

		for(int x = 0; x < get_w(); )
		{
			int image_number;
			int source_x, source_w;

			if(x < left_division) 
			{
				image_number = PROGRESS_LEFT;
				source_x = x;
				source_w = left_division - source_x;
			}
			else
			if(x >= right_division) 
			{
				image_number = PROGRESS_RIGHT;
				source_x = x - right_division;
				source_w = get_w() - x;
			}
			else
			{
				image_number = PROGRESS_MID;
				source_x = 0;
				source_w = right_division - x;
				if(source_w > images[PROGRESS_MID]->get_w_fixed())
					source_w = images[PROGRESS_MID]->get_w_fixed();
			}

// Completely in highlighted region
			if(pixel >= x + source_w)
			{
				image_number += 3;
			}
			else
// Partially in highlighted region
			if(pixel > x)
			{
				image_number += 3;
				if(pixel - x < source_w)
					source_w = pixel - x;
			}

			images[image_number]->write_drawable(pixmap, 
				x, 
				0, 
				source_w + 1, 
				images[image_number]->get_h(),
				source_x,
				0);

			x += source_w;
		}

		set_font(MEDIUMFONT);
		set_color(BLACK);     // draw decimal percentage
		sprintf(string, "%d%%", (int)(100 * (float)position / length + 0.5 / w));
		draw_center_text(w / 2, h / 2 + get_text_ascent(MEDIUMFONT) / 2, string);
		flash();
	}
	return 0;
}

int BC_ProgressBar::update(long position)
{
	this->position = position;
	draw();
	return 0;
}

int BC_ProgressBar::update_length(long length)
{
	this->length = length;
	position = 0;

	draw();
	return 0;
}

