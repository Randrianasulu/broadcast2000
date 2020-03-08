#include "bcclipboard.h"
#include "bcwindowbase.h"
#include <string.h>

BC_Clipboard::BC_Clipboard(char *display_name) : Thread()
{
	XSetWindowAttributes attr;
	unsigned long mask;

	set_synchronous(1);
	display = BC_WindowBase::init_display(display_name);
	completion_atom = XInternAtom(display, "BC_CLOSE_EVENT", False);
	primary_atom = XInternAtom(display, "PRIMARY", False);
	secondary_atom = XInternAtom(display, "SECONDARY", False);
	mask = 0;
	win = XCreateWindow(display, 
				RootWindow(display, DefaultScreen(display)), 
				0, 
				0, 
				10, 
				10, 
				0, 
				DefaultDepth(display, DefaultScreen(display)), 
				InputOutput, 
				DefaultVisual(display, DefaultScreen(display)), 
				mask, 
				&attr);
	data[0] = 0;
	data[1] = 0;
}

BC_Clipboard::~BC_Clipboard()
{
	if(data[0]) delete [] data[0];
	if(data[1]) delete [] data[1];
	XDestroyWindow(display, win);
	XCloseDisplay(display);
}

int BC_Clipboard::start_clipboard()
{
return 0;
	Thread::start();
	return 0;
}

int BC_Clipboard::stop_clipboard()
{
return 0;
	XEvent event;
	XClientMessageEvent *ptr = (XClientMessageEvent*)&event;

	event.type = ClientMessage;
	ptr->message_type = completion_atom;
	ptr->format = 32;
	XSendEvent(display, win, 0, 0, &event);
	XFlush(display);
	Thread::join();
	return 0;
}

void BC_Clipboard::run()
{
	XEvent event;
	XClientMessageEvent *ptr;
	int done = 0;

	while(!done)
	{
		XNextEvent(display, &event);

		switch(event.type)
		{
			case ClientMessage:
				ptr = (XClientMessageEvent*)&event;
				if(ptr->message_type == completion_atom)
				{
					done = 1;
				}
				else
				if(ptr->message_type == primary_atom)
				{
				}
				else
				if(ptr->message_type == secondary_atom)
				{
				}
//printf("ClientMessage %x %x %d\n", ptr->message_type, ptr->data.l[0], primary_atom);
				break;

			case SelectionRequest:
				{
					XEvent new_event;
					XClientMessageEvent *ptr1 = (XClientMessageEvent*)&new_event;
					XSelectionRequestEvent *ptr2 = (XSelectionRequestEvent*)&event;

					new_event.type = ClientMessage;
					ptr->message_type = ptr2->target;
					ptr->format = 32;
					XSendEvent(display, ptr2->requestor, 0, 0, &new_event);
					XFlush(display);
				}
//printf("SelectionRequest\n");
				break;
		}
	}
}

long BC_Clipboard::clipboard_len(int clipboard_num)
{
	char *data2;
	int len;

	data2 = XFetchBuffer(display, &len, clipboard_num);
	XFree(data2);
	return len;
return 0;
	int result = XConvertSelection(display, 
		clipboard_num == PRIMARY_SELECTION ? primary_atom : secondary_atom, 
		clipboard_num == PRIMARY_SELECTION ? primary_atom : secondary_atom, 
		None,
       	win, 
		CurrentTime);

	if(!result)
	{
		return 0;
	}
	return 0;
}

int BC_Clipboard::to_clipboard(char *data, long len, int clipboard_num)
{
	XStoreBuffer(display, data, len, clipboard_num);
return 0;
	if(this->data[clipboard_num] && length[clipboard_num] != len)
	{
		delete [] this->data[clipboard_num];
		this->data[clipboard_num] = 0;
	}

	if(!this->data[clipboard_num])
	{
		length[clipboard_num] = len;
		this->data[clipboard_num] = new char[len];
		strcpy(this->data[clipboard_num], data);
	}

	XSetSelectionOwner(display, 
		clipboard_num == PRIMARY_SELECTION ? primary_atom : secondary_atom, 
		win, 
		CurrentTime);
	return 0;
}

int BC_Clipboard::from_clipboard(char *data, long maxlen, int clipboard_num)
{
	char *data2;
	int len, i;

	data2 = XFetchBuffer(display, &len, clipboard_num);
	for(i = 0; i < len && i < maxlen; i++)
		data[i] = data2[i];

	data[i] = 0;

	XFree(data2);
return 0;
	int result = XConvertSelection(display, 
		clipboard_num == PRIMARY_SELECTION ? primary_atom : secondary_atom, 
		clipboard_num == PRIMARY_SELECTION ? primary_atom : secondary_atom, 
		None,
       	win, 
		CurrentTime);
	if(!result)
	{
		data[0] = 0;
	}
	return 0;
}
