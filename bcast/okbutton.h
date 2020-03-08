#ifndef OKBUTTON_H
#define OKBUTTON_H

#include "bcbase.h"

class OKButton : public BC_BigButton
{
public:
	OKButton(int x, int y);
	~OKButton();
	int handle_event();
	int keypress_event();
};

class CancelButton : public BC_BigButton
{
public:
	CancelButton(int x, int y);
	~CancelButton();
	int handle_event();
	int keypress_event();
};

#endif
