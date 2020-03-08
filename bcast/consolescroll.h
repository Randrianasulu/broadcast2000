#ifndef CXSCROLL_H
#define CXSCROLL_H

class CXScroll;
class CYScroll;

#include "bcbase.h"
#include "console.inc"
#include "consolescroll.inc"

class ConsoleMainScroll
{
public:
	ConsoleMainScroll(ConsoleWindow *gui);
	~ConsoleMainScroll();

	int create_objects(int w, int h);
	int handle_event(long position);
	int resize_event(int w, int h);
	int flip_vertical(int w, int h);
	int update();               // reflect new console view

private:
	long oldposition;
	ConsoleWindow *gui;
	CXScroll *cxscroll;
	CYScroll *cyscroll;
};

class CXScroll : public BC_XScrollBar
{
public:
	CXScroll(ConsoleMainScroll *scroll, int w, int h);
	~CXScroll();

	int handle_event();

private:
	ConsoleMainScroll *scroll;
};

class CYScroll : public BC_YScrollBar
{
public:
	CYScroll(ConsoleMainScroll *scroll, int w, int h);
	~CYScroll();

	int handle_event();

private:
	ConsoleMainScroll *scroll;
};

#endif
