#include <string.h>
#include "assets.h"
#include "assetmanager.h"
#include "cache.h"
#include "console.h"
#include "cropvideo.h"
#include "defaults.h"
#include "featheredits.h"
#include "levelwindow.h"
#include "loadfile.h"
#include "mainmenu.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "menuaeffects.h"
#include "menuveffects.h"
#include "new.h"
#include "playbackengine.h"
#include "preferences.h"
#include "preferencesthread.h"
#include "quit.h"
#include "render.h"
#include "savefile.h"
#include "scale.h"
#include "setchannels.h"
#include "setsamplerate.h"
#include "setframerate.h"
#include "timebar.h"
#include "tracks.h"
#include "transition.h"
#include "videowindow.h"
#include "videowindowgui.h"

MainMenu::MainMenu(MainWindowGUI *gui) : BC_MenuBar(0, 0, gui->get_w())
{ 
	this->gui = gui;
	this->mwindow = gui->mwindow; 
}

MainMenu::~MainMenu()
{
	//printf("~MainMenu\n");
}

int MainMenu::create_objects()
{
	BC_Menu *editmenu, *viewmenu, *windowmenu, *settingsmenu, *trackmenu;
	PreferencesMenuitem *preferences;
	total_loads = 0; 

	add_menu(filemenu = new BC_Menu("File"));
	filemenu->add_menuitem(new_project = new New(mwindow));

// file loaders
	filemenu->add_menuitem(load_file = new Load(mwindow, "Load...", "o", 'o', 0));
	filemenu->add_menuitem(new Load(mwindow, "Append...", "i", 'i', 1));
	load_file->set_mainmenu(this);

// new and load can be undone so no need to prompt save
	Save *save;                   //  affected by saveas
	filemenu->add_menuitem(save = new Save(mwindow));
	SaveAs *saveas;
	filemenu->add_menuitem(saveas = new SaveAs(mwindow));
	save->create_objects(saveas);
	saveas->set_mainmenu(this);

	filemenu->add_menuitem(render = new Render(mwindow->defaults, mwindow, 0));
	filemenu->add_menuitem(render_list = new Render(mwindow->defaults, mwindow, 1));
	filemenu->add_menuitem(asset_manager = new AssetManager(mwindow));
	filemenu->add_menuitem(new BC_MenuItem("-"));
	filemenu->add_menuitem(quit_program = new Quit(mwindow));
	quit_program->create_objects(save);

	add_menu(editmenu = new BC_Menu("Edit"));
	editmenu->add_menuitem(undo = new Undo(mwindow));
	editmenu->add_menuitem(redo = new Redo(mwindow));
	editmenu->add_menuitem(new BC_MenuItem("-"));
	editmenu->add_menuitem(new Clear(mwindow));
	editmenu->add_menuitem(new PasteSilence(mwindow));
	editmenu->add_menuitem(new MuteAudio(mwindow));
	editmenu->add_menuitem(new TrimSelection(mwindow));
	editmenu->add_menuitem(new SelectAll(mwindow));
	editmenu->add_menuitem(new BC_MenuItem("-"));
	editmenu->add_menuitem(new CutAutomation(mwindow));
	editmenu->add_menuitem(new CopyAutomation(mwindow));
	editmenu->add_menuitem(new PasteAutomation(mwindow));
	editmenu->add_menuitem(new ClearAutomation(mwindow));
	editmenu->add_menuitem(new BC_MenuItem("-"));
	editmenu->add_menuitem(new ClearLabels(mwindow));

	add_menu(audiomenu = new BC_Menu("Audio"));
	audiomenu->add_menuitem(new AddAudioTrack(mwindow));
	//audiomenu->add_menuitem(new DeleteAudioTrack(mwindow));
	audiomenu->add_menuitem(new SetSampleRate(mwindow));
	audiomenu->add_menuitem(new SetChannels(mwindow));
	audiomenu->add_menuitem(new BC_MenuItem("-"));
	audiomenu->add_menuitem(new TransitionMenuItem(mwindow, 1, 0));
	audiomenu->add_menuitem(new FeatherEdits(mwindow, 1, 0));
	audiomenu->add_menuitem(aeffects = new MenuAEffects(mwindow));

	add_menu(videomenu = new BC_Menu("Video"));
	videomenu->add_menuitem(new AddVideoTrack(mwindow));
	//videomenu->add_menuitem(new DeleteVideoTrack(mwindow));
	videomenu->add_menuitem(new SetFrameRate(mwindow));
	videomenu->add_menuitem(new Scale(mwindow));
	videomenu->add_menuitem(new CropVideo(mwindow));
	videomenu->add_menuitem(new BC_MenuItem("-"));
	videomenu->add_menuitem(new FeatherEdits(mwindow, 0, 1));
	videomenu->add_menuitem(new TransitionMenuItem(mwindow, 0, 1));
	videomenu->add_menuitem(new ResetTranslation(mwindow));
	videomenu->add_menuitem(veffects = new MenuVEffects(mwindow));

	add_menu(trackmenu = new BC_Menu("Tracks"));
	trackmenu->add_menuitem(new MoveTracksUp(mwindow));
	trackmenu->add_menuitem(new MoveTracksDown(mwindow));
	trackmenu->add_menuitem(new DeleteTracks(mwindow));
	trackmenu->add_menuitem(new ConcatenateTracks(mwindow));

	add_menu(settingsmenu = new BC_Menu("Settings"));
// set scrubbing speed
	ScrubSpeed *scrub_speed;
	settingsmenu->add_menuitem(scrub_speed = new ScrubSpeed(mwindow));
	if(mwindow->preferences->scrub_speed == .5) scrub_speed->set_text("Fast Shuttle");

	settingsmenu->add_menuitem(new LabelsFollowEdits(mwindow));
	settingsmenu->add_menuitem(new AutosFollowEdits(mwindow));
	settingsmenu->add_menuitem(new CursorOnFrames(mwindow));
	settingsmenu->add_menuitem(new LoopPlayback(mwindow));

	settingsmenu->add_menuitem(preferences = new PreferencesMenuitem(mwindow));
	mwindow->preferences_thread = preferences->thread;
	settingsmenu->add_menuitem(new SaveSettingsNow(mwindow));
	add_menu(viewmenu = new BC_Menu("View"));
	viewmenu->add_menuitem(show_edits = new ShowEdits(mwindow));
	viewmenu->add_menuitem(show_titles = new ShowTitles(mwindow));
	viewmenu->add_menuitem(fade_automation = new FadeAutomation(mwindow));
	viewmenu->add_menuitem(play_automation = new PlayAutomation(mwindow));
	viewmenu->add_menuitem(mute_automation = new MuteAutomation(mwindow));
	viewmenu->add_menuitem(plugin_automation = new PluginAutomation(mwindow));
	viewmenu->add_menuitem(panautomation = new PanAutomation(mwindow));
	viewmenu->add_menuitem(new BC_MenuItem("-"));
	viewmenu->add_menuitem(show_output = new ShowRenderedOutput(mwindow));
	viewmenu->add_menuitem(camera_automation = new CameraAutomation(mwindow, this));
	viewmenu->add_menuitem(project_automation = new ProjectAutomation(mwindow, this));

	add_menu(windowmenu = new BC_Menu("Window"));
	windowmenu->add_menuitem(show_console = new ShowConsole(mwindow));
	windowmenu->add_menuitem(show_levels = new ShowLevels(mwindow));
	windowmenu->add_menuitem(show_video = new ShowVideo(mwindow));
	windowmenu->add_menuitem(new OriginalSize(mwindow));
	windowmenu->add_menuitem(new VerticalTracks(mwindow));
	//windowmenu->add_menuitem(new InverseAutomation(0));

	panautomation->add_submenu(panmenu = new PanMenu);
	char string[256];
	for(int i = 0; i < mwindow->output_channels; i++)
	{
		sprintf(string, "Channel %d", i + 1);
		panmenu->add_submenuitem(panitem[i] = new PanItem(mwindow, string, i));
	}
	

	plugin_automation->add_submenu(pluginmenu = new PluginMenu);
	for(int i = 0; i < PLUGINS; i++)
	{
		sprintf(string, "Plugin %d", i + 1);
		pluginmenu->add_submenuitem(pluginitem[i] = new PluginItem(mwindow, string, i));
	}

// 	BC_Menu *debugmenu;
// 	add_menu(debugmenu = new BC_Menu("Debug"));
// 	debugmenu->add_menuitem(new DumpCache(mwindow));
// 	debugmenu->add_menuitem(new DumpAssets(mwindow));
// 	debugmenu->add_menuitem(new DumpEDL(mwindow));
return 0;
}

int MainMenu::load_defaults(Defaults *defaults)
{
	init_loads(defaults);
	init_aeffects(defaults);
	init_veffects(defaults);
	show_edits->set_checked(mwindow->tracks->handles);
	show_titles->set_checked(mwindow->tracks->titles);
	fade_automation->set_checked(mwindow->tracks->auto_conf.fade);
	play_automation->set_checked(mwindow->tracks->auto_conf.play);
	camera_automation->set_checked(mwindow->tracks->auto_conf.camera);
	project_automation->set_checked(mwindow->tracks->auto_conf.projector);
	if(!mwindow->tracks->show_output) 
		show_output->set_text("Draw Output");
	else
		show_output->set_text("Draw Tracks");

	int i;
	for(i = 0; i < mwindow->output_channels; i++) 
		panitem[i]->set_checked(mwindow->tracks->auto_conf.pan[i]);

	for(i = 0; i < PLUGINS; i++) 
		pluginitem[i]->set_checked(mwindow->tracks->auto_conf.plugin[i]);
return 0;
}

int MainMenu::save_defaults(Defaults *defaults)
{
	save_loads(defaults);
	save_aeffects(defaults);
	save_veffects(defaults);
return 0;
}





int MainMenu::change_channels(int old_channels, int new_channels)
{
	if(new_channels < old_channels)
	{
		for(int i = new_channels; i < old_channels; i++)
		{
			delete panitem[i];
		}
	}
	else
	{
		for(int i = old_channels; i < new_channels; i++)
		{
			char string[256];
			sprintf(string, "Channel %d", i + 1);
			panmenu->add_submenuitem(panitem[i] = new PanItem(mwindow, string, i));
			panitem[i]->set_checked(mwindow->tracks->auto_conf.pan[i]);
		}
	}
return 0;
}

int MainMenu::quit()
{
	quit_program->handle_event();
return 0;
}





// ================================== load most recent

int MainMenu::init_aeffects(Defaults *defaults)
{
	total_aeffects = defaults->get("TOTAL_AEFFECTS", 0);
	
	char string[1024], title[1024];
	if(total_aeffects) audiomenu->add_menuitem(new BC_MenuItem("-"));
	
	for(int i = 0; i < total_aeffects; i++)
	{
		sprintf(string, "AEFFECTRECENT%d", i);
		defaults->get(string, title);
		audiomenu->add_menuitem(aeffect[i] = new MenuAEffectItem(aeffects, title));
	}
return 0;
}

int MainMenu::init_veffects(Defaults *defaults)
{
	total_veffects = defaults->get("TOTAL_VEFFECTS", 0);
	
	char string[1024], title[1024];
	if(total_veffects) videomenu->add_menuitem(new BC_MenuItem("-"));
	
	for(int i = 0; i < total_veffects; i++)
	{
		sprintf(string, "VEFFECTRECENT%d", i);
		defaults->get(string, title);
		videomenu->add_menuitem(veffect[i] = new MenuVEffectItem(veffects, title));
	}
return 0;
}

int MainMenu::init_loads(Defaults *defaults)
{
	total_loads = defaults->get("TOTAL_LOADS", 0);
	char string[1024], path[1024], filename[1024];
	FileSystem dir;
	if(total_loads > 0) filemenu->add_menuitem(new BC_MenuItem("-"));

	for(int i = 0; i < total_loads; i++)
	{
		sprintf(string, "LOADPREVIOUS%d", i);
		defaults->get(string, path);

		filemenu->add_menuitem(load[i] = new LoadPrevious(load_file));
		dir.extract_name(filename, path);
		load[i]->set_text(filename);
		load[i]->set_path(path);
	}
return 0;
}

// ============================ save most recent

int MainMenu::save_aeffects(Defaults *defaults)
{
	defaults->update("TOTAL_AEFFECTS", total_aeffects);
	char string[1024];
	for(int i = 0; i < total_aeffects; i++)
	{
		sprintf(string, "AEFFECTRECENT%d", i);
		defaults->update(string, aeffect[i]->get_text());
	}
return 0;
}

int MainMenu::save_veffects(Defaults *defaults)
{
	defaults->update("TOTAL_VEFFECTS", total_veffects);
	char string[1024];
	for(int i = 0; i < total_veffects; i++)
	{
		sprintf(string, "VEFFECTRECENT%d", i);
		defaults->update(string, veffect[i]->get_text());
	}
return 0;
}

int MainMenu::save_loads(Defaults *defaults)
{
	defaults->update("TOTAL_LOADS", total_loads);
	char string[1024];
	for(int i = 0; i < total_loads; i++)
	{
		sprintf(string, "LOADPREVIOUS%d", i);
		defaults->update(string, load[i]->path);
	}
return 0;
}

// =================================== add most recent

int MainMenu::add_aeffect(char *title)
{
// add bar for first effect
	if(total_aeffects == 0)
	{
		audiomenu->add_menuitem(new BC_MenuItem("-"));
	}

// test for existing copy of effect
	for(int i = 0; i < total_aeffects; i++)
	{
		if(!strcmp(aeffect[i]->text, title))     // already exists
		{                                // swap for top effect
			for(int j = i; j > 0; j--)   // move preceeding effects down
			{
				aeffect[j]->set_text(aeffect[j - 1]->text);
			}
			aeffect[0]->set_text(title);
			return 1;
		}
	}

// add another blank effect
	if(total_aeffects < TOTAL_EFFECTS)
	{
		audiomenu->add_menuitem(aeffect[total_aeffects] = new MenuAEffectItem(aeffects, ""));
		total_aeffects++;
	}

// cycle effect down
	for(int i = total_aeffects - 1; i > 0; i--)
	{
	// set menu item text
		aeffect[i]->set_text(aeffect[i - 1]->text);
	}

// set up the new effect
	aeffect[0]->set_text(title);
	return 0;
return 0;
}

int MainMenu::add_veffect(char *title)
{
// add bar for first effect
	if(total_veffects == 0)
	{
		videomenu->add_menuitem(new BC_MenuItem("-"));
	}

// test for existing copy of effect
	for(int i = 0; i < total_veffects; i++)
	{
		if(!strcmp(veffect[i]->text, title))     // already exists
		{                                // swap for top effect
			for(int j = i; j > 0; j--)   // move preceeding effects down
			{
				veffect[j]->set_text(veffect[j - 1]->text);
			}
			veffect[0]->set_text(title);
			return 1;
		}
	}

// add another blank effect
	if(total_veffects < TOTAL_EFFECTS)
	{
		videomenu->add_menuitem(veffect[total_veffects] = new MenuVEffectItem(veffects, ""));
		total_veffects++;
	}

// cycle effect down
	for(int i = total_veffects - 1; i > 0; i--)
	{
// set menu item text
		veffect[i]->set_text(veffect[i - 1]->text);
	}

// set up the new effect
	veffect[0]->set_text(title);
	return 0;
return 0;
}

int MainMenu::add_load(char *path)
{
	if(total_loads == 0)
	{
		filemenu->add_menuitem(new BC_MenuItem("-"));
	}
	
// test for existing copy
	FileSystem fs;
	char text[1024], new_path[1024];      // get text and path
	fs.extract_name(text, path);
	strcpy(new_path, path);
	
	for(int i = 0; i < total_loads; i++)
	{
		if(!strcmp(load[i]->text, text))     // already exists
		{                                // swap for top load
			for(int j = i; j > 0; j--)   // move preceeding loads down
			{
				load[j]->set_text(load[j - 1]->text);
				load[j]->set_path(load[j - 1]->path);
			}
			load[0]->set_text(text);
			load[0]->set_path(new_path);
			
			return 1;
		}
	}
	
// add another load
	if(total_loads < TOTAL_LOADS)
	{
		filemenu->add_menuitem(load[total_loads] = new LoadPrevious(load_file));
		total_loads++;
	}
	
// cycle loads down
	for(int i = total_loads - 1; i > 0; i--)
	{
	// set menu item text
		load[i]->set_text(load[i - 1]->text);
	// set filename
		load[i]->set_path(load[i - 1]->path);
	}

// set up the new load
	load[0]->set_text(text);
	load[0]->set_path(new_path);
	return 0;
return 0;
}








// ================================== menu items

int MainMenu::set_show_console(int checked)
{
	show_console->set_checked(checked);
return 0;
}

int MainMenu::set_show_levels(int checked)
{
	show_levels->set_checked(checked);
return 0;
}

int MainMenu::set_show_video(int checked)
{
	show_video->set_checked(checked);
return 0;
}

int MainMenu::set_show_autos()
{
	mwindow->tracks->set_show_autos(camera_automation->get_checked(), project_automation->get_checked());
return 0;
}

DumpCache::DumpCache(MainWindow *mwindow)
 : BC_MenuItem("Dump Cache")
{ this->mwindow = mwindow; }

int DumpCache::handle_event()
{
	mwindow->cache->dump();
return 0;
}

DumpEDL::DumpEDL(MainWindow *mwindow)
 : BC_MenuItem("Dump Edits")
{ this->mwindow = mwindow; }

int DumpEDL::handle_event()
{
	mwindow->tracks->dump();
return 0;
}


DumpAssets::DumpAssets(MainWindow *mwindow)
 : BC_MenuItem("Dump Assets")
{ this->mwindow = mwindow; }

int DumpAssets::handle_event()
{
	mwindow->assets->dump();
return 0;
}

// ================================================= edit

Undo::Undo(MainWindow *mwindow) : BC_MenuItem("Undo", "z", 'z') 
{ this->mwindow = mwindow; }
int Undo::handle_event()
{ 
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->undo(); 
	mwindow->console->gui->unlock_window();
return 0;
}
int Undo::update_caption(char *new_caption)
{
	char string[1024];
	sprintf(string, "Undo %s", new_caption);
	set_text(string);
return 0;
}


Redo::Redo(MainWindow *mwindow) : BC_MenuItem("Redo", "Shift+Z", 'Z') 
{ set_shift(); this->mwindow = mwindow; }

int Redo::handle_event()
{ 
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->redo(); 
	mwindow->console->gui->unlock_window();
return 0;
}
int Redo::update_caption(char *new_caption)
{
	char string[1024];
	sprintf(string, "Redo %s", new_caption);
	set_text(string);
return 0;
}

CutAutomation::CutAutomation(MainWindow *mwindow) : BC_MenuItem("Cut Auto", "Shift-X", 'X')
{ set_shift(); this->mwindow = mwindow; }

int CutAutomation::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->undo->update_undo_automation("Automation", 0); 
	mwindow->cut_automation(); 
	mwindow->undo->update_undo_automation(); 
return 0;
}

CopyAutomation::CopyAutomation(MainWindow *mwindow) : BC_MenuItem("Copy Auto", "Shift-C", 'C')
{ set_shift(); this->mwindow = mwindow; }

int CopyAutomation::handle_event()
{ 
	mwindow->stop_playback(1);
	mwindow->undo->update_undo_automation("Automation", 0); 
	mwindow->copy_automation(); 
	mwindow->undo->update_undo_automation(); 
return 0;
}

PasteAutomation::PasteAutomation(MainWindow *mwindow) : BC_MenuItem("Paste Auto", "Shift-V", 'V')
{ set_shift(); this->mwindow = mwindow; }

int PasteAutomation::handle_event()
{ 
	mwindow->stop_playback(1);
	mwindow->undo->update_undo_automation("Automation", 0); 
	mwindow->paste_automation(); 
	mwindow->undo->update_undo_automation(); 
return 0;
}

ClearAutomation::ClearAutomation(MainWindow *mwindow) : BC_MenuItem("Clear Auto", "Shift-Del", BACKSPACE)
{ set_shift(); this->mwindow = mwindow; }

int ClearAutomation::handle_event()
{ 
	mwindow->stop_playback(1);
	mwindow->undo->update_undo_automation("Automation", 0); 
	mwindow->clear_automation(); 
	mwindow->undo->update_undo_automation(); 
return 0;
}

Clear::Clear(MainWindow *mwindow) : BC_MenuItem("Clear", "Del", BACKSPACE) 
{ this->mwindow = mwindow; }

int Clear::handle_event()
{ 
	mwindow->stop_playback(1);
	mwindow->undo->update_undo_edits("Clear", 0); 
	mwindow->clear(mwindow->selectionstart, mwindow->selectionend); 
	mwindow->undo->update_undo_edits(); 
return 0;
}

PasteSilence::PasteSilence(MainWindow *mwindow) : BC_MenuItem("Paste silence", "Shift+Space", ' ') 
{ this->mwindow = mwindow; set_shift(); }

int PasteSilence::handle_event()
{ 
	mwindow->stop_playback(1);
	mwindow->undo->update_undo_edits("Silence", 0); 
	mwindow->paste_silence(); 
	mwindow->undo->update_undo_edits(); 
return 0;
}

SelectAll::SelectAll(MainWindow *mwindow) : BC_MenuItem("Select All", "a", 'a') 
{ this->mwindow = mwindow; }

int SelectAll::handle_event()
{
	mwindow->stop_playback(); 
	mwindow->set_selection(0, mwindow->tracks->total_samples()); 
	mwindow->timebar->draw();

return 0;
}

ClearLabels::ClearLabels(MainWindow *mwindow) : BC_MenuItem("Clear labels") 
{ this->mwindow = mwindow; }

int ClearLabels::handle_event()
{ 
	mwindow->undo->update_undo_timebar("Clear Labels", 0);
	mwindow->timebar->clear_labels(mwindow->selectionstart, mwindow->selectionend); 
	mwindow->undo->update_undo_timebar(); 
return 0;
}












// ============================================= audio

AddAudioTrack::AddAudioTrack(MainWindow *mwindow)
 : BC_MenuItem("Add track", "t", 't')
{
	this->mwindow = mwindow;
}

int AddAudioTrack::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->update_undo_audio("Add track", 0);
	mwindow->add_audio_track();
	mwindow->undo->update_undo_audio();
	mwindow->console->gui->unlock_window();
return 0;
}

DeleteAudioTrack::DeleteAudioTrack(MainWindow *mwindow)
 : BC_MenuItem("Delete track")
{
	this->mwindow = mwindow;
}

int DeleteAudioTrack::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->update_undo_audio("Delete track", 0);
	mwindow->tracks->delete_audio_track();
	mwindow->undo->update_undo_all();
	mwindow->console->gui->unlock_window();
return 0;
}



MuteAudio::MuteAudio(MainWindow *mwindow)
 : BC_MenuItem("Mute Region", "m", 'm')
{
	this->mwindow = mwindow;
}

int MuteAudio::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->undo->update_undo_edits("Mute", 0);
	mwindow->mute_audio();
	mwindow->undo->update_undo_edits();
return 0;
}


TrimSelection::TrimSelection(MainWindow *mwindow)
 : BC_MenuItem("Trim Selection")
{
	this->mwindow = mwindow;
}

int TrimSelection::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->undo->update_undo_edits("Trim Selection", 0);
	mwindow->trim_selection();
	mwindow->undo->update_undo_edits();
return 0;
}


// ============================================= video


AddVideoTrack::AddVideoTrack(MainWindow *mwindow)
 : BC_MenuItem("Add track", "Shift-T", 'T')
{
	set_shift();
	this->mwindow = mwindow;
}

int AddVideoTrack::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->update_undo_audio("Add track", 0);
	mwindow->add_video_track();
	mwindow->undo->update_undo_audio();
	mwindow->console->gui->unlock_window();
return 0;
}


DeleteVideoTrack::DeleteVideoTrack(MainWindow *mwindow)
 : BC_MenuItem("Delete track")
{
	this->mwindow = mwindow;
}

int DeleteVideoTrack::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->update_undo_audio("Delete track", 0);
	mwindow->tracks->delete_video_track();
	mwindow->undo->update_undo_audio();
	mwindow->console->gui->unlock_window();
return 0;
}



ResetTranslation::ResetTranslation(MainWindow *mwindow)
 : BC_MenuItem("Reset Translation")
{
	this->mwindow = mwindow;
}

int ResetTranslation::handle_event()
{
	mwindow->console->gui->lock_window();
	mwindow->undo->update_undo_edits("Reset Translation", 0);
	mwindow->tracks->reset_translation(mwindow->selectionstart, mwindow->selectionend);
	mwindow->undo->update_undo_edits();
	mwindow->console->gui->unlock_window();
return 0;
}














// ============================================ settings

DeleteTracks::DeleteTracks(MainWindow *mwindow)
 : BC_MenuItem("Delete tracks")
{
	this->mwindow = mwindow;
}

int DeleteTracks::handle_event()
{
// Main Window is locked here.
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->update_undo_all("Delete tracks", 0);
	mwindow->tracks->delete_tracks();
	mwindow->undo->update_undo_audio();
	mwindow->console->gui->unlock_window();
return 0;
}

DeleteTrack::DeleteTrack(MainWindow *mwindow)
 : BC_MenuItem("Delete track", "d", 'd')
{
	this->mwindow = mwindow;
}

int DeleteTrack::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->update_undo_all("Delete tracks", 0);
	mwindow->tracks->delete_track();
	mwindow->undo->update_undo_audio();
	mwindow->console->gui->unlock_window();
return 0;
}

MoveTracksUp::MoveTracksUp(MainWindow *mwindow)
 : BC_MenuItem("Move tracks up", "Shift-U", 'U')
{
	set_shift(); this->mwindow = mwindow;
}

int MoveTracksUp::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->update_undo_all("Move tracks up", 0);
	mwindow->tracks->move_tracks_up();
	mwindow->undo->update_undo_audio();
	mwindow->console->gui->unlock_window();
return 0;
}

MoveTracksDown::MoveTracksDown(MainWindow *mwindow)
 : BC_MenuItem("Move tracks down", "Shift-D", 'D')
{
	set_shift(); this->mwindow = mwindow;
}

int MoveTracksDown::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->update_undo_all("Move tracks down", 0);
	mwindow->tracks->move_tracks_down();
	mwindow->undo->update_undo_audio();
	mwindow->console->gui->unlock_window();
return 0;
}




ConcatenateTracks::ConcatenateTracks(MainWindow *mwindow)
 : BC_MenuItem("Concatenate tracks")
{
	set_shift(); this->mwindow = mwindow;
}

int ConcatenateTracks::handle_event()
{
	mwindow->stop_playback(1);
	mwindow->console->gui->lock_window();
	mwindow->undo->update_undo_edits("Concatenate tracks", 0);
	mwindow->tracks->concatenate_tracks();
	mwindow->undo->update_undo_edits();
	mwindow->console->gui->unlock_window();
return 0;
}





LoopPlayback::LoopPlayback(MainWindow *mwindow) : BC_MenuItem("Loop Playback", "Shift+L", 'L')
{
	this->mwindow = mwindow;
	set_checked(mwindow->loop_playback);
	set_shift();
}

int LoopPlayback::handle_event()
{
	mwindow->set_loop_boundaries();
	set_checked(mwindow->loop_playback);
return 0;
}



LabelsFollowEdits::LabelsFollowEdits(MainWindow *mwindow) : BC_MenuItem("Labels follow edits") 
{ 
	this->mwindow = mwindow; 
	set_checked(mwindow->labels_follow_edits);
}

int LabelsFollowEdits::handle_event()
{
	set_checked(get_checked() ^ 1);
	mwindow->labels_follow_edits = get_checked(); 
return 0;
}

AutosFollowEdits::AutosFollowEdits(MainWindow *mwindow) : BC_MenuItem("Autos follow edits") 
{ 
	this->mwindow = mwindow; 
	set_checked(mwindow->autos_follow_edits);
}

int AutosFollowEdits::handle_event()
{ mwindow->autos_follow_edits ^= 1; checked ^= 1;return 0;
}


CursorOnFrames::CursorOnFrames(MainWindow *mwindow) : BC_MenuItem("Align cursor on frames") 
{ 
	this->mwindow = mwindow; 
	set_checked(mwindow->cursor_on_frames);
}

int CursorOnFrames::handle_event()
{ mwindow->cursor_on_frames ^= 1; checked ^= 1;return 0;
}


ScrubSpeed::ScrubSpeed(MainWindow *mwindow) : BC_MenuItem("Slow Shuttle")
{
	this->mwindow = mwindow;
}

int ScrubSpeed::handle_event()
{
	if(mwindow->preferences->scrub_speed == .5)
	{
		mwindow->preferences->scrub_speed = 2;
		set_text("Slow Shuttle");
	}
	else
	{
		mwindow->preferences->scrub_speed = .5;
		set_text("Fast Shuttle");
	}
return 0;
}

SaveSettingsNow::SaveSettingsNow(MainWindow *mwindow) : BC_MenuItem("Save settings now") 
{ 
	this->mwindow = mwindow; 
}

int SaveSettingsNow::handle_event()
{
	mwindow->save_defaults();
	mwindow->defaults->save();
return 0;
}



// ============================================ window

ShowConsole::ShowConsole(MainWindow *mwindow)
 : BC_MenuItem("Show Console", "")
{ 
	this->mwindow = mwindow; 
	set_checked(!mwindow->defaults->get("HIDECONSOLE", 0));
}

int ShowConsole::handle_event()
{
	set_checked(get_checked() ^ 1);
	if(checked) mwindow->console->gui->show_window(); else mwindow->console->gui->hide_window();
return 0;
}







ShowLevels::ShowLevels(MainWindow *mwindow)
 : BC_MenuItem("Show Levels", "")
{ 
	this->mwindow = mwindow; 
	set_checked(mwindow->defaults->get("METERVISIBLE", 1));
}

int ShowLevels::handle_event()
{
	set_checked(get_checked() ^ 1);
	if(checked) mwindow->level_window->show_window(); else mwindow->level_window->hide_window();
return 0;
}


ShowVideo::ShowVideo(MainWindow *mwindow)
 : BC_MenuItem("Show Video", "")
{ 
	this->mwindow = mwindow; 
	set_checked(mwindow->defaults->get("VIDEOVISIBLE", 1));
}

int ShowVideo::handle_event()
{
	set_checked(get_checked() ^ 1);

	if(checked) 
		mwindow->video_window->show_window();
	else
		mwindow->video_window->hide_window();
return 0;
}

OriginalSize::OriginalSize(MainWindow *mwindow)
 : BC_MenuItem("Original size")
{ 
	this->mwindow = mwindow; 
}

int OriginalSize::handle_event()
{
	mwindow->video_window->gui->lock_window();
	mwindow->video_window->original_size();
	mwindow->video_window->gui->unlock_window();
return 0;
}








ShowEdits::ShowEdits(MainWindow *mwindow) : BC_MenuItem("Show edits", "1", '1')
{ this->mwindow = mwindow; set_checked(0); }
int ShowEdits::handle_event()
{
	mwindow->tracks->toggle_handles();
	set_checked(get_checked() ^ 1);
return 0;
}

ShowTitles::ShowTitles(MainWindow *mwindow) : BC_MenuItem("Show titles", "2", '2')
{ this->mwindow = mwindow; set_checked(0); }
int ShowTitles::handle_event()
{
	mwindow->tracks->toggle_titles();
	set_checked(get_checked() ^ 1);
return 0;
}

VerticalTracks::VerticalTracks(MainWindow *mwindow)
 : BC_MenuItem("Vertical tracks", "")
{
	this->mwindow = mwindow;
	set_checked(mwindow->tracks_vertical);
}

int VerticalTracks::handle_event()
{
	set_checked(get_checked() ^ 1);
	mwindow->flip_vertical(checked);
return 0;
}



FadeAutomation::FadeAutomation(MainWindow *mwindow) : BC_MenuItem("Fade autos", "3", '3')
{ this->mwindow = mwindow; set_checked(0); } 
int FadeAutomation::handle_event()
{ 
	mwindow->tracks->toggle_auto_fade();
	set_checked(get_checked() ^ 1);
return 0;
}

PlayAutomation::PlayAutomation(MainWindow *mwindow) : BC_MenuItem("Play autos", "4", '4')
{ this->mwindow = mwindow; checked = 0; } 
int PlayAutomation::handle_event()
{ 
	mwindow->tracks->toggle_auto_play();
	set_checked(get_checked() ^ 1);
return 0;
}


CameraAutomation::CameraAutomation(MainWindow *mwindow, MainMenu *menu)
 : BC_MenuItem("Camera", "7", '7') 
{ this->mwindow = mwindow; this->menu = menu; checked = 0; } 
int CameraAutomation::handle_event()
{ 
	set_checked(get_checked() ^ 1);
	menu->set_show_autos();
return 0;
}


ProjectAutomation::ProjectAutomation(MainWindow *mwindow, MainMenu *menu)
 : BC_MenuItem("Projector", "8", '8') 
{ this->mwindow = mwindow; this->menu = menu; checked = 0; } 
int ProjectAutomation::handle_event()
{
	set_checked(get_checked() ^ 1);
	menu->set_show_autos();
return 0;
}


ShowRenderedOutput::ShowRenderedOutput(MainWindow *mwindow) : BC_MenuItem("Draw Output", "6", '6')
{ this->mwindow = mwindow; set_checked(0); } 
int ShowRenderedOutput::handle_event()
{
	mwindow->tracks->set_draw_output();
	if(mwindow->tracks->show_output) set_text("Draw Tracks"); else set_text("Draw Output");
return 0;
}


MuteAutomation::MuteAutomation(MainWindow *mwindow) : BC_MenuItem("Mute autos", "5", '5')
{ this->mwindow = mwindow; }
int MuteAutomation::handle_event()
{
	mwindow->tracks->toggle_auto_mute();
	set_checked(mwindow->tracks->auto_conf.mute);
return 0;
}


PanAutomation::PanAutomation(MainWindow *mwindow) : BC_MenuItem("Pan autos", "") 
{ this->mwindow = mwindow; }
int PanAutomation::handle_event()
{
return 0;
}

PanMenu::PanMenu() : BC_SubMenu() {}

PanItem::PanItem(MainWindow *mwindow, char *text, int number)
 : BC_SubMenuItem(text) 
{
	this->mwindow = mwindow; 
	this->number = number; 
	set_checked(0);
}
int PanItem::handle_event()
{
	mwindow->tracks->toggle_auto_pan(number);
	set_checked(get_checked() ^ 1);
return 0;
}

PluginAutomation::PluginAutomation(MainWindow *mwindow) : BC_MenuItem("Plugin autos", "") 
{ this->mwindow = mwindow; }
int PluginAutomation::handle_event()
{
return 0;
}

PluginMenu::PluginMenu() : BC_SubMenu() {}

PluginItem::PluginItem(MainWindow *mwindow, char *text, int number)
 : BC_SubMenuItem(text) 
{
	this->mwindow = mwindow; 
	this->number = number; 
	set_checked(0);
}
int PluginItem::handle_event()
{
	mwindow->tracks->toggle_auto_plugin(number);
	set_checked(get_checked() ^ 1);
return 0;
}
