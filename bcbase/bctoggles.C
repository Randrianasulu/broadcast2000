#include <string.h>
#include "bctitle.h"
#include "bcresources.h"
#include "bctoggles.h"

BC_Toggle::BC_Toggle(int x_, int y_, int w_, int h_, int down_, char *text)
	: BC_Tool(x_, y_, w_, h_)
{
	down = down_;
	highlighted = 0;
	this->text = text;
}

int BC_Toggle::create_tool_objects()
{
	create_window(x, y, w, h, MEGREY);
	if(text) subwindow->add_tool(new BC_Title(x + w + 5, y, text));
return 0;
}

int BC_Toggle::cursor_left_()
{
	if(highlighted)
	{
		if(get_cursor_x() < 0 || get_cursor_x() > w ||
			 get_cursor_y() < 0 || get_cursor_y() > h)
		{   // draw unhighlighted
			highlighted = 0;
			draw();
		}
		return 0;
	}
return 0;
}

int BC_Toggle::button_press_()
{
	if(  // subwindow test
		subwindow->get_cursor_x() > 0 && subwindow->get_cursor_x() < subwindow->get_w()
			&& subwindow->get_cursor_y() > 0 && subwindow->get_cursor_y() < subwindow->get_h()
		&& 	 // label test
		get_cursor_x() > 0 && get_cursor_x() < w
			&& get_cursor_y() > 0 && get_cursor_y() < h
		 )
	{
		activate();
		down ^= 1;
		draw();
		button_press();
		handle_event(); // status changed
		return 1;
	}
	return 0;
return 0;
}

int BC_Toggle::button_release_()
{
	button_release();      // user event
return 0;
}

int BC_Toggle::unhighlight_()
{
	if(highlighted)
	{
		highlighted = 0;
		draw();
	}
	return 0;
return 0;
}

int BC_Toggle::cursor_motion_()
{
	int result;
	result = 0;

	if(  // subwindow test
		subwindow->get_cursor_x() > 0 && subwindow->get_cursor_x() < subwindow->get_w()
			&& subwindow->get_cursor_y() > 0 && subwindow->get_cursor_y() < subwindow->get_h()
		&& 	 // label test
		get_cursor_x() > 0 && get_cursor_x() < w
			&& get_cursor_y() > 0 && get_cursor_y() < h)
	{
// Highlight
//		result = 1;
		if(!highlighted)
		{
			top_level->unhighlight();
			highlighted = 1;
			draw();
			cursor_moved_over();    // user can recieve cursor motion over
		}
	}
	else
	if(highlighted)
	{
// Unhighlight
		highlighted = 0;
		draw();
	}
	return result;
return 0;
}

int BC_Toggle::resize(int x_, int y_, int w_, int h_, int down_)
{
	x = x_; y = y_; w = w_; h = h_;
	down = down_;
	resize_window(x, y, w, h);
return 0;
}

int BC_Toggle::resize_tool(int x, int y)
{
	resize_window(x, y, w, h);
	draw();
return 0;
}

int BC_Toggle::update(int down_)
{
	down = down_;
	draw();
return 0;
}

int BC_Toggle::get_value()
{
	return down;
return 0;
}

int BC_Toggle::draw_disc()
{
	if(!down)
	{
		if(highlighted)
			draw_3d_circle(0, 0, w, h, 
				top_level->get_resources()->button_light, 
				top_level->get_resources()->button_highlighted, 
				top_level->get_resources()->button_highlighted, 
				top_level->get_resources()->button_shadow, 
				BLACK);
		else
			draw_3d_circle(0, 0, w, h, 
				top_level->get_resources()->button_light, 
				top_level->get_resources()->button_up, 
				top_level->get_resources()->button_up, 
				top_level->get_resources()->button_shadow, 
				BLACK);
	}
	else
	{
		if(highlighted)
			draw_3d_circle(0, 0, w, h, 
				top_level->get_resources()->button_shadow, 
				BLACK, 
				PINK, 
				PINK,
				top_level->get_resources()->button_light);
		else
			draw_3d_circle(0, 0, w, h, 
				top_level->get_resources()->button_shadow, 
				BLACK, 
				RED, 
				RED,
				top_level->get_resources()->button_light);
	}
return 0;
}


BC_Radial::BC_Radial(int x_, int y_, int w_, int h_, int down, char *text)
	: BC_Toggle(x_, y_, w_, h_, down, text)
{
}

int BC_Radial::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	if(text) subwindow->add_tool(new BC_Title(x + w + 5, y, text));
	draw();
return 0;
}

int BC_Radial::draw()
{
	draw_disc();
	flash();
return 0;
}

BC_CheckBox::BC_CheckBox(int x_, int y_, int w_, int h_, int down, char *text, char letter)
	: BC_Toggle(x_, y_, w_, h_, down, text)
{
	this->letter = letter;
}

int BC_CheckBox::create_tool_objects()
{
	create_window(x, y, w, h, MEGREY);
	if(text) subwindow->add_tool(new BC_Title(x + w + 5, y, text));
	draw();
return 0;
}

int BC_CheckBox::draw()
{
	draw_box_colored(0, 0, w, h, down, highlighted);

	if(letter)
	{
		char string[2];

		string[0] = letter;
		string[1] = 0;

		if(down)
			set_color(RED);
		else
			set_color(BLACK);
		
		draw_text(2, h - 2, string);
	}
	else
	if(down)
	{
// draw the checkmark
		set_color(BLACK);
		draw_line(4, h/2-1, 6, h/2+1);
		draw_line(4, h/2+0, 6, h/2+2);
		draw_line(4, h/2+1, 6, h/2+3);
		draw_line(6, h/2+1, w - 5, h/2-4);
		draw_line(6, h/2+2, w - 5, h/2-3);
		draw_line(6, h/2+3, w - 5, h/2-2);
	}
	flash();
return 0;
}

BC_RecordPatch::BC_RecordPatch(int x_, int y_, int w_, int h_, int down)
	: BC_Toggle(x_, y_, w_, h_, down)
{
}

int BC_RecordPatch::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_RecordPatch::draw()
{
	draw_disc();
	flash();
return 0;
}

BC_PlayPatch::BC_PlayPatch(int x_, int y_, int w_, int h_, int down)
	: BC_Toggle(x_, y_, w_, h_, down)
{
}

int BC_PlayPatch::create_tool_objects()
{
	create_window(x, y, w, h, subwindow->get_color());
	draw();
return 0;
}

int BC_PlayPatch::draw()
{
	if(get_value())
	{
		if(highlighted)
			draw_triangle_right(0, 0, w, h, 
				get_resources()->button_shadow,
				BLACK,
				LTGREEN,
				LTGREEN,
				get_resources()->button_light);
		else
			draw_triangle_right(0, 0, w, h, 
				get_resources()->button_shadow,
				BLACK,
				GREEN,
				GREEN,
				get_resources()->button_light);
	}
	else
	{
		if(highlighted)
			draw_triangle_right(0, 0, w, h, 
				get_resources()->button_light,
				get_resources()->button_highlighted,
				get_resources()->button_highlighted,
				get_resources()->button_shadow,
				BLACK);
		else
			draw_triangle_right(0, 0, w, h, 
				get_resources()->button_light,
				get_resources()->button_up,
				get_resources()->button_up,
				get_resources()->button_shadow,
				BLACK);
	}
	flash();
return 0;
}

// ================================= labels ================================

BC_Label::BC_Label(int x_, int y_, int w_, int h_, int down)
	: BC_Toggle(x_, y_, w_, h_, down)
{
	// draw();
	// x isn't set in the constructor
}

int BC_Label::create_tool_objects()
{
	create_window(x, y, w, h, MDPURPLE);
	draw();
return 0;
}

int BC_Label::set_status(int down_)
{
	down = down_;
	draw();
return 0;
}

int BC_Label::draw()
{
	if(down)
	{
		if(highlighted)
		draw_3d_big(0, 0, w, h, DKPURPLE, MEPURPLE, LTPURPLE);
		else
		draw_3d_big(0, 0, w, h, DKPURPLE, MDPURPLE, LTPURPLE);
	}
	else
	{
		if(highlighted)
		draw_3d_big(0, 0, w, h, WHITE, LTPURPLE, DKPURPLE);
		else
		draw_3d_big(0, 0, w, h, LTPURPLE, MEPURPLE, DKPURPLE);
	}
	flash();
return 0;
}
