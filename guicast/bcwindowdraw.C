#include "bcbitmap.h"
#include "bcpixmap.h"
#include "bcpopup.h"
#include "bcresources.h"
#include "bcwindowbase.h"
#include "colors.h"
#include "fonts.h"
#include "vframe.h"
#include <string.h>

void BC_WindowBase::draw_box(int x, int y, int w, int h, BC_Pixmap *pixmap)
{
//if(x == 0) printf("BC_WindowBase::draw_box %d %d %d %d\n", x, y, w, h);
	XFillRectangle(top_level->display, 
		pixmap ? pixmap->opaque_pixmap : this->pixmap, 
		top_level->gc, 
		x, 
		y, 
		w, 
		h);
}


void BC_WindowBase::clear_box(int x, int y, int w, int h, BC_Pixmap *pixmap)
{
	set_color(bg_color);
	XFillRectangle(top_level->display, 
		pixmap ? pixmap->opaque_pixmap : this->pixmap, 
		top_level->gc, 
		x, 
		y, 
		w, 
		h);
}

void BC_WindowBase::draw_text(int x, int y, char *text, int length, BC_Pixmap *pixmap)
{
	if(length < 0) length = strlen(text);
	int boldface = top_level->current_font & BOLDFACE;
	int font = top_level->current_font & 0xff;

	switch(font)
	{
		case MEDIUM_7SEGMENT:
			for(int i = 0; i < length; i++)
			{
				VFrame *image;
				switch(text[i])
				{
					case '0':
						image = get_resources()->medium_7segment[0];
						break;
					case '1':
						image = get_resources()->medium_7segment[1];
						break;
					case '2':
						image = get_resources()->medium_7segment[2];
						break;
					case '3':
						image = get_resources()->medium_7segment[3];
						break;
					case '4':
						image = get_resources()->medium_7segment[4];
						break;
					case '5':
						image = get_resources()->medium_7segment[5];
						break;
					case '6':
						image = get_resources()->medium_7segment[6];
						break;
					case '7':
						image = get_resources()->medium_7segment[7];
						break;
					case '8':
						image = get_resources()->medium_7segment[8];
						break;
					case '9':
						image = get_resources()->medium_7segment[9];
						break;
					case ':':
						image = get_resources()->medium_7segment[10];
						break;
					default:
						image = get_resources()->medium_7segment[11];
						break;
				}

				draw_vframe(image, 
					x, 
					y - image->get_h());
				x += image->get_w();
			}
			break;

		default:
		{
			int color = get_color();
			if(boldface) set_color(BLACK);

			for(int k = (boldface ? 1 : 0); k >= 0; k--)
			{
				for(int i = 0, j = 0, x2 = x, y2 = y; 
					i <= length; 
					i++)
				{
					if(text[i] == '\n' || text[i] == 0)
					{
						if(get_resources()->use_fontset && top_level->get_curr_fontset())
        					XmbDrawString(top_level->display, 
                				pixmap ? pixmap->opaque_pixmap : this->pixmap, 
                				top_level->get_curr_fontset(),
                				top_level->gc, 
                				x2 + k, 
                				y2 + k, 
                				&text[j], 
                				i - j);
						else
							XDrawString(top_level->display, 
								pixmap ? pixmap->opaque_pixmap : this->pixmap, 
								top_level->gc, 
								x2 + k, 
								y2 + k, 
								&text[j], 
								i - j);
						j = i + 1;
						y2 += get_text_height(MEDIUMFONT);
					}
				}
				if(boldface) set_color(color);
			}
		}
			break;
	}
}

void BC_WindowBase::draw_center_text(int x, int y, char *text, int length)
{
	if(length < 0) length = strlen(text);
	int w = get_text_width(current_font, text, length);
	x -= w / 2;
	draw_text(x, y, text, length);
}

void BC_WindowBase::draw_line(int x1, int y1, int x2, int y2, BC_Pixmap *pixmap)
{
	XDrawLine(top_level->display, pixmap ? pixmap->opaque_pixmap : this->pixmap, top_level->gc, x1, y1, x2, y2);
}

void BC_WindowBase::draw_rectangle(int x, int y, int w, int h)
{
	XDrawRectangle(top_level->display, pixmap, top_level->gc, x, y, w - 1, h - 1);
}

void BC_WindowBase::draw_3d_border(int x, int y, int w, int h, 
	int light1, int light2, int shadow1, int shadow2)
{
	int lx, ly, ux, uy;

	h--; w--;

	lx = x+1;  ly = y+1;
	ux = x+w-1;  uy = y+h-1;

	set_color(light1);
	draw_line(x, y, ux, y);
	draw_line(x, y, x, uy);
	set_color(light2);
	draw_line(lx, ly, ux - 1, ly);
	draw_line(lx, ly, lx, uy - 1);

	set_color(shadow1);
	draw_line(ux, ly, ux, uy);
	draw_line(lx, uy, ux, uy);
	set_color(shadow2);
	draw_line(x + w, y, x + w, y + h);
	draw_line(x, y + h, x + w, y + h);
}

void BC_WindowBase::draw_3d_box(int x, 
	int y, 
	int w, 
	int h, 
	int light1, 
	int light2, 
	int middle, 
	int shadow1, 
	int shadow2,
	BC_Pixmap *pixmap)
{
	int lx, ly, ux, uy;

	h--; w--;

	lx = x+1;  ly = y+1;
	ux = x+w-1;  uy = y+h-1;

	set_color(middle);
	draw_box(x, y, w, h, pixmap);

	set_color(light1);
	draw_line(x, y, ux, y, pixmap);
	draw_line(x, y, x, uy, pixmap);
	set_color(light2);
	draw_line(lx, ly, ux - 1, ly, pixmap);
	draw_line(lx, ly, lx, uy - 1, pixmap);

	set_color(shadow1);
	draw_line(ux, ly, ux, uy, pixmap);
	draw_line(lx, uy, ux, uy, pixmap);
	set_color(shadow2);
	draw_line(x + w, y, x + w, y + h, pixmap);
	draw_line(x, y + h, x + w, y + h, pixmap);
}

void BC_WindowBase::draw_colored_box(int x, int y, int w, int h, int down, int highlighted)
{
	if(!down)
	{
		if(highlighted)
			draw_3d_box(x, y, w, h, 
				top_level->get_resources()->button_light, 
				top_level->get_resources()->button_highlighted, 
				top_level->get_resources()->button_highlighted, 
				top_level->get_resources()->button_shadow,
				BLACK);
		else
			draw_3d_box(x, y, w, h, 
				top_level->get_resources()->button_light, 
				top_level->get_resources()->button_up, 
				top_level->get_resources()->button_up, 
				top_level->get_resources()->button_shadow,
				BLACK);
	}
	else
	{
// need highlighting for toggles
		if(highlighted)
			draw_3d_box(x, y, w, h, 
				top_level->get_resources()->button_shadow, 
				BLACK, 
				top_level->get_resources()->button_up, 
				top_level->get_resources()->button_up,
				top_level->get_resources()->button_light);
		else
			draw_3d_box(x, y, w, h, 
				top_level->get_resources()->button_shadow, 
				BLACK, 
				top_level->get_resources()->button_down, 
				top_level->get_resources()->button_down,
				top_level->get_resources()->button_light);
	}
}

void BC_WindowBase::draw_border(char *text, int x, int y, int w, int h)
{
	int left_indent = 20;
	int lx, ly, ux, uy;

	h--; w--;
	lx = x + 1;  ly = y + 1;
	ux = x + w - 1;  uy = y + h - 1;

	set_opaque();
	if(text && text[0] != 0)
	{
		set_color(BLACK);
		set_font(MEDIUMFONT);
		draw_text(x + left_indent, y + get_text_height(MEDIUMFONT) / 2, text);
	}
	
	set_color(top_level->get_resources()->button_shadow);
	draw_line(x, y, x + left_indent - 5, y);
	draw_line(x, y, x, uy);
	draw_line(x + left_indent + 5 + get_text_width(MEDIUMFONT, text), y, ux, y);
	draw_line(x, y, x, uy);
	draw_line(ux, ly, ux, uy);
	draw_line(lx, uy, ux, uy);
	set_color(top_level->get_resources()->button_light);
	draw_line(lx, ly, x + left_indent - 5 - 1, ly);
	draw_line(lx, ly, lx, uy - 1);
	draw_line(x + left_indent + 5 + get_text_width(MEDIUMFONT, text), ly, ux - 1, ly);
	draw_line(lx, ly, lx, uy - 1);
	draw_line(x + w, y, x + w, y + h);
	draw_line(x, y + h, x + w, y + h);
}

void BC_WindowBase::draw_triangle_down_flat(int x, int y, int w, int h)
{
	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	x1 = x; x2 = x + w / 2; x3 = x + w - 1;
	y1 = y; y2 = y + h - 1;

	point[0].x = x2; point[0].y = y2; point[1].x = x3;
	point[1].y = y1; point[2].x = x1; point[2].y = y1;

	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);
}

void BC_WindowBase::draw_triangle_up(int x, int y, int w, int h, 
	int light1, int light2, int middle, int shadow1, int shadow2)
{
	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	x1 = x; y1 = y; x2 = x + w / 2;
	y2 = y + h - 1; x3 = x + w - 1;

// middle
	point[0].x = x2; point[0].y = y1; point[1].x = x3;
	point[1].y = y2; point[2].x = x1; point[2].y = y2;

	set_color(middle);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// bottom and top right
	set_color(shadow1);
	draw_line(x3, y2-1, x1, y2-1);
	draw_line(x2-1, y1, x3-1, y2);
	set_color(shadow2);
	draw_line(x3, y2, x1, y2);
	draw_line(x2, y1, x3, y2);

// top left
	set_color(light2);
	draw_line(x2+1, y1, x1+1, y2);
	set_color(light1);
	draw_line(x2, y1, x1, y2);
}

void BC_WindowBase::draw_triangle_down(int x, int y, int w, int h, 
	int light1, int light2, int middle, int shadow1, int shadow2)
{
	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	x1 = x; x2 = x + w / 2; x3 = x + w - 1;
	y1 = y; y2 = y + h - 1;

	point[0].x = x2; point[0].y = y2; point[1].x = x3;
	point[1].y = y1; point[2].x = x1; point[2].y = y1;

	set_color(middle);
	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// top and bottom left
	set_color(light2);
	draw_line(x3-1, y1+1, x1+1, y1+1);
	draw_line(x1+1, y1, x2+1, y2);
	set_color(light1);
	draw_line(x3, y1, x1, y1);
	draw_line(x1, y1, x2, y2);

// bottom right
	set_color(shadow1);
  	draw_line(x3-1, y1, x2-1, y2);
	set_color(shadow2);
	draw_line(x3, y1, x2, y2);
}

void BC_WindowBase::draw_triangle_left(int x, int y, int w, int h, 
	int light1, int light2, int middle, int shadow1, int shadow2)
{
  	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	// draw back arrow
  	y1 = y; x1 = x; y2 = y + h / 2;
  	x2 = x + w - 1; y3 = y + h - 1;

	point[0].x = x1; point[0].y = y2; point[1].x = x2; 
	point[1].y = y1; point[2].x = x2; point[2].y = y3;

	set_color(middle);
  	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// right and bottom right
	set_color(shadow1);
  	draw_line(x2-1, y1, x2-1, y3-1);
  	draw_line(x2, y3-1, x1, y2-1);
	set_color(shadow2);
  	draw_line(x2, y1, x2, y3);
  	draw_line(x2, y3, x1, y2);

// top left
	set_color(light1);
	draw_line(x1, y2, x2, y1);
	set_color(light2);
	draw_line(x1, y2+1, x2, y1+1);
}

void BC_WindowBase::draw_triangle_right(int x, int y, int w, int h, 
	int light1, int light2, int middle, int shadow1, int shadow2)
{
  	int x1, y1, x2, y2, x3, y3;
	XPoint point[3];

	y1 = y; y2 = y + h / 2; y3 = y + h - 1; 
	x1 = x; x2 = x + w - 1;

	point[0].x = x1; point[0].y = y1; point[1].x = x2; 
	point[1].y = y2; point[2].x = x1; point[2].y = y3;

	set_color(middle);
  	XFillPolygon(top_level->display, pixmap, top_level->gc, (XPoint *)point, 3, Nonconvex, CoordModeOrigin);

// left and top right
	set_color(light2);
	draw_line(x1+1, y3, x1+1, y1);
	draw_line(x1, y1+1, x2, y2+1);
	set_color(light1);
	draw_line(x1, y3, x1, y1);
	draw_line(x1, y1, x2, y2);

// bottom right
	set_color(shadow1);
  	draw_line(x2, y2-1, x1, y3-1);
	set_color(shadow2);
  	draw_line(x2, y2, x1, y3);
}


void BC_WindowBase::draw_check(int x, int y)
{
	const int w = 15, h = 15;
	draw_line(x + 3, y + h / 2 + 0, x + 6, y + h / 2 + 2);
	draw_line(x + 3, y + h / 2 + 1, x + 6, y + h / 2 + 3);
	draw_line(x + 6, y + h / 2 + 2, x + w - 4, y + h / 2 - 3);
	draw_line(x + 3, y + h / 2 + 2, x + 6, y + h / 2 + 4);
	draw_line(x + 6, y + h / 2 + 2, x + w - 4, y + h / 2 - 3);
	draw_line(x + 6, y + h / 2 + 3, x + w - 4, y + h / 2 - 2);
	draw_line(x + 6, y + h / 2 + 4, x + w - 4, y + h / 2 - 1);
}

void BC_WindowBase::draw_tiles(BC_Pixmap *tile, int origin_x, int origin_y, int x, int y, int w, int h)
{
	if(!tile)
	{
		set_color(bg_color);
		draw_box(x, y, w, h);
	}
	else
	{
		XSetFillStyle(top_level->display, top_level->gc, FillTiled);
// Don't know how slow this is
		XSetTile(top_level->display, top_level->gc, tile->get_pixmap());
		XSetTSOrigin(top_level->display, top_level->gc, origin_x, origin_y);
		draw_box(x, y, w, h);
		XSetFillStyle(top_level->display, top_level->gc, FillSolid);
	}
}

void BC_WindowBase::draw_top_tiles(BC_WindowBase *parent_window, int x, int y, int w, int h)
{
	Window tempwin;
	int origin_x, origin_y;
	XTranslateCoordinates(top_level->display, 
			parent_window->win, 
			win, 
			0, 
			0, 
			&origin_x, 
			&origin_y, 
			&tempwin);

	draw_tiles(parent_window->bg_pixmap, 
		origin_x,
		origin_y,
		x,
		y,
		w,
		h);
}

void BC_WindowBase::draw_top_background(BC_WindowBase *parent_window, int x, int y, int w, int h, BC_Pixmap *pixmap)
{
	Window tempwin;
	int top_x, top_y;
	XTranslateCoordinates(top_level->display, 
			win, 
			parent_window->win, 
			x, 
			y, 
			&top_x, 
			&top_y, 
			&tempwin);

	XCopyArea(top_level->display, 
		parent_window->pixmap, 
		pixmap ? pixmap->opaque_pixmap : this->pixmap, 
		top_level->gc, 
		top_x, 
		top_y, 
		w, 
		h, 
		x, 
		y);
}

void BC_WindowBase::draw_background(int x, int y, int w, int h)
{
	draw_tiles(bg_pixmap, 0, 0, x, y, w, h);
}

void BC_WindowBase::draw_bitmap(BC_Bitmap *bitmap, 
	int dont_wait,
	int dest_x, 
	int dest_y,
	int dest_w,
	int dest_h,
	int src_x,
	int src_y,
	int src_w,
	int src_h,
	BC_Pixmap *pixmap)
{
	if(dest_w <= 0 || dest_h <= 0)
	{
// Use hardware scaling to canvas dimensions if proper color model.
		if(bitmap->get_color_model() == BC_YUV420P)
		{
			dest_w = w;
			dest_h = h;
		}
		else
		{
			dest_w = bitmap->get_w();
			dest_h = bitmap->get_h();
		}
	}

	if(src_w <= 0 || src_h <= 0)
	{
		src_w = bitmap->get_w();
		src_h = bitmap->get_h();
	}

	if(video_on)
	{
		bitmap->write_drawable(win, 
			top_level->gc, 
			src_x, 
			src_y, 
			src_w,
			src_h,
			dest_x, 
			dest_y, 
			dest_w, 
			dest_h, 
			dont_wait);
		top_level->flush();
	}
	else
	{
		bitmap->write_drawable(pixmap ? pixmap->opaque_pixmap : this->pixmap, 
			top_level->gc, 
			dest_x, 
			dest_y, 
			src_x, 
			src_y, 
			dest_w, 
			dest_h, 
			dont_wait);
	}
}

void BC_WindowBase::draw_pixmap(BC_Pixmap *pixmap, 
	int dest_x, 
	int dest_y,
	int dest_w,
	int dest_h,
	int src_x,
	int src_y)
{
	pixmap->write_drawable(this->pixmap,
			dest_x, 
			dest_y,
			dest_w,
			dest_h,
			src_x,
			src_y);
}

void BC_WindowBase::draw_vframe(VFrame *frame, 
		int dest_x, 
		int dest_y, 
		int dest_w, 
		int dest_h,
		int src_x,
		int src_y,
		BC_Pixmap *pixmap)
{
	if(dest_w < 0) dest_w = frame->get_w();
	if(dest_h < 0) dest_h = frame->get_h();
	if(!temp_bitmap) temp_bitmap = new BC_Bitmap(top_level, dest_w, dest_h, get_color_model(), 0);
	temp_bitmap->match_params(dest_w, dest_h, get_color_model(), 0);
	temp_bitmap->read_frame(frame, 0, 
		0, 0, frame->get_w(), frame->get_h(),
		0, 0, dest_w, dest_h);
	draw_bitmap(temp_bitmap, 
		0, 
		dest_x, 
		dest_y,
		dest_w,
		dest_h,
		src_x,
		src_y,
		-1,
		-1,
		pixmap);
}

void BC_WindowBase::draw_tooltip()
{
	if(tooltip_popup)
	{
		int w = tooltip_popup->get_w(), h = tooltip_popup->get_h();
		tooltip_popup->set_color(get_resources()->tooltip_bg_color);
		tooltip_popup->draw_box(0, 0, w, h);
		tooltip_popup->set_color(BLACK);
		tooltip_popup->draw_rectangle(0, 0, w, h);
		tooltip_popup->set_font(MEDIUMFONT);
		tooltip_popup->draw_text(TOOLTIP_MARGIN, 
			get_text_ascent(MEDIUMFONT) + TOOLTIP_MARGIN, 
			tooltip_text);
	}
}

void BC_WindowBase::slide_left(int distance)
{
	if(distance < w)
	{
		XCopyArea(top_level->display, pixmap, pixmap, top_level->gc, distance, 0, w - distance, h, 0, 0);
	}
}

void BC_WindowBase::slide_right(int distance)
{
	if(distance < w)
	{
		XCopyArea(top_level->display, pixmap, pixmap, top_level->gc, 0, 0, w - distance, h, distance, 0);
	}
}

void BC_WindowBase::slide_up(int distance)
{
	if(distance < h)
	{
		XCopyArea(top_level->display, pixmap, pixmap, top_level->gc, 0, distance, w, h - distance, 0, 0);
		set_color(bg_color);
		XFillRectangle(top_level->display, pixmap, top_level->gc, 0, h - distance, w, distance);
	}
}

void BC_WindowBase::slide_down(int distance)
{
	if(distance < h)
	{
		XCopyArea(top_level->display, pixmap, pixmap, top_level->gc, 0, 0, w, h - distance, 0, distance);
		set_color(bg_color);
		XFillRectangle(top_level->display, pixmap, top_level->gc, 0, 0, w, distance);
	}
}

void BC_WindowBase::draw_3segment(int x, 
	int y, 
	int w, 
	int h, 
	BC_Pixmap *left_image,
	BC_Pixmap *mid_image,
	BC_Pixmap *right_image,
	BC_Pixmap *pixmap)
{
	int left_boundary = left_image->get_w_fixed();
	int right_boundary = w - right_image->get_w_fixed();

	for(int i = 0; i < w; )
	{
		BC_Pixmap *image;

		if(i < left_boundary)
			image = left_image;
		else
		if(i < right_boundary)
			image = mid_image;
		else
			image = right_image;
		
		int output_w = image->get_w_fixed();

		if(i < left_boundary)
		{
			if(i + output_w > left_boundary) output_w = left_boundary - i;
		}
		else
		if(i < right_boundary)
		{
			if(i + output_w > right_boundary) output_w = right_boundary - i;
		}
		else
			if(i + output_w > w) output_w = w - i;

		image->write_drawable(pixmap ? pixmap->opaque_pixmap : this->pixmap, 
				x + i, 
				y,
				output_w,
				h,
				0,
				0);

		i += output_w;
	}
}

void BC_WindowBase::draw_3segment(int x, 
	int y, 
	int w, 
	int h, 
	VFrame *left_image,
	VFrame *mid_image,
	VFrame *right_image,
	BC_Pixmap *pixmap)
{
	int left_boundary = left_image->get_w_fixed();
	int right_boundary = w - right_image->get_w_fixed();

	for(int i = 0; i < w; )
	{
		VFrame *image;

		if(i < left_boundary)
			image = left_image;
		else
		if(i < right_boundary)
			image = mid_image;
		else
			image = right_image;
		
		int output_w = image->get_w_fixed();

		if(i < left_boundary)
		{
			if(i + output_w > left_boundary) output_w = left_boundary - i;
		}
		else
		if(i < right_boundary)
		{
			if(i + output_w > right_boundary) output_w = right_boundary - i;
		}
		else
			if(i + output_w > w) output_w = w - i;

		if(image)
			draw_vframe(image, 
					x + i, 
					y,
					output_w,
					h,
					0,
					0,
					pixmap);

		if(output_w == 0) break;
		i += output_w;
	}
}


