#ifndef QUESTION_H
#define QUESTION_H

#include "bcbase.h"

class QuestionYesButton;
class QuestionNoButton;
class QuestionCancelButton;

class QuestionWindow : public BC_Window
{
public:
	QuestionWindow(char *display);
	~QuestionWindow();

	int create_objects(char *string, char *string1, int use_cancel);
};

class QuestionYesButton : public BC_BigButton
{
public:
	QuestionYesButton(QuestionWindow *window);

	int handle_event();
	int keypress_event();

	QuestionWindow *window;
};

class QuestionNoButton : public BC_BigButton
{
public:
	QuestionNoButton(QuestionWindow *window);

	int handle_event();
	int keypress_event();

	QuestionWindow *window;
};

class QuestionCancelButton : public BC_BigButton
{
public:
	QuestionCancelButton(QuestionWindow *window);

	int handle_event();
	int keypress_event();

	QuestionWindow *window;
};

#endif
