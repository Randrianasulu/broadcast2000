#ifndef MAINMENU_H
#define MAINMENU_H

class AEffectMenu;
class CameraAutomation;
class FadeAutomation;
class MuteAutomation;
class PanAutomation;
class PanMenu;
class PanItem;
class PlayAutomation;
class PluginAutomation;
class PluginItem;
class PluginMenu;

class ProjectAutomation;
class Redo;
class ShowConsole;
class ShowLevels;
class ShowVideo;
class ShowEdits;
class ShowRenderedOutput;
class ShowTitles;
class Undo;

#include "arraylist.h"
#include "assetmanager.inc"
#include "bcbase.h"
#include "defaults.inc"
#include "loadfile.inc"
#include "mainwindow.inc"
#include "mainwindowgui.inc"
#include "maxchannels.h"
#include "menuaeffects.inc"
#include "menuveffects.inc"
#include "module.inc"
#include "new.inc"
#include "plugindialog.inc"
#include "quit.inc"
#include "render.inc"
#include "threadloader.inc"

#define TOTAL_LOADS 5      // number of files to cache
#define TOTAL_EFFECTS 5     // number of effects to cache

class MainMenu : public BC_MenuBar
{
public:
	MainMenu(MainWindowGUI *gui);
	~MainMenu();
	int create_objects();
	int load_defaults(Defaults *defaults);
	int save_defaults(Defaults *defaults);

// most recent loads
	int add_load(const char *path);
	int init_loads(Defaults *defaults);
	int save_loads(Defaults *defaults);

// most recent effects
	int init_aeffects(Defaults *defaults);
	int save_aeffects(Defaults *defaults);
	int add_aeffect(const char *title);
	int init_veffects(Defaults *defaults);
	int save_veffects(Defaults *defaults);
	int add_veffect(const char *title);

	int change_channels(int old_channels, int new_channels);

	int quit();
	int set_show_console(int checked);
	int set_show_levels(int checked);
	int set_show_video(int checked);
// show only one of these at a time
	int set_show_autos();

	MainWindowGUI *gui;
	MainWindow *mwindow;
	ThreadLoader *threadloader;
	PanMenu *panmenu;
	PluginMenu *pluginmenu;
	PanItem *panitem[MAXCHANNELS];
	PluginItem *pluginitem[PLUGINS];
	MenuAEffects *aeffects;
	MenuVEffects *veffects;
	AssetManager *asset_manager;

// for previous document loader
	Load *load_file;
	LoadPrevious *load[TOTAL_LOADS];
	int total_loads;


	Render *render, *render_list;
	New *new_project;
	MenuAEffectItem *aeffect[TOTAL_EFFECTS];
	MenuVEffectItem *veffect[TOTAL_EFFECTS];
	Quit *quit_program;              // affected by save
	Undo *undo;
	Redo *redo;
	int total_aeffects;
	int total_veffects;
	BC_Menu *filemenu, *audiomenu, *videomenu;      // needed by most recents
	ShowConsole *show_console;
	ShowLevels *show_levels;
	ShowVideo *show_video;
	ShowEdits *show_edits;
	ShowTitles *show_titles;
	CameraAutomation *camera_automation;
	ProjectAutomation *project_automation;
	ShowRenderedOutput *show_output;
	FadeAutomation *fade_automation;
	PlayAutomation *play_automation;
	MuteAutomation *mute_automation;
	PanAutomation *panautomation;
	PluginAutomation *plugin_automation;
};

// ========================================= edit

class Undo : public BC_MenuItem
{
public:
	Undo(MainWindow *mwindow);
	int handle_event();
	int update_caption(const char *new_caption = "");
	MainWindow *mwindow;
};



class DumpCache : public BC_MenuItem
{
public:
	DumpCache(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class DumpEDL : public BC_MenuItem
{
public:
	DumpEDL(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class DumpAssets : public BC_MenuItem
{
public:
	DumpAssets(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class Redo : public BC_MenuItem
{
public:
	Redo(MainWindow *mwindow);
	int handle_event();
	int update_caption(const char *new_caption = "");
	MainWindow *mwindow;
};

class Clear : public BC_MenuItem
{
public:
	Clear(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class CutAutomation : public BC_MenuItem
{
public:
	CutAutomation(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class CopyAutomation : public BC_MenuItem
{
public:
	CopyAutomation(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class PasteAutomation : public BC_MenuItem
{
public:
	PasteAutomation(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class ClearAutomation : public BC_MenuItem
{
public:
	ClearAutomation(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class PasteSilence : public BC_MenuItem
{
public:
	PasteSilence(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class SelectAll : public BC_MenuItem
{
public:
	SelectAll(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class ClearLabels : public BC_MenuItem
{
public:
	ClearLabels(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

// ======================================== audio

class AddAudioTrack : public BC_MenuItem
{
public:
	AddAudioTrack(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class DeleteAudioTrack : public BC_MenuItem
{
public:
	DeleteAudioTrack(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class MuteAudio : public BC_MenuItem
{
public:
	MuteAudio(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class TrimSelection : public BC_MenuItem
{
public:
	TrimSelection(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

// ========================================== video


class AddVideoTrack : public BC_MenuItem
{
public:
	AddVideoTrack(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};


class DeleteVideoTrack : public BC_MenuItem
{
public:
	DeleteVideoTrack(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class ResetTranslation : public BC_MenuItem
{
public:
	ResetTranslation(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

// ========================================== settings


class MoveTracksUp : public BC_MenuItem
{
public:
	MoveTracksUp(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class MoveTracksDown : public BC_MenuItem
{
public:
	MoveTracksDown(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class DeleteTracks : public BC_MenuItem
{
public:
	DeleteTracks(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class ConcatenateTracks : public BC_MenuItem
{
public:
	ConcatenateTracks(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class DeleteTrack : public BC_MenuItem
{
public:
	DeleteTrack(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class LoopPlayback : public BC_MenuItem
{
public:
	LoopPlayback(MainWindow *mwindow);

	int handle_event();
	MainWindow *mwindow;
};

class LabelsFollowEdits : public BC_MenuItem
{
public:
	LabelsFollowEdits(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class CursorOnFrames : public BC_MenuItem
{
public:
	CursorOnFrames(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class AutosFollowEdits : public BC_MenuItem
{
public:
	AutosFollowEdits(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class ScrubSpeed : public BC_MenuItem
{
public:
	ScrubSpeed(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class SaveSettingsNow : public BC_MenuItem
{
public:
	SaveSettingsNow(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

// ========================================== window

class ShowEdits : public BC_MenuItem
{
public:
	ShowEdits(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class ShowConsole : public BC_MenuItem
{
public:
	ShowConsole(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class ShowLevels : public BC_MenuItem
{
public:
	ShowLevels(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class ShowVideo : public BC_MenuItem
{
public:
	ShowVideo(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class OriginalSize : public BC_MenuItem
{
public:
	OriginalSize(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class ShowTitles : public BC_MenuItem
{
public:
	ShowTitles(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class VerticalTracks : public BC_MenuItem
{
public:
	VerticalTracks(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};


class CameraAutomation : public BC_MenuItem
{
public:
	CameraAutomation(MainWindow *mwindow, MainMenu *menu);
	int handle_event();
	MainWindow *mwindow;
	MainMenu *menu;
};

class ProjectAutomation : public BC_MenuItem
{
public:
	ProjectAutomation(MainWindow *mwindow, MainMenu *menu);
	int handle_event();
	MainWindow *mwindow;
	MainMenu *menu;
};

class ShowRenderedOutput : public BC_MenuItem
{
public:
	ShowRenderedOutput(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class FadeAutomation : public BC_MenuItem
{
public:
	FadeAutomation(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class PlayAutomation : public BC_MenuItem
{
public:
	PlayAutomation(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class MuteAutomation : public BC_MenuItem
{
public:
	MuteAutomation(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class PanAutomation : public BC_MenuItem
{
public:
	PanAutomation(MainWindow *mwindow);
	int handle_event();
	int change_channels(int old_channels, int new_channels);
	MainWindow *mwindow;
};

class PanMenu : public BC_SubMenu
{
public:
	PanMenu();
};

class PanItem : public BC_SubMenuItem
{
public:
	PanItem(MainWindow *mwindow, char *text, int number);
	int handle_event();
	MainWindow *mwindow;
	int number;
};

class PluginAutomation : public BC_MenuItem
{
public:
	PluginAutomation(MainWindow *mwindow);
	int handle_event();
	MainWindow *mwindow;
};

class PluginMenu : public BC_SubMenu
{
public:
	PluginMenu();
};

class PluginItem : public BC_SubMenuItem
{
public:
	PluginItem(MainWindow *mwindow, const char *text, int number);
	int handle_event();
	MainWindow *mwindow;
	int number;
};

#endif
