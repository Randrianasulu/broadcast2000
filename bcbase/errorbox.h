#ifndef ERRORBOX_H
#define ERRORBOX_H

#include "bcbase.h"

class ErrorBoxOkButton;


class ErrorBox : public BC_Window
{
public:
	ErrorBox(const char *display = "", const char *title = "2000: Error");
	virtual ~ErrorBox();

	int create_objects(char *text1, char *text2 = 0, char *text3 = 0, char *text4 = 0);
	ErrorBoxOkButton *ok;
};

class ErrorBoxOkButton : public BC_BigButton
{
public:
	ErrorBoxOkButton(ErrorBox *errorbox);

	int handle_event();
	int keypress_event();

	ErrorBox *errorbox;
};

#endif

