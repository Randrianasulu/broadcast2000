#include <string.h>
#include "bcresources.h"
#include "bcscrollbar.h"
//#include <sys/timeb.h>

BC_ScrollBar::BC_ScrollBar(int x_, int y_, int w_, int h_, long length_, long position_, long handlelength_)
	: BC_Tool(x_, y_, w_, h_)
{
	buttondown = backarrow = forwardarrow = box = backpage = forwardpage = 0;
	backhi = forwardhi = boxhi = 0;
	length = length_;
	position = position_;
	distance = 0;
	handlelength = handlelength_;
	init_repeat = 300;
	sustain_repeat = 100;
	next_repeat = sustain_repeat;
}

int BC_ScrollBar::create_tool_objects()
{
	create_window(x, y, w, h, MDGREY);
	draw();
return 0;
}

int BC_ScrollBar::resize(int x_, int y_, int w_, int h_, long length_, long position_, long handlelength_)
{
	x = x_; y = y_; w = w_; h = h_;
	length = length_;
	position = position_;
	handlelength = handlelength_;
	resize_window(x, y, w, h);
	//draw();
return 0;
}

int BC_ScrollBar::resize(int w, int h)
{
	//draw();	
return 0;
}

int BC_ScrollBar::button_release_()
{
	if(buttondown)
	{
		buttondown = backarrow = forwardarrow = box = backpage = forwardpage = 0;
		test_highlight();
		draw();
		unset_repeat();
	}
return 0;
}

int BC_ScrollBar::set_size(int x_, int y_, int w_, int h_)
{
	x = x_; y = y_; w = w_; h = h_;
	resize_window(x, y, w, h);

	draw();
return 0;
}

int BC_ScrollBar::set_position(long length, long position, long handlelength)
{
	this->length = length;
	this->position = position;
	this->handlelength = handlelength;
	draw();
return 0;
}

int BC_ScrollBar::set_position(long position)
{
	this->position = position;
	draw();
return 0;
}

int BC_ScrollBar::get_positions(int boxlength)
{
  if(length > handlelength)
  {
		handlew = (int)((float)boxlength / length * handlelength) + 2;
		if(handlew < 10)
		{          // length too small
			int boxlength_;   // imaginary boxlength
			
			handlew = 10;   // force length
			boxlength_ = boxlength - handlew;    // fake boxlength and actual length
    	handlex = (int)((float)boxlength_ / (length - handlelength) * position);
		}
		else
		{
    	handlex = (int)((float)boxlength / length * position);
    }
	}
	else
	{  // fill entire scrollbar with handle
    	handlex = 0;
    	handlew = boxlength;
	}
	
	if(handlex < 0) handlex = 0;
	if(handlex > boxlength)
	{
		handlex = 0; handlew = boxlength;  // give up if beyond end
	}
	else
	if(handlew + handlex > boxlength) handlew = boxlength - handlex;
return 0;
}

long BC_ScrollBar::get_position()
{
	return position;
}

long BC_ScrollBar::get_distance() { return distance; }       // old position of scrollbar

long BC_ScrollBar::get_length() { return length; }

int BC_ScrollBar::in_use()
{
	if(backarrow || forwardarrow || box || backpage || forwardpage) return 1;
	else
	return 0;
return 0;
}

int BC_ScrollBar::repeat_()         // need repeat routine since most events are Expose
{
	handle_arrows();
	handle_event();
	return 1;
return 0;
}

int BC_ScrollBar::unhighlight_()
{
	if(boxhi || backhi || forwardhi)
	{
		boxhi = backhi = forwardhi = 0;
		draw();
	}
	return 0;
return 0;
}

int BC_ScrollBar::handle_arrows(int change_repeat)
{
	long oldposition;
	int result = 0;
	distance = 0;

//printf("BC_ScrollBar::handle_arrows %x %d %d\n", this, change_repeat, next_repeat);
	if(backarrow)
	{
		if(change_repeat) set_repeat(next_repeat);
		oldposition = position;
		if(handlelength < 100) position -= 1; // for list boxes
		else position -= handlelength / 10;
		if(position < 0) position = 0;
		if(oldposition != position)
		{ 
			draw(); 
			result = 1; 
			distance = position - oldposition; 
		}
		else result = 0;
	}
	else if(forwardarrow)
	{
		if(change_repeat) set_repeat(next_repeat);

		oldposition = position;
		if(handlelength < 100) position += 1; // for list boxes
		else position += handlelength / 10;
		if(position > length - handlelength) position = length - handlelength;
		if(position < 0) position = 0; // the handlelength may be longer than the list
		if(oldposition != position)
		{ 
			draw(); 
			result = 1; 
			distance = position - oldposition; 
		}
		else result = 0;
	}
	else if(backpage)
	{
		if(change_repeat) set_repeat(next_repeat);

		oldposition = position;
		position -= handlelength;
		if(position < 0) position = 0;
		if(oldposition != position)
		{ 
			draw(); 
			result = 1; 
			distance = position - oldposition; 
		}
		else result = 0;
	}
	else if(forwardpage)
	{
		if(change_repeat) set_repeat(next_repeat);

		oldposition = position;
		position += handlelength;
		if(position > length - handlelength) position = length - handlelength;
		if(oldposition != position)
		{ 
			draw(); 
			result = 1; 
			distance = position - oldposition; 
		}
		else result = 0;
	}
	return result;
return 0;
}

//================================ x scrollbar=======================

BC_XScrollBar::BC_XScrollBar(int x_, int y_, int w_, int h_, long length_, long position_, long handlelength_)
	: BC_ScrollBar(x_, y_, w_, h_, length_, position_, handlelength_)
{
}

int BC_XScrollBar::test_highlight()
{
	int result;
	result = 0;
	if(!enabled) return 0;

	if(get_cursor_y() > 0 && get_cursor_y() < h)
	{
		if(get_cursor_x() > handlex + h && get_cursor_x() < handlex + h + handlew)
		{
			if(!boxhi)
			{
				top_level->unhighlight();
				boxhi = 1;
				draw();
			}
			result = 1;
		}
		else 
		if(get_cursor_x() > 0 && get_cursor_x() < h)
		{
			if(!backhi)
			{
				top_level->unhighlight();
				backhi = 1;
				draw();
			}
			result = 1;
		}
		else 
		if(get_cursor_x() > w - h && get_cursor_x() < w)
		{
			if(!forwardhi)
			{
				top_level->unhighlight();
				forwardhi = 1;
				draw();
			}
			result = 1;
		}
	}

	if(!result)
	{
		if(boxhi)
		{
			if(get_cursor_y() < 0 || get_cursor_y() > h ||
				 get_cursor_x() < handlex + h || get_cursor_x() > handlex + h + handlew)
			{
				boxhi = 0;
				draw();
			}
		}
		else if(backhi)
		{
			if(get_cursor_y() < 0 || get_cursor_y() > h ||
				 get_cursor_x() < 0 || get_cursor_x() > h)
			{
				backhi = 0;
				draw();
			}
		}
		else if(forwardhi)
		{
			if(get_cursor_y() < 0 || get_cursor_y() > h ||
				 get_cursor_x() < w - h || get_cursor_x() > w)
			{
				forwardhi = 0;
				draw();
			}
		}
	}
	return result;
return 0;
}

int BC_XScrollBar::cursor_left_()
{
// highlighting
	if(!buttondown && !backarrow && !forwardarrow && !box && !backpage && !forwardpage)
	{
		if(cursor_x < 0 || cursor_x > w || cursor_y < 0 || cursor_y > h)
		test_highlight();
	}
return 0;
}

int BC_XScrollBar::button_press_()
{
	int result = 0;

	next_repeat = sustain_repeat;
	if(!get_repeat()) 
	{
		if(cursor_y > 0 && cursor_y < h && cursor_x > 0 && cursor_x < w)
		{
			buttondown = 1;

    		if(cursor_x < h)
			{
    			set_repeat(next_repeat);
      			backarrow = 1;
      			draw();
    		}
			else
    		if(cursor_x > w - h)
			{
				set_repeat(next_repeat);
				forwardarrow = 1;
				draw();
    		}
			else
    		if(cursor_x > h && cursor_x < w - h) // between pointers
    		{
      			get_positions(w - h * 2);

      			if(cursor_x < handlex + h) 
      			{
     				set_repeat(next_repeat);
     				backpage = 1;
      			}
      			else
      			if(cursor_x > handlex + h + handlew) 
      			{
     				set_repeat(next_repeat);
     				forwardpage = 1;
      			}
      			else
      			{
        			oldx = cursor_x;
        			relativex = (int)(position / ((float)length / (w - h * 2)));
        			box = 1;
        			draw();
      			}
    		}
			result = 1;
  		}
	}
	if(handle_arrows(0)) handle_event();
	return result;
return 0;
}

int BC_XScrollBar::cursor_motion_()
{
	int result;
	result = 0;
	distance = 0;
	if(!buttondown && !backarrow && !forwardarrow && !box && !backpage && !forwardpage)
	{
		result = test_highlight();
	}

	if(box)
	{
		result = 1; 
  		oldposition = position;

  		relativex += cursor_x - oldx;
  		oldx = cursor_x;

  		position = (long)((float)length / (w - h * 2) * relativex);

  		if(position < 0) position = 0;
  		if(position > length - handlelength) position = length - handlelength;
  		if(position < 0) position = 0;
  		if(oldposition != position) 
		{ 
			draw(); 
			distance = position - oldposition; 
			handle_event(); 
		}
	}
	return result;
return 0;
}

int BC_XScrollBar::draw()
{
	if(w < h * 2 + 5)
	{            // too small
		draw_box_colored(0, 0, w, h, 0, 0);
	}
	else
	{
// background box
		draw_box_colored(0, 0, w, h, 1, 0);

// backward arrow
		if(backarrow)
			draw_triangle_left(2, 2, h - 2, h - 4, 
				get_resources()->button_shadow, 
				BLACK, 
				get_resources()->button_down, 
				get_resources()->button_down, 
				get_resources()->button_light);
		else
		if(backhi)
			draw_triangle_left(2, 2, h - 2, h - 4, 
				get_resources()->button_light, 
				get_resources()->button_highlighted, 
				get_resources()->button_highlighted, 
				get_resources()->button_down, 
				BLACK);
		else
			draw_triangle_left(2, 2, h - 2, h - 4, 
				get_resources()->button_light, 
				get_resources()->button_up, 
				get_resources()->button_up, 
				get_resources()->button_down, 
				BLACK);

// forward arrow
		if(forwardarrow)
			draw_triangle_right(w - h, 2, h - 2, h - 4, 
				get_resources()->button_shadow, 
				BLACK, 
				get_resources()->button_down, 
				get_resources()->button_down, 
				get_resources()->button_light);
		else
		if(forwardhi)
			draw_triangle_right(w - h, 2, h - 2, h - 4, 
				get_resources()->button_light, 
				get_resources()->button_highlighted, 
				get_resources()->button_highlighted, 
				get_resources()->button_down, 
				BLACK);
		else
			draw_triangle_right(w - h, 2, h - 2, h - 4, 
				get_resources()->button_light, 
				get_resources()->button_up, 
				get_resources()->button_up, 
				get_resources()->button_down, 
				BLACK);


		get_positions(w - h * 2);
		draw_box_colored(handlex + h, 2, handlew, h - 4, box, boxhi && !box);
	}
	
	flash();
return 0;
}

//================================ y scrollbar=======================
BC_YScrollBar::BC_YScrollBar(int x_, int y_, int w_, int h_, long length_, long position_, long handlelength_)
	: BC_ScrollBar(x_, y_, w_, h_, length_, position_, handlelength_)
{
}

int BC_YScrollBar::test_highlight()
{
	int result;
	result = 0;
	if(!enabled) return 0;

	if(get_cursor_x() > 0 && get_cursor_x() < w)
	{
		if(get_cursor_y() > handlex + w && get_cursor_y() < handlex + w + handlew)
		{
			if(!boxhi)
			{
				top_level->unhighlight();
				boxhi = 1;
				draw();
			}
			result = 1;
		}
		else 
		if(get_cursor_y() > 0 && get_cursor_y() < w)
		{
			if(!backhi)
			{
				top_level->unhighlight();
				backhi = 1;
				draw();
			}
			result = 1;
		}
		else 
		if(get_cursor_y() > h - w && get_cursor_y() < h)
		{
			if(!forwardhi)
			{
				top_level->unhighlight();
				forwardhi = 1;
				draw();
			}
			result = 1;
		}
	}

	if(!result)
	{
		if(boxhi)
		{
			if(get_cursor_x() < 0 || get_cursor_x() > w ||
				 get_cursor_y() < handlex + w || get_cursor_y() > handlex + w + handlew)
			{
				boxhi = 0;
				draw();
			}
		}
		else 
		if(backhi)
		{
			if(get_cursor_x() < 0 || get_cursor_x() > w ||
				 get_cursor_y() < 0 || get_cursor_y() > w)
			{
				backhi = 0;
				draw();
			}
		}
		else if(forwardhi)
		{
			if(get_cursor_x() < 0 || get_cursor_x() > w ||
				 get_cursor_y() < h - w || get_cursor_y() > h)
			{
				forwardhi = 0;
				draw();
			}
		}
	}

	return result;
return 0;
}

int BC_YScrollBar::cursor_left_()
{
// highlighting
	if(!buttondown && !backarrow && !forwardarrow && !box && !backpage && !forwardpage)
	{
		if(cursor_x < 0 || cursor_x > w || cursor_y < 0 || cursor_y > h)
		test_highlight();
	}
return 0;
}

int BC_YScrollBar::button_press_()
{
	int result = 0;
	next_repeat = sustain_repeat;
	if(!get_repeat()) 
	{
		if(cursor_y > 0 && cursor_y < h && cursor_x > 0 && cursor_x < w)
		{
			buttondown = 1;
    		if(cursor_y < w)
			{ 
				set_repeat(next_repeat); 
				backarrow = 1; 
				draw(); 
			}
    		else
    		if(cursor_y > h - w)
			{ 
				set_repeat(next_repeat); 
				forwardarrow = 1; 
				draw(); 
			}
    		else
    		if(cursor_y > w && cursor_y < h - w) // between pointers
    		{
      			get_positions(h - w * 2);

      			if(cursor_y < handlex + w) 
				{ 
					set_repeat(next_repeat); 
					backpage = 1; 
				}
      			else
      			if(cursor_y > handlex + w + handlew) 
				{ 
					set_repeat(next_repeat); 
					forwardpage = 1; 
				}
      			else
      			{
        			oldy = cursor_y;
        			relativey = (int)(position / ((float)length / (h - w * 2)));
        			box = 1;
        			draw();
      			}
    		}
			result = 1;
  		}
	}
// handle first repeat
	if(handle_arrows(0)) handle_event();
	return result;
return 0;
}

int BC_YScrollBar::cursor_motion_()
{
// highlighting
	int result;
	result = 0;
	distance = 0;
	if(!buttondown && !backarrow && !forwardarrow && !box && !backpage && !forwardpage)
	{
		result = test_highlight();
	}

	if(box)
	{
		result = 1;
		oldposition = position;

		relativey += cursor_y - oldy;
		oldy = cursor_y;

		position = (long)((float)length / (h - w * 2) * relativey);

		if(position < 0) position = 0;
		if(position > length - handlelength) position = length - handlelength;
		if(position < 0) position = 0;
		if(oldposition != position)
		{ 
			draw(); 
			distance = position - oldposition; 
			handle_event();
		}
	}
	return result;
return 0;
}



int BC_YScrollBar::draw()
{
 	if(h < w * 2 + 5)
	{            // too small
		draw_box_colored(0, 0, w, h, 0, 0);
		//draw_3d_big(0, 0, w, h, LTGREY, MEGREY, DKGREY);
	}
	else
	{
 		int x1, y1, x2, y2, x3, y3;
  		XPoint point[3];

// background box
		draw_box_colored(0, 0, w, h, 1, 0);

// draw back arrow
		if(backarrow)
			draw_triangle_up(2, 2, w-4, w-2, 
				get_resources()->button_shadow, 
				BLACK, 
				get_resources()->button_down, 
				get_resources()->button_down, 
				get_resources()->button_light);
		else
		if(backhi)
			draw_triangle_up(2, 2, w-4, w-2, 
				get_resources()->button_light, 
				get_resources()->button_highlighted, 
				get_resources()->button_highlighted, 
				get_resources()->button_down, 
				BLACK);
		else
			draw_triangle_up(2, 2, w-4, w-2, 
				get_resources()->button_light, 
				get_resources()->button_up, 
				get_resources()->button_up, 
				get_resources()->button_down, 
				BLACK);

// draw forward arrow
 		if(forwardarrow)
			draw_triangle_down(2, h-w, w-4, w-2, 
				get_resources()->button_shadow, 
				BLACK, 
				get_resources()->button_down, 
				get_resources()->button_down, 
				get_resources()->button_light);
		else
		if(forwardhi)
			draw_triangle_down(2, h-w, w-4, w-2, 
				get_resources()->button_light, 
				get_resources()->button_highlighted, 
				get_resources()->button_highlighted, 
				get_resources()->button_down, 
				BLACK);
		else
			draw_triangle_down(2, h-w, w-4, w-2, 
				get_resources()->button_light, 
				get_resources()->button_up, 
				get_resources()->button_up, 
				get_resources()->button_down, 
				BLACK);

// handle
		get_positions(h - w * 2);
		draw_box_colored(2, handlex + w, w - 4, handlew, box, boxhi && !box);
	}
	
  flash();
return 0;
}
