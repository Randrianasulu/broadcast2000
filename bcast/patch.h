#ifndef PATCH_H
#define PATCH_H

class PlayPatch;
class RecordPatch;
class TitlePatch;
class AutoPatch;
class DrawPatch;

#include "bcbase.h"
#include "filehtal.inc"
#include "mainwindow.inc"
#include "module.inc"
#include "patch.inc"
#include "patchbay.inc"


// coordinates for widgets

#define PATCH_ROW2 23
#define PATCH_TITLE 3
#define PATCH_PLAY 7
#define PATCH_REC 27
#define PATCH_AUTO 47
#define PATCH_AUTO_TITLE 67
#define PATCH_DRAW 72
#define PATCH_DRAW_TITLE 97


class Patch : public ListItem<Patch>
{
public:
	Patch() { };
	Patch(MainWindow *mwindow, PatchBay *patchbay, int data_type);
	~Patch();

	int save(FileHTAL *htal);
	int load(FileHTAL *htal);

	int create_objects(char *text, int pixel); // linked list doesn't allow parameters in append()
	int set_pixel(int pixel);
	int set_title(char *new_title);
	int flip_vertical();
	int pixelmovement(int distance);

	int record;
	int play;
	int automate;
	int draw;
	char title[1024];

	PatchBay *patches;
	MainWindow *mwindow;

	RecordPatch *recordpatch;
	PlayPatch *playpatch;
	TitlePatch *title_text;
	AutoPatch *autopatch;
	DrawPatch *drawpatch;
	//BC_Title *autotitle, *drawtitle;
	Module* get_module();    // return module of corresponding patch
	int pixel;      // distance from top of track window
	int data_type;
};

class PlayPatch : public BC_PlayPatch
{
public:
	PlayPatch(Patch *patch, int x, int y);
	int handle_event();
	int cursor_moved_over();
	int button_release();
	PatchBay *patches;
	Patch *patch;
};

class RecordPatch : public BC_RecordPatch
{
public:
	RecordPatch(Patch *patch, int x, int y);
	int handle_event();
	int cursor_moved_over();
	int button_release();
	Patch *patch;
	PatchBay *patches;
};

class TitlePatch : public BC_TextBox
{
public:
	TitlePatch(Patch *patch, char *text, int x, int y);
	int handle_event();
	Patch *patch;
	PatchBay *patches;
	Module *module;
};

class AutoPatch : public BC_CheckBox
{
public:
	AutoPatch(Patch *patch, int x, int y);
	int handle_event();
	int cursor_moved_over();
	int button_release();
	PatchBay *patches;
	Patch *patch;
};

class DrawPatch : public BC_CheckBox
{
public:
	DrawPatch(Patch *patch, int x, int y);
	int handle_event();
	int cursor_moved_over();
	PatchBay *patches;
	Patch *patch;
};

#endif
