#ifndef FRATEWINDOW_H
#define FRATEWINDOW_H

#include "bcbase.h"

class FRateWindowOK;
class FRateWindowCancel;
class FRateWindowRate;

class FRateWindow : public BC_Window
{
public:
	FRateWindow();
	~FRateWindow();
	
	int create_objects(float *output_rate);
	
	float *output_rate;
	FRateWindowOK *ok;
	FRateWindowCancel *cancel;
	FRateWindowRate *rate_text;
};

class FRateWindowRate : public BC_TextBox
{
public:
	FRateWindowRate(float *output_rate);
	~FRateWindowRate();
	
	int handle_event();
	float *output_rate;
};

class FRateWindowOK : public BC_BigButton
{
public:
	FRateWindowOK(FRateWindow *window);
	
	int handle_event();
	int keypress_event();
	
	FRateWindow *window;
};

class FRateWindowCancel : public BC_BigButton
{
public:
	FRateWindowCancel(FRateWindow *window);
	
	int handle_event();
	int keypress_event();
	
	FRateWindow *window;
};

#endif
