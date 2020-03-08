#include <string.h>
#include "bcfont.h"
#include "bcresources.h"
#include "bctextarea.h"

// ===============================================

BC_TextAreaScroll::BC_TextAreaScroll(BC_TextArea *textarea, int totallines_, int yposition)
 : BC_YScrollBar(textarea->x + textarea->w, textarea->y, 17, textarea->h, totallines_, yposition, textarea->h / textarea->itemheight)
{
	this->textarea = textarea;
}

int BC_TextAreaScroll::handle_event()
{
	if(textarea->get_yposition() != get_position())
	{
		textarea->set_yposition(get_position());
	}
return 0;
}

// ===============================================

BC_TextArea::BC_TextArea(int x_, int y_, int w_, int h_, char *text, int yposition)
	: BC_Tool(x_, y_, w_ - 17, h_)
{
	this->text = text;
	this->yposition = yposition;
	button_down = highlighted = 0;
}

BC_TextArea::~BC_TextArea()
{
	delete scrollbar;
}

int BC_TextArea::create_tool_objects()
{
	itemheight = get_text_height(MEDIUMFONT);
	create_window(x, y, w, h, WHITE);
	scrollbar = new BC_TextAreaScroll(this, get_total_lines(), 0);
	subwindow->add_tool(scrollbar);
	draw();
return 0;
}

int BC_TextArea::resize(int w, int h)
{
	draw();
return 0;
}

int BC_TextArea::get_total_lines()
{
	int i, result;
	
	for(i = 0, result = 0; text[i] != 0; i++) if(text[i] == '\n') result++;
	return result;
return 0;
}

int BC_TextArea::draw()
{
	int i, j, y_, total_lines, text_pointer, result;
	char line[1024];

// find start in text
	text_pointer = 0;
	result = 0;
	for(i = 0; i < yposition && !result;)
	{
		if(text[text_pointer] == '\n') i++;
		if(text[text_pointer] == 0) result = 1;
		text_pointer++;
	}

// draw background
	if(highlighted)
		draw_3d_big(0, 0, w, h, 
			top_level->get_resources()->button_shadow, 
			RED, 
			WHITE, 
			LTPINK,
			top_level->get_resources()->button_light);
	else
		draw_3d_big(0, 0, w, h, 
			top_level->get_resources()->button_shadow, 
			BLACK, 
			WHITE, 
			top_level->get_resources()->button_up,
			top_level->get_resources()->button_light);

	// draw items
	set_color(BLACK);
	total_lines = get_total_lines();

	result = 0;
	for(i = yposition, y_ = get_text_ascent(MEDIUMFONT) + 2; i < total_lines && y_ <= h; i++, y_ += itemheight)
	{	
// load the line of text
		for(j = 0; text[text_pointer] != '\n' && text[text_pointer] != 0 && j < 80; j++, text_pointer++)
		{
			line[j] = text[text_pointer];
		}
		line[j] = 0;
		text_pointer++;  // get the eoln
		if(text[text_pointer] == 0) result = 1;
		
		draw_text(5, y_, line);
	}

	// flash just the box
	flash();
return 0;
}

int BC_TextArea::set_size(int x_, int y_, int w_, int h_)
{
	x = x_; y = y_; w = w_ - 17; h = h_;
	scrollbar->set_size(x + w, y, 17, h);
	draw();
return 0;
}

int BC_TextArea::set_contents(char *text, int yposition)
{
	this->text = text;
	this->yposition = yposition;
	
	scrollbar->set_position(get_total_lines(), yposition, h / itemheight);
	draw();
return 0;
}

int BC_TextArea::append_contents(char *new_text)
{
	int i, lines;
	strcat(text, new_text);
	for(i = 0, lines = 0; new_text[i] != 0; i++) if(new_text[i] == '\n') lines++;

	if(get_total_lines() + lines > yposition + h / itemheight)
	yposition += (get_total_lines() + lines) - (yposition + h / itemheight);
	
	scrollbar->set_position(get_total_lines(), yposition, h / itemheight);
	draw();
return 0;
}

int BC_TextArea::deactivate()
{
return 0;
}

int BC_TextArea::set_yposition(int yposition)
{
	this->yposition = yposition;
	scrollbar->set_position(get_total_lines(), yposition, h / itemheight);
	draw();
return 0;
}

int BC_TextArea::get_yposition()
{
	return yposition;
return 0;
}

int BC_TextArea::button_press_()
{
	if(cursor_x > 0 && cursor_x < w
		 && cursor_y > 0 && cursor_y < h)
	{
		button_down = 1;
		activate();
		return 1;
	}
	return 0;
return 0;
}

int BC_TextArea::cursor_left_()
{
	if(highlighted && !button_down)
	{
		highlighted = 0;
		draw();
	}
return 0;
}

int BC_TextArea::cursor_motion_()
{
	int result;
	result = 0;

	if(button_down)
	{
		if(get_cursor_y() < 0 && yposition > 0)
		{
			result = 1;
			set_yposition(yposition - 1);
		}
		else 
		if(get_cursor_y() > h && yposition < get_total_lines() - get_h() / itemheight)
		{
			result = 1;
			set_yposition(yposition + 1);
		}
	}
	else
	{
		if(get_cursor_x() > 0 && get_cursor_x() < w &&
			 get_cursor_y() > 0 && get_cursor_y() < h)
		{
			if(!highlighted)
			{
				top_level->unhighlight();
				highlighted = 1;
				draw();
			}
			result = 1;
		}
		else
		if(highlighted)
		{
			highlighted = 0;
			draw();
		}
	}
	return result;
return 0;
}

int BC_TextArea::button_release_()
{
	if(button_down)
	{
		button_down = 0;
	}
return 0;
}

int BC_TextArea::unhighlight_()
{
	if(highlighted)
	{
		highlighted = 0;
		draw();
	}
	return 0;
return 0;
}
