#ifndef BCCLIPBOARD_H
#define BCCLIPBOARD_H

#include "thread.h"

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xatom.h>
#include <X11/Xlib.h>

#define PRIMARY_SELECTION 0
#define SECONDARY_SELECTION 1

class BC_Clipboard : public Thread
{
public:
	BC_Clipboard(char *display_name);
	~BC_Clipboard();

	int start_clipboard();
	void run();
	int stop_clipboard();
	long clipboard_len(int clipboard_num);
	int to_clipboard(char *data, long len, int clipboard_num);
	int from_clipboard(char *data, long maxlen, int clipboard_num);

	Display *display;
	Atom completion_atom;
	Atom primary_atom, secondary_atom;
	Window win;
	char *data[2];
	long length[2];
};

#endif
