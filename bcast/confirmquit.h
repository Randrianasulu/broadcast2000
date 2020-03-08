#ifndef CONFIRMSAVE_H
#define CONFIRMSAVE_H

#include "bcbase.h"

class ConfirmQuitYesButton;
class ConfirmQuitNoButton;
class ConfirmQuitCancelButton;

class ConfirmQuitWindow : public BC_Window
{
public:
	ConfirmQuitWindow(char *display);
	~ConfirmQuitWindow();

	int create_objects(char *string);
	ConfirmQuitYesButton *yes; 
	ConfirmQuitNoButton *no;
	ConfirmQuitCancelButton *cancel;
};

class ConfirmQuitYesButton : public BC_BigButton
{
public:
	ConfirmQuitYesButton(ConfirmQuitWindow *window);

	int handle_event();
	int keypress_event();

	ConfirmQuitWindow *window;
};

class ConfirmQuitNoButton : public BC_BigButton
{
public:
	ConfirmQuitNoButton(ConfirmQuitWindow *window);

	int handle_event();
	int keypress_event();

	ConfirmQuitWindow *window;
};

class ConfirmQuitCancelButton : public BC_BigButton
{
public:
	ConfirmQuitCancelButton(ConfirmQuitWindow *window);

	int handle_event();
	int keypress_event();

	ConfirmQuitWindow *window;
};

#endif
