#ifndef SRATEWINDOW_H
#define SRATEWINDOW_H

#include "bcbase.h"

class SRateWindowOK;
class SRateWindowCancel;
class SRateWindowRate;

class SRateWindow : public BC_Window
{
public:
	SRateWindow();
	~SRateWindow();
	
	int create_objects(int *output_rate);
	
	int *output_rate;
	SRateWindowOK *ok;
	SRateWindowCancel *cancel;
	SRateWindowRate *rate_text;
};

class SRateWindowRate : public BC_TextBox
{
public:
	SRateWindowRate(int *output_rate);
	~SRateWindowRate();
	
	int handle_event();
	int *output_rate;
};

class SRateWindowOK : public BC_BigButton
{
public:
	SRateWindowOK(SRateWindow *window);
	
	int handle_event();
	int keypress_event();
	
	SRateWindow *window;
};

class SRateWindowCancel : public BC_BigButton
{
public:
	SRateWindowCancel(SRateWindow *window);
	
	int handle_event();
	int keypress_event();
	
	SRateWindow *window;
};

#endif
