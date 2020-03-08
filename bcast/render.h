#ifndef RENDER_H
#define RENDER_H


#include "bcbase.h"
#include "bitspopup.h"
#include "browsebutton.h"
#include "compresspopup.h"
#include "defaults.inc"
#include "edit.inc"
#include "errorbox.inc"
#include "file.inc"
#include "formatpopup.inc"
#include "formattools.inc"
#include "mainwindow.inc"
#include "maxchannels.h"
#include "playabletracks.inc"
#include "progressbox.inc"
#include "render.inc"
#include "track.inc"
#include "vframe.inc"

class Render : public BC_MenuItem, public Thread
{
public:
	Render(Defaults *defaults, MainWindow *mwindow, int list_mode);
	~Render();
	
	int handle_event();
	void run();
	int load_defaults(Asset *asset);
	int save_defaults(Asset *asset);
// force asset parameters regardless of window
// This should be integrated into the Asset Class.
	int check_asset(Asset &asset); 
// Force filename to have a 0 padded number if rendering to a list.
	int check_numbering(Asset &asset);
	int inject_number(Asset &asset, int current_number);
	int direct_frame_copy(long &render_video_position, File *file);
	int direct_copy_possible(long current_position, 
			Track* playable_track,  // The one track which is playable
			Edit* &playable_edit, // The edit which is playing
			File *file);   // Output file

	int in_progress;
	int to_tracks;
	int do_audio;
	int do_video;
	int dither;
// Background compression must be disabled when direct frame copying and reenabled afterwards
	int direct_frame_copying;
	int list_mode;          // Whether or not this render instance is for rendering a list of segments
	int number_start;       // Character in the filename path at which the number begins
	int total_digits;       // Total number of digits including padding the user specified.
	int current_number;    // The number the being injected into the filename.
	int total_files;       // Total number of 

	VFrame *compressed_frame;
	MainWindow *mwindow;
	Defaults *defaults;
	PlayableTracks *playable_tracks;
};

class RenderOK;
class RenderCancel;
class RenderToTracks;

class RenderWindow : public BC_Window
{
public:
	RenderWindow(Render *render, Asset *asset);
	~RenderWindow();

	int create_objects();


	RenderOK *ok_button;
	RenderCancel *cancel_button;
	RenderToTracks *to_tracks_button;
	FormatTools *format_tools;

	Render *render;
	Asset *asset;
};

class RenderOK : public BC_BigButton
{
public:
	RenderOK(int x, int y);
	~RenderOK();
	int handle_event();
	int keypress_event();
};

class RenderCancel : public BC_BigButton
{
public:
	RenderCancel(int x, int y);
	~RenderCancel();
	int handle_event();
	int keypress_event();
};

class RenderToTracks : public BC_CheckBox
{
public:
	RenderToTracks(int x, int y, Render *render, int default_);
	~RenderToTracks();

	int handle_event();
	Render *render;
};

#endif
