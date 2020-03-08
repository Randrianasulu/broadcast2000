#ifndef BCTITLE_H
#define BCTITLE_H

#include "bccolors.h"
#include "bcfont.h"
#include "bctool.h"

class BC_Title : public BC_Tool
{
public:
	BC_Title(int x, int y, 
					 char *text,          // string to draw
					 int font = MEDIUMFONT,        // a font macro
					 int color = BLACK);     // a color macro

	int create_tool_objects();
	int resize(int w, int h);
	int resize_tool(int x, int y);
	int set_color(int color);
	int update(char *text);         // replace the text
	int draw();
	
	char text[256];
	int color;
	int font;
};

#endif
