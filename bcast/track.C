#include <string.h>
#include "assets.h"
#include "autoconf.h"
#include "console.h"
#include "edit.h"
#include "edits.h"
#include "filehtal.h"
#include "floatautos.h"
#include "mainwindow.h"
#include "module.h"
#include "modules.h"
#include "patch.h"
#include "patchbay.h"
#include "toggleautos.h"
#include "track.h"
#include "trackcanvas.h"
#include "tracks.h"
#include "transition.h"
#include "vedit.h"

Track::Track(MainWindow *mwindow, Tracks *tracks) : ListItem<Track>()
{
	this->mwindow = mwindow;
	this->tracks = tracks;
	transition = 0;
}

Track::~Track()
{
	delete play_autos;
	delete mute_autos;
	delete fade_autos;
	for(int i = 0; i < PLUGINS; i++)
		delete plugin_autos[i];
}

int Track::create_objects(int pixel, int flash)
{
	this->pixel = pixel;
	play_autos = new ToggleAutos(this, LTGREY, 1);
	play_autos->create_objects();

	mute_autos = new ToggleAutos(this, LTGREY, -1);
	mute_autos->create_objects();

	for(int i = 0; i < PLUGINS; i++)
	{
		plugin_autos[i] = new FloatAutos(this, LTGREY, -1, 1, AUTOS_VIRTUAL_HEIGHT, 1);
		plugin_autos[i]->create_objects();
	}

	create_derived_objs(flash);
return 0;
}

// ======================================== accounting

int Track::number_of() { return tracks->number_of(this); return 0;
}

Patch* Track::get_patch_of()
{
	return mwindow->patches->number(tracks->number_of(this));
}

Module* Track::get_module_of()
{
	return mwindow->console->modules->module_number(tracks->number_of(this));
}

int Track::track_visible(int x, int w, int y, int h)
{
	if(mwindow->tracks_vertical)
	{
		if(               // top edge of track in view
			(pixel >= x &&
			 pixel < x + w)
			 ||             // bottom edge of track in view
			(pixel + mwindow->zoom_track > x &&
			 pixel + mwindow->zoom_track <= x + w)
			 ||             // view in track
			(x > pixel && x + w < pixel + mwindow->zoom_track)
			)
		{
			return 1;
		}
	}
	else
	{
		if(               // top edge of track in view
			(pixel >= y &&
			 pixel < y + h)
			 ||             // bottom edge of track in view
			(pixel + mwindow->zoom_track > y &&
			 pixel + mwindow->zoom_track <= y + h)
			 ||             // view in track
			(y > pixel && y + h < pixel + mwindow->zoom_track)
			)
		{
			return 1;
		}
	}
	return 0;
return 0;
}

long Track::length()
{
//printf("Track::length 0\n");
	return 0;
}

int Track::save(FileHTAL *htal)
{
	htal->tag.set_title("TRACK");
	
	if(data_type == TRACK_AUDIO)
		htal->tag.set_property("TYPE", "AUDIO");
	else
		htal->tag.set_property("TYPE", "VIDEO");

	htal->append_tag();
	htal->append_newline();

	Patch* patch = get_patch_of();
	patch->save(htal);

	Module* module = get_module_of();
	module->save(htal);

	if(play_autos->total())
	{
		htal->tag.set_title("PLAYAUTOS");
		htal->append_tag();
		play_autos->save(htal);
		htal->tag.set_title("/PLAYAUTOS");
		htal->append_tag();
		htal->append_newline();
	}

	if(mute_autos->total())
	{
		htal->tag.set_title("MUTEAUTOS");
		htal->append_tag();
		mute_autos->save(htal);
		htal->tag.set_title("/MUTEAUTOS");
		htal->append_tag();
		htal->append_newline();
	}

	for(int i = 0; i < PLUGINS; i++)
	{
		if(plugin_autos[i]->total())
		{
			htal->tag.set_title("PLUGINAUTOS");
			htal->tag.set_property("PLUGIN", i);
			htal->append_tag();
			plugin_autos[i]->save(htal);
			htal->tag.set_title("/PLUGINAUTOS");
			htal->append_tag();
			htal->append_newline();
		}
	}

	if(fade_autos->total())
	{
		htal->tag.set_title("FADEAUTOS");
		htal->append_tag();
		fade_autos->save(htal);
		htal->tag.set_title("/FADEAUTOS");
		htal->append_tag();
		htal->append_newline();
	}

	save_derived(htal);

	htal->tag.set_title("/TRACK");
	htal->append_tag();
	htal->append_newline();
	htal->append_newline();
return 0;
}


int Track::load_automation(FileHTAL *htal)
{
	load(htal, 0, 1, 0);
return 0;
}

int Track::load_edits(FileHTAL *htal)
{
	load(htal, 0, 0, 1);
return 0;
}

int Track::load(FileHTAL *htal, int track_offset, int automation_only, int edits_only)
{
	int result = 0;
	int current_channel = 0;
	int load_all = 0;
	int plugin = 0;
	if(!(automation_only || edits_only)) { load_all = 1; }

	do{
		result = htal->read_tag();

		if(!result)
		{
			if(htal->tag.title_is("/TRACK"))
			{
				result = 1;
			}
			else
			if(htal->tag.title_is("PATCH"))
			{
				if(load_all)
				{
					Patch* patch = get_patch_of();
					patch->load(htal);
				}
			}
			else
			if(htal->tag.title_is("MODULE"))
			{
				if(load_all)
				{
					Module* module = get_module_of();
					module->load(htal, track_offset);
				}
			}
			else
			if(htal->tag.title_is("PLAYAUTOS"))
			{
				play_autos->load(htal, "/PLAYAUTOS");
			}
			else
			if(htal->tag.title_is("PLUGINAUTOS"))
			{
				plugin = htal->tag.get_property("PLUGIN", plugin);
				plugin_autos[plugin]->load(htal, "/PLUGINAUTOS");
			}
			else
			if(htal->tag.title_is("MUTEAUTOS"))
			{
				mute_autos->load(htal, "/MUTEAUTOS");
			}
			else
			if(htal->tag.title_is("FADEAUTOS"))
			{
				fade_autos->load(htal, "/FADEAUTOS");
			}
			else
			if(htal->tag.title_is("EDITS"))
			{
				if(load_all || edits_only) edits->load(htal, track_offset);
			}
			else
			load_derived(htal, automation_only, edits_only, load_all, current_channel);
		}
	}while(!result);
	
	if(mwindow->gui)
	{
		draw_clear(0, tracks->canvas->w, 0, tracks->canvas->h, 0);
		draw(0, tracks->canvas->w, 0, tracks->canvas->h, 1);
	}
return 0;
}


int Track::render_init(int realtime_sched, int duplicate, long position)
{
	Edit *current;

// Store transition pointer for comparison with next render.
	transition = get_transition(position);
	for(current = edits->first; current; current = NEXT)
	{
		if(current->transition)
		{
			current->transition->render_init(realtime_sched, duplicate);
		}
	}
	return 0;
return 0;
}

int Track::render_stop(int duplicate)
{
	Edit *current;

	for(current = edits->first; current; current = NEXT)
	{
		if(current->transition)
		{
			current->transition->render_stop(duplicate);
		}
	}
	return 0;
return 0;
}


// ========================================== drawing




int Track::draw(int x, int w, int y, int h, int flash)
{       // make sure this track is visible
	if(tracks->canvas && track_visible(x, w, y, h))
	{
		draw_derived(x, w, y, h, flash);

		for(Edit* current = edits->first; current; current = NEXT)
		{
			current->draw(flash, pixel + mwindow->zoom_track / 2, x, w, y, h, 0);
		}
	}
return 0;
}

int Track::draw_clear(int x, int w, int y, int h, int flash)
{       // make sure this track is visible
	if(tracks->canvas && track_visible(x, w, y, h))
	{
		if(mwindow->tracks_vertical)
		tracks->canvas->clear_box(pixel, y, mwindow->zoom_track, h);
		else
		tracks->canvas->clear_box(x, pixel, w, mwindow->zoom_track);

		if(flash)
		{
// boundaries are the same for vertical
			tracks->canvas->flash(x, w, y, h);
		}
	}
return 0;
}

int Track::draw_autos(AutoConf *auto_conf)
{
	int i;
	
	if(tracks->canvas && track_visible(0, tracks->canvas->w, 0, tracks->canvas->h))
	{
		float view_start, view_units, zoom_units;

		get_dimensions(view_start, view_units, zoom_units);

		if(auto_conf->play) 
			play_autos->draw(tracks->canvas, 
							pixel, 
							mwindow->zoom_track, 
							zoom_units, 
							view_start, 
							mwindow->tracks_vertical);

		for(i = 0; i < PLUGINS; i++)
		{
			if(auto_conf->plugin[i])
			{
				plugin_autos[i]->draw(tracks->canvas,
									pixel,
									mwindow->zoom_track,
									zoom_units,
									view_start,
									mwindow->tracks_vertical);
			}
		}
		
		if(auto_conf->play) 
			play_autos->draw(tracks->canvas, 
							pixel, 
							mwindow->zoom_track, 
							zoom_units, 
							view_start, 
							mwindow->tracks_vertical);

		if(auto_conf->mute) 
			mute_autos->draw(tracks->canvas, 
							pixel, 
							mwindow->zoom_track, 
							zoom_units, 
							view_start, 
							mwindow->tracks_vertical);

		if(auto_conf->fade) 
			fade_autos->draw(tracks->canvas, 
							pixel, 
							mwindow->zoom_track, 
							zoom_units, 
							view_start, 
							mwindow->tracks_vertical);

		draw_autos_derived(view_start, zoom_units, auto_conf);
	}
return 0;
}

int Track::draw_floating_autos(AutoConf *auto_conf, int flash)
{
	int i;
	
	if(tracks->canvas && track_visible(0, tracks->canvas->w, 0, tracks->canvas->h))
	{
		float view_start, view_units, zoom_units;

		get_dimensions(view_start, view_units, zoom_units);

		if(auto_conf->play) 
			play_autos->draw_floating_autos(tracks->canvas, 
								pixel, 
								mwindow->zoom_track, 
								zoom_units, 
								view_start, 
								mwindow->tracks_vertical, 
								flash);

		for(i = 0; i < PLUGINS; i++)
		{
			if(auto_conf->plugin[i])
			{
				plugin_autos[i]->draw_floating_autos(tracks->canvas,
									pixel,
									mwindow->zoom_track,
									zoom_units,
									view_start,
									mwindow->tracks_vertical,
									flash);
			}
		}
		
		if(auto_conf->mute) 
			mute_autos->draw_floating_autos(tracks->canvas, 
								pixel, 
								mwindow->zoom_track, 
								zoom_units, 
								view_start, 
								mwindow->tracks_vertical, 
								flash);
		if(auto_conf->fade) 
			fade_autos->draw_floating_autos(tracks->canvas, 
								pixel, 
								mwindow->zoom_track, 
								zoom_units, 
								view_start, 
								mwindow->tracks_vertical, 
								flash);
	
		draw_floating_autos_derived(view_start, zoom_units, auto_conf, flash);
	}
return 0;
}


int Track::draw_handles()
{
	if(tracks->canvas && track_visible(0, tracks->canvas->w, 0, tracks->canvas->h))
	{
		float view_start, view_units, zoom_units;
		int view_pixels = tracks->view_pixels();

		get_dimensions(view_start, view_units, zoom_units);

		for(Edit* current = edits->first; current; current = NEXT)
		{
			current->draw_handles(tracks->canvas, view_start, view_units, zoom_units, view_pixels, pixel + mwindow->zoom_track / 2);
		}
	}
return 0;
}

int Track::draw_titles()
{
	if(tracks->canvas && track_visible(0, tracks->canvas->w, 0, tracks->canvas->h))
	{
		float view_start, view_units, zoom_units;
		int view_pixels = mwindow->tracks->view_pixels();

		get_dimensions(view_start, view_units, zoom_units);
		for(Edit* current = edits->first; current; current = NEXT)
		{
			current->draw_titles(tracks->canvas, view_start, zoom_units, view_pixels, pixel + mwindow->zoom_track / 2);
		}
	}
return 0;
}




// ================================================= editing

int Track::select_auto(AutoConf *auto_conf, int cursor_x, int cursor_y)
{
	int result = 0, i;

	Patch *patch = get_patch_of();

	if(patch->record)
	{
		float view_start, view_units, zoom_units;

		get_dimensions(view_start, view_units, zoom_units);

		if(auto_conf->play) 
			result = play_autos->select_auto(tracks->canvas, 
											pixel, 
											mwindow->zoom_track, 
											zoom_units, 
											view_start, 
											cursor_x, 
											cursor_y, 
											mwindow->tracks_vertical);

		for(i = 0; i < PLUGINS; i++)
		{
			if(auto_conf->plugin[i]) 
				result = plugin_autos[i]->select_auto(tracks->canvas, 
												pixel, 
												mwindow->zoom_track, 
												zoom_units, 
												view_start, 
												cursor_x, 
												cursor_y, 
												mwindow->tracks_vertical);
		}

		if(auto_conf->mute) 
			result = mute_autos->select_auto(tracks->canvas, 
											pixel, 
											mwindow->zoom_track, 
											zoom_units, 
											view_start, 
											cursor_x, 
											cursor_y, 
											mwindow->tracks_vertical);

		if(!result && auto_conf->fade) 
			result = fade_autos->select_auto(tracks->canvas, 
											pixel, 
											mwindow->zoom_track, 
											zoom_units, 
											view_start, 
											cursor_x, 
											cursor_y, 
											mwindow->tracks_vertical);

		if(!result) result = select_auto_derived(zoom_units, 
											view_start, 
											auto_conf, 
											cursor_x, 
											cursor_y);
	}

	return result;
return 0;
}

int Track::move_auto(AutoConf *auto_conf, int cursor_x, int cursor_y, int shift_down)
{
	int result, i;
	result = 0;

//	float view_start = (float)mwindow->view_start;
//	float view_units = (float)tracks->view_samples();
//	float zoom_units = mwindow->zoom_sample;
//// need samples for proper cursor displaying
// the cursor doesn't display when moving autos, buddy

	float view_start, view_units, zoom_units;
	get_dimensions(view_start, view_units, zoom_units);

	if(auto_conf->play) 
		result = play_autos->move_auto(tracks->canvas, 
										pixel, 
										mwindow->zoom_track, 
										zoom_units, 
										view_start, 
										cursor_x, 
										cursor_y, 
										shift_down, 
										mwindow->tracks_vertical);

	for(i = 0; i < PLUGINS; i++)
	{
		if(auto_conf->plugin[i]) 
			result = plugin_autos[i]->move_auto(tracks->canvas, 
											pixel, 
											mwindow->zoom_track, 
											zoom_units, 
											view_start, 
											cursor_x, 
											cursor_y, 
											shift_down, 
											mwindow->tracks_vertical);
	}

	if(auto_conf->mute) 
		result = mute_autos->move_auto(tracks->canvas, 
										pixel, 
										mwindow->zoom_track, 
										zoom_units, 
										view_start, 
										cursor_x, 
										cursor_y, 
										shift_down, 
										mwindow->tracks_vertical);

	if(!result && auto_conf->fade) 
		result = fade_autos->move_auto(tracks->canvas, 
									pixel, 
									mwindow->zoom_track, 
									zoom_units, 
									view_start,
									cursor_x, 
									cursor_y, 
									shift_down, 
									mwindow->tracks_vertical);

	if(!result) result = move_auto_derived(zoom_units, 
									view_start, 
									auto_conf, 
									cursor_x, 
									cursor_y, 
									shift_down);

	return result;
return 0;
}

int Track::release_auto()
{
	int result, i;
	result = 0;

	result = play_autos->release_auto();
	for(i = 0; i < PLUGINS && !result; i++)
	{
		result = plugin_autos[i]->release_auto();
	}
	if(!result) result = mute_autos->release_auto();
	if(!result) result = fade_autos->release_auto();
	if(!result) result = release_auto_derived();		

	return result;
return 0;
}

int Track::copy_automation(AutoConf *auto_conf, long selectionstart, long selectionend, FileHTAL *htal)
{
// used for copying automation alone and for copying everything with fake auto_conf
	int i;

	Patch *patch = get_patch_of();
	if(patch->record)
	{
		samples_to_units(selectionstart, selectionend);

		if(auto_conf->play)
		{
			htal->tag.set_title("PLAYAUTOS");
			htal->append_tag();

			play_autos->copy(selectionstart, 
						selectionend, 
						htal, 
						1);

			htal->tag.set_title("/PLAYAUTOS");
			htal->append_tag();
			htal->append_newline();
		}

		for(i = 0; i < PLUGINS; i++)
		{
			if(auto_conf->plugin[i])
			{
				htal->tag.set_title("PLUGINAUTOS");
				htal->tag.set_property("PLUGIN", i);
				htal->append_tag();

				plugin_autos[i]->copy(selectionstart, 
							selectionend, 
							htal, 
							1);

				htal->tag.set_title("/PLUGINAUTOS");
				htal->append_tag();
				htal->append_newline();
			}
		}

		if(auto_conf->mute)
		{
			htal->tag.set_title("MUTEAUTOS");
			htal->append_tag();

			mute_autos->copy(selectionstart, 
						selectionend, 
						htal, 
						1);

			htal->tag.set_title("/MUTEAUTOS");
			htal->append_tag();
			htal->append_newline();
		}

		if(auto_conf->fade)
		{
			htal->tag.set_title("FADEAUTOS");
			htal->append_tag();

			fade_autos->copy(selectionstart, 
						selectionend, 
						htal, 
						1);

			htal->tag.set_title("/FADEAUTOS");
			htal->append_tag();
			htal->append_newline();
		}

		copy_automation_derived(auto_conf, selectionstart, selectionend, htal);
	}
return 0;
}

int Track::paste_automation(long selectionstart, long selectionend, long total_length, FileHTAL *htal, int shift_autos)
{
// Only used for pasting automation alone.
// Called by MainWindow::paste_automation directly.
	int result = 0;
	int current_pan = 0;
	int i;
	Patch *patch = get_patch_of();

	if(patch->record)
	{
		samples_to_units(selectionstart, selectionend, total_length);

//printf("selectionstart %d selectionend %d total_length %d\n", selectionstart, selectionend, total_length);
		while(!result)
		{
			result = htal->read_tag();
			
			if(!result)
			{
				if(htal->tag.title_is("/TRACK"))
					result = 1;
				else
				if(htal->tag.title_is("PLAYAUTOS"))
					play_autos->paste(selectionstart, selectionend, total_length, htal, "/PLAYAUTOS", 1, shift_autos);
				else
				if(htal->tag.title_is("PLUGINAUTOS"))
				{
					i = htal->tag.get_property("PLUGIN", i);
					plugin_autos[i]->paste(selectionstart, selectionend, total_length, htal, "/PLUGINAUTOS", 1, shift_autos);
				}
				else
				if(htal->tag.title_is("MUTEAUTOS"))
					mute_autos->paste(selectionstart, selectionend, total_length, htal, "/MUTEAUTOS", 1, shift_autos);
				else
				if(htal->tag.title_is("FADEAUTOS"))
					fade_autos->paste(selectionstart, selectionend, total_length, htal, "/FADEAUTOS", 1, shift_autos);
				else
					paste_automation_derived(selectionstart, selectionend, total_length, htal, shift_autos, current_pan);
			}
		}
		return 1;
	}
	else
	return 0;
return 0;
}

int Track::clear_automation(AutoConf *auto_conf, long selectionstart, long selectionend, int shift_autos)
{
// used by menu option
	int i;
	Patch *patch = get_patch_of();

	if(patch->record)
	{
		samples_to_units(selectionstart, selectionend);

		if(auto_conf->play)
			play_autos->clear(selectionstart, selectionend, 1, shift_autos);

		for(i = 0; i < PLUGINS; i++)
		{
			if(auto_conf->plugin[i])
				plugin_autos[i]->clear(selectionstart, selectionend, 1, shift_autos);
		}

		if(auto_conf->mute)
			mute_autos->clear(selectionstart, selectionend, 1, shift_autos);

		if(auto_conf->fade)
			fade_autos->clear(selectionstart, selectionend, 1, shift_autos);

		clear_automation_derived(auto_conf, selectionstart, selectionend, shift_autos);
		return 1;
	}
	else
	return 0;
return 0;
}

int Track::paste_auto_silence(long start, long end)
{
// used when pasting silence for edits
	int i;
	Patch *patch = get_patch_of();

	if(patch->record)
	{
// unit conversion done in calling routine
		//samples_to_units(start, end);

		fade_autos->paste_silence(start, end);
		play_autos->paste_silence(start, end);
		for(i = 0; i < PLUGINS; i++)
		{
			plugin_autos[i]->paste_silence(start, end);
		}
		mute_autos->paste_silence(start, end);

		paste_auto_silence_derived(start, end);
	}
return 0;
}

int Track::copy(long start, long end, FileHTAL *htal)
{
// Make a copy of the selection in converted units before passing originals to
// copy_automation for a second conversion.
	long start_copy = start, end_copy = end;
	samples_to_units(start_copy, end_copy);
	htal->tag.set_title("TRACK");

	if(data_type == TRACK_AUDIO)
		htal->tag.set_property("TYPE", "AUDIO");
	else
		htal->tag.set_property("TYPE", "VIDEO");

	htal->append_tag();
	htal->append_newline();

	edits->copy(start_copy, end_copy, htal);

	if(mwindow->autos_follow_edits)
	{
		AutoConf auto_conf;
		auto_conf.set_all();
		copy_automation(&auto_conf, start, end, htal);
	}

	copy_derived(start_copy, end_copy, htal);

	htal->tag.set_title("/TRACK");
	htal->append_tag();
	htal->append_newline();
	htal->append_newline();
return 0;
}

int Track::copy_assets(FileHTAL *htal, ArrayList<Asset*> *asset_list, long start, long end)
{
	samples_to_units(start, end);
	Edit *current = edits->editof(start);
	int i, result;

// Search all edits
	while(current && current->startproject < end)
	{
// Check for duplicate assets
		if(current->asset)
		{
			for(i = 0, result = 0; i < asset_list->total; i++)
			{
				if(asset_list->values[i] == current->asset) result = 1;
			}
// append pointer to new asset
			if(!result) asset_list->append(current->asset);
		}

		current = NEXT;
	}
return 0;
}


int Track::paste(long start, long end, long total_length, FileHTAL *htal)
{
// handles automation pasting itself
	int result = 0;
	int current_channel = 0;
	int i = 0;

	samples_to_units(start, end, total_length);

	do{
		result = htal->read_tag();

		if(!result)
		{
			if(!strcmp(htal->tag.get_title(), "/TRACK"))
			{
				result = 1;
			}
			else
			if(!strcmp(htal->tag.get_title(), "EDITS"))
			{
				edits->paste(start, end, total_length, htal);
			}
			else
			if(!strcmp(htal->tag.get_title(), "PLAYAUTOS"))
			{
				play_autos->paste(start, end, total_length, htal, "/PLAYAUTOS", mwindow->autos_follow_edits);
			}
			else
			if(!strcmp(htal->tag.get_title(), "PLUGINAUTOS"))
			{
				i = htal->tag.get_property("PLUGIN", i);
				plugin_autos[i]->paste(start, end, total_length, htal, "/PLUGINAUTOS", mwindow->autos_follow_edits);
			}
			else
			if(!strcmp(htal->tag.get_title(), "MUTEAUTOS"))
			{
				mute_autos->paste(start, end, total_length, htal, "/MUTEAUTOS", mwindow->autos_follow_edits);
			}
			else
			if(!strcmp(htal->tag.get_title(), "FADEAUTOS"))
			{
				fade_autos->paste(start, end, total_length, htal, "/FADEAUTOS", mwindow->autos_follow_edits);
			}
			else
			paste_derived(start, end, total_length, htal, current_channel);
		}
	}while(!result);
	return 0;
return 0;
}

int Track::paste_transition(long startproject, 
				long endproject, 
				Transition *transition)
{
	samples_to_units(startproject, endproject);
	edits->paste_transition(startproject, endproject, transition);
	return 0;
return 0;
}

int Track::clear(long start, long end, int convert_units)
{
	int i;
	Patch *patch = get_patch_of();
	
	if(patch->record)
	{
// Edits::move_auto calls this routine after the units are converted to the track
// format.
		if(convert_units) samples_to_units(start, end);

		if(mwindow->autos_follow_edits)
		{
			play_autos->clear(start, end, 1, 1);
			for(i = 0; i < PLUGINS; i++)
				plugin_autos[i]->clear(start, end, 1, 1);
			mute_autos->clear(start, end, 1, 1);
			fade_autos->clear(start, end, 1, 1);
		}

		edits->clear(start, end);
		clear_derived(start, end);
	}
	return 0;
return 0;
}

int Track::clear_handle(long start, long end)
{
	samples_to_units(start, end);
	edits->clear_handle(start, end);
return 0;
}

int Track::popup_transition(int cursor_x, int cursor_y)
{
	int result = 0;

	if(cursor_y > pixel && cursor_y < pixel + mwindow->zoom_track)
	{
		float view_start, view_units, zoom_units;

		get_dimensions(view_start, view_units, zoom_units);
		for(Edit* current = edits->first; current; current = NEXT)
		{
			result = current->popup_transition(view_start, zoom_units, cursor_x, cursor_y);
		}
	}

	return result;
return 0;
}

int Track::select_handle(int cursor_x, int cursor_y, long &selection)
{
	int result = 0;
	Edit *current_edit;
	Patch *patch = get_patch_of();
	
//	if(patch->record)
//	{
		float view_start, view_units, zoom_units;

		get_dimensions(view_start, view_units, zoom_units);
		for(current_edit = edits->first; current_edit && !result; current_edit = current_edit->next)
		{
			result = current_edit->select_handle(view_start, zoom_units, cursor_x, cursor_y, selection);
		}
//	}

	if(!patch->record && result) result = 3;

	return result;
return 0;
}


int Track::modify_handles(long oldposition, long newposition, int currentend, int handle_mode)
{
	Patch *patch = get_patch_of();

// need samples for proper cursor drawing
	samples_to_units(newposition);
	samples_to_units(oldposition);

	if(patch->record)
	{
		edits->modify_handles(oldposition, newposition, currentend, handle_mode);
	}
return 0;
}


int Track::paste_silence(long start, long end)
{
	samples_to_units(start, end);
	edits->paste_silence(start, end);
	if(mwindow->autos_follow_edits) paste_auto_silence(start, end);
return 0;
}

int Track::select_edit(long cursor_position, int cursor_x, int cursor_y, long &new_start, long &new_end)
{
	int result = 0;
	long pixel1, pixel2;
	long left_unit, right_unit;
	float view_start, view_units, zoom_units;
	
	if(cursor_y > pixel && cursor_y < pixel + mwindow->zoom_track)
	{
		get_dimensions(view_start, view_units, zoom_units);
		for(Edit* edit = edits->first; edit && !result; edit = edit->next)
		{
			edit->get_handle_parameters(pixel1, pixel2, left_unit, right_unit, view_start, zoom_units);
			if(cursor_x > pixel1 && cursor_x < pixel2)
			{
				result = 1;
				new_start = edit->startproject;
				new_end = new_start + edit->length;
				units_to_samples(new_start, new_end);
			}
		}
	}
	return result;
return 0;
}


int Track::feather_edits(long start, long end, long units)
{
	Edit* current_edit;
	Patch *patch = get_patch_of();
	
	if(patch->record)
	{
		samples_to_units(start, end);
// Selected range is in samples but feathering units are specific to the track type

		for(current_edit = edits->editof(start); current_edit; current_edit = current_edit->next)
		{
// start of edit is in range
			if(current_edit->startproject >= start && current_edit->startproject <= end)
			{
				current_edit->feather_left = units;
			}
// end of edit is in range
			if(current_edit->startproject + current_edit->length >= start && current_edit->startproject + current_edit->length <= end)
			{
				current_edit->feather_right = units;
			}
		}
	}
return 0;
}


long Track::get_feather(long selectionstart, long selectionend)
{
// when calling feather edits, want the feather of the edits that are selected
	Edit *current_edit;

	samples_to_units(selectionstart, selectionend);
	for(current_edit = edits->editof(selectionstart); current_edit; current_edit = current_edit->next)
	{
// end of edit is in range
		if(current_edit->startproject + current_edit->length >= selectionstart && current_edit->startproject + current_edit->length <= selectionend)
		{
			return current_edit->feather_right;
		}
// start of edit is in range
		if(current_edit->startproject >= selectionstart && current_edit->startproject <= selectionend)
		{
			return current_edit->feather_left;
		}
	}
	return 0;
}

int Track::scale_time(float rate_scale, int scale_edits, int scale_autos, long start, long end)
{
	Edit *current_edit;
	long current_position = 0;
	int i;

	samples_to_units(start, end);
	for(current_edit = edits->first; current_edit && scale_edits; current_edit = current_edit->next)
	{
		current_edit->length = (long)(current_edit->length * rate_scale + 0.5);
		current_edit->startsource = (long)(current_edit->startsource * rate_scale + 0.5);
		current_edit->startproject = current_position;
		current_position += current_edit->length;
	}

	play_autos->scale_time(rate_scale, scale_edits, scale_autos, start, end);
	for(i = 0; i < PLUGINS; i++)
	{
		plugin_autos[i]->scale_time(rate_scale, scale_edits, scale_autos, start, end);
	}
	mute_autos->scale_time(rate_scale, scale_edits, scale_autos, start, end);
	fade_autos->scale_time(rate_scale, scale_edits, scale_autos, start, end);

	scale_time_derived(rate_scale, scale_edits, scale_autos, start, end);
return 0;
}

Transition* Track::get_transition(long position)
{
	Edit* current_edit;

	for(current_edit = edits->first; current_edit; current_edit = current_edit->next)
	{
		if(current_edit->transition && 
			current_edit->startproject <= position && 
			current_edit->startproject + current_edit->length > position)
		{
			return current_edit->transition;
		}
	}
	return 0;
}

int Track::swap_transitions(int number1, int number2)
{
	Edit *current_edit;
	Transition *transition;

	for(current_edit = edits->first; current_edit; current_edit = current_edit->next)
	{
		transition = current_edit->transition;
		if(transition)
		{
			if(transition->plugin_type == 2)
			{
				if(transition->shared_plugin_location.module == number1)
					transition->shared_plugin_location.module = number2;
				else
				if(transition->shared_plugin_location.module == number2)
					transition->shared_plugin_location.module = number1;
			}

			if(transition->plugin_type == 3)
			{
				if(transition->shared_module_location.module == number1)
					transition->shared_module_location.module = number2;
				else
				if(transition->shared_module_location.module == number2)
					transition->shared_module_location.module = number1;
			}
		}
	}
	return 0;
return 0;
}

int Track::shift_module_pointers(int deleted_track)
{
	Edit *current_edit;
	Transition *transition;

	for(current_edit = edits->first; current_edit; current_edit = current_edit->next)
	{
		transition = current_edit->transition;
		if(transition)
		{
			if(transition->plugin_type == 2)
			{
				if(transition->shared_plugin_location.module > deleted_track)
					transition->shared_plugin_location.module--;
				else
				if(transition->shared_plugin_location.module == deleted_track)
					transition->detach();
			}
			
			if(transition->plugin_type == 3)
			{
				if(transition->shared_module_location.module > deleted_track)
					transition->shared_module_location.module--;
				else
				if(transition->shared_module_location.module == deleted_track)
					transition->detach();
			}
		}
	}
	return 0;
return 0;
}

int Track::test_transition(long current_position)
{
	if(get_transition(current_position) != transition) 
		return 1;
	else
		return 0;
return 0;
}

int Track::playable_edit(long position)
{
	int result = 0;
	for(Edit *current = edits->first; current && !result; current = NEXT)
	{
		if(current->startproject <= position && current->startproject + current->length > position)
		{
			if(current->transition || (current->asset && !current->asset->silence)) result = 1;
		}
	}
	return result;
return 0;
}


int Track::edit_is_interesting(Edit *current, int test_transitions)
{
	return ((test_transitions && current->transition) ||
		(!test_transitions && current->asset && current->asset->silence));
return 0;
}

long Track::edit_change_duration(long input_position, long input_length, int reverse, int test_transitions)
{
	Edit *current;
	long edit_length = input_length;

	if(reverse)
	{
// ================================= Reverse playback
// Get first edit on or after position
		for(current = edits->first; 
			current && current->startproject + current->length <= input_position;
			current = NEXT)
			;

		if(current)
		{
			if(current->startproject > input_position)
			{
// Before first edit
				;
			}
			else
			if(edit_is_interesting(current, test_transitions))
			{
// Over an edit of interest.
				if(input_position - current->startproject < input_length)
					edit_length = input_position - current->startproject + 1;
			}
			else
			{
// Over an edit that isn't of interest.
// Search for next edit of interest.
				for(current = PREVIOUS ; 
					current && 
					current->startproject + current->length > input_position - input_length &&
					!edit_is_interesting(current, test_transitions);
					current = PREVIOUS)
					;

					if(current && 
						edit_is_interesting(current, test_transitions) &&
						current->startproject + current->length > input_position - input_length)
                    	edit_length = input_position - current->startproject - current->length + 1;
			}
		}
		else
		{
// Not over an edit.  Try the last edit.
			current = edits->last;
			if(current && 
				((test_transitions && current->transition) ||
				(!test_transitions && current->asset && !current->asset->silence)))
				edit_length = input_position - edits->last->startproject - edits->last->length + 1;
		}
	}
	else
	{
// =================================== forward playback
// Get first edit on or before position
		for(current = edits->last; 
			current && current->startproject > input_position;
			current = PREVIOUS)
			;

		if(current)
		{
			if(current->startproject + current->length <= input_position)
			{
// Beyond last edit.
				;
			}
			else
			if(edit_is_interesting(current, test_transitions))
			{
// Over an edit of interest.
// Next edit is going to require a change.
				if(current->length + current->startproject - input_position < input_length)
					edit_length = current->startproject + current->length - input_position;
			}
			else
			{
// Over an edit that isn't of interest.
// Search for next edit of interest.
				for(current = NEXT ; 
					current && 
					current->startproject < input_position + input_length &&
					!edit_is_interesting(current, test_transitions);
					current = NEXT)
					;

					if(current && 
						edit_is_interesting(current, test_transitions) &&
						current->startproject < input_position + input_length) 
						edit_length = current->startproject - input_position;
			}
		}
		else
		{
// Not over an edit.  Try the first edit.
			current = edits->first;
			if(current && 
				((test_transitions && current->transition) ||
				(!test_transitions && current->asset && !current->asset->silence)))
				edit_length = edits->first->startproject - input_position;
		}
	}

	if(edit_length < input_length)
		return edit_length;
	else
		return input_length;
}

int Track::purge_asset(Asset *asset)
{
	Edit* current_edit;
	int result = 0;

	for(current_edit = edits->first; current_edit; current_edit = current_edit->next)
	{
		if(current_edit->asset == asset)
		{
			current_edit->asset = mwindow->assets->get_asset(SILENCE);
			result++;
		}
	}
	return result;
return 0;
}

int Track::asset_used(Asset *asset)
{
	Edit* current_edit;
	int result = 0;

	for(current_edit = edits->first; current_edit; current_edit = current_edit->next)
	{
		if(current_edit->asset == asset)
		{
			result++;
		}
	}
	return result;
return 0;
}

int Track::automation_is_used(long start, long end)
{
	int result = 0, i;
	Auto *first_auto = 0, *last_auto = 0;

	if(!get_patch_of()->automate) return 0;

// Automatically disqualified if a plugin is attached
// 	for(i = 0; i < PLUGINS; i++)
// 	{
// 		if(plugin_autos[i]->total()) result = 1;
// 	}

	if(fade_autos->automation_is_constant(start, end, &first_auto, &last_auto))
	{
		if(fade_autos->get_automation_constant(start, end, &first_auto, &last_auto) != 0)
			result = 1;
	}
	else
	result = 1;

	if(mute_autos->total() ||
		automation_is_used_derived(start, end))
		result = 1;

	return result;
return 0;
}

int Track::direct_copy_possible(long start, long end)
{
	int result = 1;
	VEdit *playable_edit = (VEdit*)edits->first;

// No automation must be present in the track
	if(result)
	{
		if(automation_is_used(start, end))
			result = 0;
	}

// No console routing must be present
	if(result)
	{
		if(get_module_of()->console_routing_used()) result = 0;
	}

// No console adjustments
	if(result &&
		get_module_of()->console_adjusting_used()) result = 0;

// No translation
	if(result && playable_edit && 
		(playable_edit->center_x != 0 ||
		playable_edit->center_y != 0 ||
		playable_edit->center_z != 1)) result = 0;

	return result;
return 0;
}

int Track::samples_to_units(long &start, long &end)
{
	samples_to_units(start);
	samples_to_units(end);
return 0;
}

int Track::samples_to_units(long &start, long &end, long &total_length)
{
	samples_to_units(start);
	samples_to_units(end);
	samples_to_units(total_length);
return 0;
}

long Track::units_to_samples(long &units)
{
	units = 0;
	return 0;
}

int Track::units_to_samples(long &start, long &end)
{
	units_to_samples(start);
	units_to_samples(end);
return 0;
}

int Track::dump()
{
	for(Edit* current = edits->first; current; current = NEXT)
	{
		//printf(" path %s", current->asset->path);
		//printf(" startproject %ld startsource %ld length %ld\n", current->startproject, current->startsource, current->length);
		current->dump();
	}
	play_autos->dump();
	mute_autos->dump();
	fade_autos->dump();
return 0;
}



