#ifndef ERRORBOX_H
#define ERRORBOX_H

#include "guicast.h"

class ErrorBox : public BC_Window
{
public:
	ErrorBox(char *title, 
		int x = INFINITY, 
		int y = INFINITY, 
		int w = 400, 
		int h = 120);
	virtual ~ErrorBox();

	int create_objects(char *text);
};

#endif

