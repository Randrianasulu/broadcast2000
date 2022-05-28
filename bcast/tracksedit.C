#include <string.h>
#include "assets.h"
#include "atrack.h"
#include "console.h"
#include "aedits.h"
#include "edit.h"
#include "edits.h"
#include "filehtal.h"
#include "mainundo.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "module.h"
#include "modules.h"
#include "patch.h"
#include "patchbay.h"
#include "timebar.h"
#include "trackcanvas.h"
#include "tracks.h"
#include "trackscroll.h"
#include "transition.h"
#include "vtrack.h"

int Tracks::import_audio_track(long length, int channel, Asset *asset)
{
	if(mwindow->gui)
	{
		hide_overlays(0);
	}

	add_audio_track(0);
	((ATrack*)last)->paste_output(0, length, 0, length, channel, asset);

	if(mwindow->gui)
	{
		if(mwindow->tracks_vertical)
		last->draw(0, vertical_pixels(), 0, view_pixels(), 0);
		else
		last->draw(0, view_pixels(), 0, vertical_pixels(), 0);

		mwindow->gui->trackscroll->update();
		show_overlays(1);
	}
return 0;
}

int Tracks::import_video_track(long length, int layer, Asset *asset)
{
	if(mwindow->gui)
	{
		hide_overlays(0);
	}

	add_video_track(0);
	((VTrack*)last)->paste_output(0, length, 0, length, layer, asset);

	if(mwindow->gui)
	{
		if(mwindow->tracks_vertical)
		last->draw(0, vertical_pixels(), 0, view_pixels(), 0);
		else
		last->draw(0, view_pixels(), 0, vertical_pixels(), 0);

		mwindow->gui->trackscroll->update();
		show_overlays(1);
	}
return 0;
}

int Tracks::add_audio_track(int flash)
{
	int pixel;
	ATrack* current;
	
	if(last)   // set new y
	{        // additional track
		pixel = last->pixel;
		pixel += mwindow->zoom_track;
	}
	else
	{        // first track
		pixel = 0 - view_start;
	}

	mwindow->patches->add_track(view_start, "Audio", TRACK_AUDIO);
	append(current = new ATrack(mwindow, this));
	current->create_objects(pixel, flash);
	mwindow->console->add_audio_track();
return 0;
}

int Tracks::add_video_track(int flash)
{
	int pixel;
	VTrack* current;
	
	if(last)
	{
		pixel = last->pixel;
		pixel += mwindow->zoom_track;
	}
	else
	{
		pixel = 0 - view_start;
	}
	
	mwindow->patches->add_track(view_start, "Video", TRACK_VIDEO);
	append(current = new VTrack(mwindow, this));
	current->create_objects(pixel, flash);
	mwindow->console->add_video_track();
return 0;
}

int Tracks::delete_track()
{
	if(last)
	{
		delete_track(last);
	}
return 0;
}

int Tracks::delete_tracks()
{
	int result = 1, total_deleted = 0;

	Patch *patch, *next_patch;
	Track *track, *next_track, *shifted_track;
	Module *module, *next_module, *shifted_module;
	int deleted_number;
	
	while(result)
	{
// keep deleting until all the recordable tracks are gone
		result = 0;
		for(patch = mwindow->patches->first, track = first, module = mwindow->console->modules->first; 
			patch && !result; patch = next_patch, track = next_track, module = next_module)
		{
			next_patch = patch->next;
			next_track = track->next;
			next_module = module->next;
			
			if(patch->record)
			{
				deleted_number = number_of(track);
// Delete the track.
				delete patch;
				delete track;
				delete module;

// Shift all the plugin pointers.
				for(shifted_track = next_track, shifted_module = next_module;
					shifted_track && shifted_module;
					shifted_track = shifted_track->next, shifted_module = shifted_module->next)
				{
					shifted_track->shift_module_pointers(deleted_number);
					shifted_module->shift_module_pointers(deleted_number);
				}
				result = 1;
				total_deleted++;
			}
		}
	}

	if(total_deleted)
	{
		if(mwindow->gui)
		{
			redo_pixels();
			mwindow->patches->redo_pixels(view_start);
			mwindow->console->redo_pixels();
			draw();
			show_overlays(1);
		}

		mwindow->changes_made = 1;
	}
return 0;
}

int Tracks::concatenate_tracks()
{
	Track *output_track, *first_output_track, *input_track;
	Patch *output_patch, *first_output_patch, *input_patch;
	int i, data_type = TRACK_AUDIO;
	long output_start;
	long input_end;
	FileHTAL *clipboard;
	int result = 0;

// Relocate tracks
	for(i = 0; i < 2; i++)
	{
// Get first output track
		for(output_track = first, output_patch = mwindow->patches->first; 
			output_track; 
			output_track = output_track->next, output_patch = output_patch->next)
			if(output_track->data_type == data_type && 
				output_patch->record) break;

		first_output_track = output_track;
		first_output_patch = output_patch;

// Get first input track
		for(input_track = first, input_patch = mwindow->patches->first;
			input_track;
			input_track = input_track->next, input_patch = input_patch->next)
			if(input_track->data_type == data_type &&
				input_patch->play && !input_patch->record) break;

		if(output_track && input_track)
		{
// Transfer input track to end of output track one at a time
			while(input_track)
			{
				clipboard = new FileHTAL;
				output_start = output_track->length();
				input_end = input_track->length();
				input_track->copy(0, input_end, clipboard);
				clipboard->terminate_string();
				clipboard->rewind();
				output_track->paste(output_start, output_start, input_end, clipboard);
				delete clipboard;


				for(input_track = input_track->next, input_patch = input_patch->next; 
					input_track; 
					input_track = input_track->next, input_patch = input_patch->next)
					if(input_track->data_type == data_type && !input_patch->record && input_patch->play) break;

				for(output_track = output_track->next, output_patch = output_patch->next; 
					output_track; 
					output_track = output_track->next, output_patch = output_patch->next)
					if(output_track->data_type == data_type && output_patch->record) break;

				if(!output_track)
				{
					output_track = first_output_track;
					output_patch = first_output_patch;
				}
			}
			result = 1;
		}
		if(data_type == TRACK_AUDIO) data_type = TRACK_VIDEO;
	}
	if(result)
	{
		mwindow->draw();
		mwindow->changes_made = 1;
	}

	return 0;
return 0;
}

int Tracks::delete_audio_track()
{
	Track *current;

	for(current = last; current && current->data_type != TRACK_AUDIO; current = PREVIOUS)
	{
		;
	}

	if(current) delete_track(current);
return 0;
}

int Tracks::delete_video_track()
{
	Track *current;

	for(current = last; current && current->data_type != TRACK_VIDEO; current = PREVIOUS)
	{
		;
	}

	if(current) delete_track(current);
return 0;
}

int Tracks::delete_track(Track* track)
{
	if(mwindow->gui) hide_overlays(0);
	mwindow->changes_made = 1;

	int pixel = last->pixel;
	Patch* patch = track->get_patch_of();
	Module* module = track->get_module_of();

	delete patch;
	delete track;
	delete module;

	if(mwindow->gui)
	{
		redo_pixels();
		mwindow->patches->redo_pixels(view_start);
		mwindow->console->redo_pixels();

		if(mwindow->tracks_vertical)
			draw(pixel, mwindow->zoom_track, 0, canvas->h);
		else
			draw(0, canvas->w, pixel, mwindow->zoom_track);

		show_overlays(1);
	}
	return 0;
return 0;
}

int Tracks::delete_all(int flash)
{
	while(last) delete_track();
	
	if(flash) draw(flash);
	return 0;
return 0;
}

int Tracks::move_tracks_up()
{
	Patch *patch, *next_patch;
	Track *track, *next_track;
	Module *module, *next_module;
	int result = 0;

	for(patch = mwindow->patches->first, track = first, module = mwindow->console->modules->first;
		patch; patch = next_patch, track = next_track, module = next_module)
	{
		next_patch = patch->next;
		next_track = track->next;
		next_module = module->next;

		if(patch->record)
		{
			if(track->previous)
			{
				swap_transitions(track->previous, track);
				mwindow->console->modules->swap_plugins(module->previous, module);

				swap(track->previous, track);
				mwindow->patches->swap(patch->previous, patch);
				mwindow->console->modules->swap(module->previous, module);
				result = 1;
			}
		}
	}

	if(result)
	{
		if(mwindow->gui) 
		{
			redo_pixels();
			mwindow->patches->redo_pixels(view_start);
			mwindow->console->redo_pixels();
			draw();
			show_overlays(1);
		}

		mwindow->changes_made = 1;
	}
	return 0;
return 0;
}

int Tracks::move_tracks_down()
{
	Patch *patch, *previous_patch;
	Track *track, *previous_track;
	Module *module, *previous_module;
	int result = 0;
	
	for(patch = mwindow->patches->last, track = last, module = mwindow->console->modules->last;
		patch; patch = previous_patch, track = previous_track, module = previous_module)
	{
		previous_patch = patch->previous;
		previous_track = track->previous;
		previous_module = module->previous;

		if(patch->record)
		{
			if(track->next)
			{
				swap_transitions(track, track->next);
				mwindow->console->modules->swap_plugins(module, module->next);

				swap(track, track->next);
				mwindow->patches->swap(patch, patch->next);
				mwindow->console->modules->swap(module, module->next);
				result = 1;
			}
		}
	}
	
	if(result)
	{
		if(mwindow->gui) 
		{
			hide_overlays(0);
			redo_pixels();
			mwindow->patches->redo_pixels(view_start);
			mwindow->console->redo_pixels();
			draw();
			show_overlays(1);
		}

		mwindow->changes_made = 1;
	}
return 0;
}

int Tracks::swap_transitions(Track *track1, Track *track2)
{
	int number1 = number_of(track1) - 1;
	int number2 = number_of(track2) - 1;
	Track* current = first;

//printf("Tracks::swap_transitions %d %d\n", number1, number2);
	for( ; current; current = current->next)
	{
		current->swap_transitions(number1, number2);
	}
	return 0;
return 0;
}


// =========================================== EDL editing

int Tracks::cut(long start, long end) { return 0;
}

int Tracks::copy(long start, long end)
{
// nothing selected
	if(start == end) return 1;

	FileHTAL htal;
	
	htal.tag.set_title("HTAL");
	htal.append_tag();
	htal.append_newline();
	
	htal.tag.set_title("CLIPBOARD");
	htal.tag.set_property("SAMPLES", end - start);
	htal.append_tag();
	htal.append_newline();
	htal.append_newline();

	copy_assets(&htal, start, end, 0);
	
	mwindow->timebar->copy(start, end, &htal);

	Track* current_track;
	Patch* current_patch;
	
	for(current_track = first, current_patch = mwindow->patches->first; current_track; current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record)
		{
			current_track->copy(start, end, &htal);
		}
	}
	
	htal.tag.set_title("/CLIPBOARD");
	htal.append_tag();
	htal.append_newline();

	htal.tag.set_title("/HTAL");
	htal.append_tag();
	htal.append_newline();
	htal.terminate_string();

	if(mwindow->gui) mwindow->gui->to_clipboard(htal.string);
	return 0;
return 0;
}

int Tracks::copy_assets(FileHTAL *htal, long start, long end, int all)
{
	ArrayList<Asset*> asset_list;
	Track* current_track;
	Patch* current_patch;

	htal->tag.set_title("ASSETS");
	htal->append_tag();
	htal->append_newline();

	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(all || current_patch->record)
		{
			current_track->copy_assets(htal, &asset_list, start, end);
		}
	}

	for(int i = 0; i < asset_list.total; i++)
	{
		asset_list.values[i]->write(mwindow, htal, 0, 0);
	}

	htal->tag.set_title("/ASSETS");
	htal->append_tag();
	htal->append_newline();
	return 0;
return 0;
}




int Tracks::paste(long start, long end)
{
	FileHTAL htal;

// load from the clipboard
	{
		char *string;
		string = new char[mwindow->gui->clipboard_len() + 1];
		mwindow->gui->from_clipboard(string, mwindow->gui->clipboard_len() + 1);

		htal.read_from_string(string);
		delete [] string;
	}

	int result = 0;

// scan file for HTAL block	
	do{
		result = htal.read_tag();
	}while(!result && strcmp(htal.tag.get_title(), "HTAL"));

// scan file for clipboard	
	if(!result)
	do{
		result = htal.read_tag();
	}while(!result && strcmp(htal.tag.get_title(), "CLIPBOARD"));

	long total_length = 0;
	if(!result)
	{
		total_length = htal.tag.get_property("SAMPLES", (long)0);
	}

	Track *current_video_track = first;
	Track *current_audio_track = first;
	Patch *current_video_patch = mwindow->patches->first;
	Patch *current_audio_patch = mwindow->patches->first;
	char track_type[1024];
	sprintf(track_type, "AUDIO");

	while(!result)
	{
		result = htal.read_tag();

		if(!result)
		{
			if(!strcmp(htal.tag.get_title(), "/CLIPBOARD"))
			{
				result = 1;
			}
			else
			if(!strcmp(htal.tag.get_title(), "ASSETS"))
			{
				paste_assets(&htal);
			}
			else
			if(!strcmp(htal.tag.get_title(), "LABELS"))
			{
				mwindow->timebar->paste(start, end, total_length, &htal);
			}
			else
			if(!strcmp(htal.tag.get_title(), "TRACK"))
			{
				htal.tag.get_property("TYPE", track_type);
				if(!strcmp(track_type, "AUDIO"))
				{
// get next recordable audio track
					while(current_audio_patch && current_audio_track &&
						(!current_audio_patch->record || 
						current_audio_track->data_type != TRACK_AUDIO))
					{
						current_audio_patch = current_audio_patch->next;
						current_audio_track = current_audio_track->next;
					}
					
					if(current_audio_track)
					{
						current_audio_track->paste(start, end, total_length, &htal);
						current_audio_patch = current_audio_patch->next;
						current_audio_track = current_audio_track->next;
					}
				}
				else
				if(!strcmp(track_type, "VIDEO") && current_video_track)
				{
// get a recordable video track
					while(current_video_patch && current_video_track &&
						(!current_video_patch->record || 
						current_video_track->data_type != TRACK_VIDEO))
					{
						current_video_patch = current_video_patch->next;
						current_video_track = current_video_track->next;
					}
					
					if(current_video_track)
					{
						current_video_track->paste(start, end, total_length, &htal);
						current_video_patch = current_video_patch->next;
						current_video_track = current_video_track->next;
					}
				}
			}      // end TRACK tag
		}
	}

// fill remaining tracks with silence
	while(current_audio_track || current_video_track)
	{
		if(current_audio_track)
		{
			if(current_audio_patch->record && current_audio_track->data_type == TRACK_AUDIO)
			{
				current_audio_track->paste_silence(start, start + total_length);
			}

			current_audio_patch = current_audio_patch->next;
			current_audio_track = current_audio_track->next;
		}

		if(current_video_track)
		{
			if(current_video_patch->record && current_video_track->data_type == TRACK_VIDEO)
			{
				current_video_track->paste_silence(start, start + total_length);
			}

			current_video_patch = current_video_patch->next;
			current_video_track = current_video_track->next;
		}
	}

	mwindow->draw();
	mwindow->set_selection(start, start + total_length);
	mwindow->changes_made = 1;
	return result;
return 0;
}

int Tracks::paste_assets(FileHTAL *htal)
{
	int result = 0;

	while(!result)
	{
		result = htal->read_tag();
		if(!result)
		{
			if(htal->tag.title_is("/ASSETS"))
			{
				result = 1;
			}
			else
			if(htal->tag.title_is("ASSET"))
			{
				char *path = htal->tag.get_property("SRC");
				Asset new_asset(path ? path : SILENCE);
				new_asset.read(mwindow, htal);
				mwindow->assets->update(&new_asset);
			}
		}
	}
return 0;
}

int Tracks::paste_output(long startproject, 
				long endproject, 
				long startsource_sample, 
				long endsource_sample, 
				long startsource_frame, 
				long endsource_frame, 
				Asset *asset)
{
	Track *current_track;
	Patch *current_patch;
	int result, channel, layer;

	result = 0;
	channel = 0;
	layer = 0;

	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch;
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record)
		{
			if(current_track->data_type == TRACK_AUDIO)
			{
				if(channel < asset->channels && asset->audio_data)
				{
					((ATrack*)current_track)->paste_output(startproject, 
												endproject, 
												startsource_sample, 
												endsource_sample, 
												channel, 
												asset);
					channel++;
				}
// inserting silence into extra tracks sucked
				//else { current_track->paste_silence(startproject, endsource - startsource); }
			}
			else
			if(current_track->data_type == TRACK_VIDEO)
			{
				if(layer < asset->layers && asset->video_data)
				{
					long start = startproject, end = endproject;
					current_track->samples_to_units(start);
					current_track->samples_to_units(end);

					((VTrack*)current_track)->paste_output(start, 
												end, 
												startsource_frame, 
												endsource_frame, 
												layer, 
												asset);
					layer++;
				}
// inserting silence into extra tracks sucked
				//else { current_track->paste_silence(startproject, endsource - startsource); }
			}
		}
	}
return 0;
}

int Tracks::paste_transition(long startproject, 
				long endproject, 
				Transition *transition)
{
	Track *current_track;
	Patch *current_patch;

	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch;
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record)
		{
			if((current_track->data_type == TRACK_AUDIO && transition->audio) ||
				(current_track->data_type == TRACK_VIDEO && transition->video))
			{
					current_track->paste_transition(startproject, 
												endproject, 
												transition);
			}
		}
	}
	
	return 0;
return 0;
}

int Tracks::clear(long start, long end)
{
	Track *current_track;
	Patch *current_patch;

	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record) { current_track->clear(start, end); }
	}
return 0;
}

int Tracks::copy_automation(long selectionstart, long selectionend, FileHTAL *htal)
{
// called by MainWindow::copy_automation for copying automation alone
	Track* current_track;
	Patch* current_patch;

	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch; current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record)
		{
			htal->tag.set_title("TRACK");
			htal->append_tag();
			htal->append_newline();

			current_track->copy_automation(&auto_conf, selectionstart, selectionend, htal);

			htal->tag.set_title("/TRACK");
			htal->append_tag();
			htal->append_newline();
			htal->append_newline();
		}
	}
return 0;
}

int Tracks::clear_automation(long selectionstart, long selectionend)
{
	Track* current_track;
	Patch* current_patch;
	
	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record) 
		{ 
			current_track->clear_automation(&auto_conf, selectionstart, selectionend, 0); 
		}
	}
return 0;
}

int Tracks::clear_handle(long start, long end)
{
	Track* current_track;
	Patch* current_patch;
	
	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record) { current_track->clear_handle(start, end); }
	}
return 0;
}

int Tracks::paste_silence(long start, long end)
{
	Track* current_track;
	Patch* current_patch;
	
	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record) { current_track->paste_silence(start, end); }
	}
return 0;
}

int Tracks::select_translation(int cursor_x, int cursor_y)
{
// cursor_x is relative to samples
	int result = 0;
	Patch* current_patch;
	Track* current_track;
	
	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && !result; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record) 
			result = current_track->select_translation(cursor_x, cursor_y);
	}
	if(result)
	{
		mwindow->undo->update_undo_edits("Translation", 0);
	}
	return result;
return 0;
}

int Tracks::update_translation(int cursor_x, int cursor_y, int shift_down)
{
	int result = 0;
	for(Track* current = first; current && !result; current = NEXT)
	{
		result = current->update_translation(cursor_x, cursor_y, shift_down);
	}
return 0;
}

int Tracks::end_translation()
{
	Track *current;
	int result = 0;

	for(current = first; current; current = NEXT)
	{
		result = current->end_translation();
	}
	if(result)
	{
		mwindow->changes_made = 1;
		mwindow->undo->update_undo_edits();
	}
return 0;
}

int Tracks::select_handle(int cursor_x, int cursor_y, long &handle_oldposition, long &handle_position, int &handle_pixel)
{
	int center_pixel;
	int result = 0;
	long selection;
	
	if(handles)
	{
		for(Track* current = first; current && !result; current = NEXT) 
		{
			center_pixel = current->pixel + mwindow->zoom_track / 2;

			if(cursor_y > center_pixel - 6 && cursor_y < center_pixel + 6)
				result = current->select_handle(cursor_x, cursor_y, selection);
		}
	}

// Result is 3 if the track was recordable or 1,2 if it wasn't recordable
	if(result) 
	{
// Modify selected region
		result = mwindow->init_handle_selection(selection, cursor_x, result);

		if(result && result != 3)
		{
// not a region selection and a recordable track
			handle_oldposition = selection;
			handle_position = selection;
			handle_pixel = cursor_x;
		}
	}
	
	return result;
return 0;
}

int Tracks::select_auto(int cursor_x, int cursor_y)
{
	int result = 0;
	for(Track* current = first; current && !result; current = NEXT) { result = current->select_auto(&auto_conf, cursor_x, cursor_y); }
	return result;
return 0;
}

int Tracks::move_auto(int cursor_x, int cursor_y, int shift_down)
{
	int result = 0;

	for(Track* current = first; current && !result; current = NEXT) 
	{
		result = current->move_auto(&auto_conf, cursor_x, cursor_y, shift_down); 
	}
return 0;
}

int Tracks::release_auto()
{
	mwindow->changes_made = 1;
// save the before undo
	mwindow->undo->update_undo_automation("Automation", 0);

	hide_overlays(0);

	int result = 0;
	for(Track* current = first; current && !result; current = NEXT) 
	{ 
		result = current->release_auto(); 
	}

// save the after undo
	mwindow->undo->update_undo_automation();
	draw();
	show_overlays(1);
return 0;
}

int Tracks::modify_handles(long oldposition, long newposition, int currentend, int handle_mode)
{
	Track *current;
	Patch *current_patch;

	for(current = first, current_patch = mwindow->patches->first; 
		current && current_patch; 
		current = NEXT, current_patch = current_patch->next)
	{
		if(current_patch->record) { current->modify_handles(oldposition, newposition, currentend, handle_mode); }
	}
return 0;
}

int Tracks::select_edit(long cursor_position, int cursor_x, int cursor_y, long &new_start, long &new_end)
{
	int result = 0;
	for(Track *track = first; track && !result; track = track->next)
	{
		result = track->select_edit(cursor_position, cursor_x, cursor_y, new_start, new_end);
	}
	return result;
return 0;
}

int Tracks::feather_edits(long start, long end, long samples, int audio, int video)
{
	Patch *current_patch;
	Track *current_track;

	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record && 
			((audio && current_track->data_type == TRACK_AUDIO) ||
			(video && current_track->data_type == TRACK_VIDEO)))
		{ 
			current_track->feather_edits(start, end, samples); 
		}
	}
return 0;
}

long Tracks::get_feather(long selectionstart, long selectionend, int audio, int video)
{
	Patch *current_patch;
	Track *current_track;

	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record && 
			((audio && current_track->data_type == TRACK_AUDIO) ||
			(video && current_track->data_type == TRACK_VIDEO)))
		{ 
			return current_track->get_feather(selectionstart, selectionend);
		}
	}
return 0;
}

int Tracks::reset_translation(long start, long end)
{
	Patch *current_patch;
	Track *current_track;
	int result = 0;

	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->record) { result += current_track->reset_translation(start, end); }
	}

	if(result)
	{
		mwindow->draw();
		mwindow->changes_made = 1;
	}
return 0;
}



int Tracks::purge_asset(Asset *asset)
{
	Track *current_track;
	int result = 0;
	
	for(current_track = first; current_track; current_track = current_track->next)
	{
		result += current_track->purge_asset(asset); 
	}
	return result;
return 0;
}

int Tracks::asset_used(Asset *asset)
{
	Track *current_track;
	int result = 0;
	
	for(current_track = first; current_track; current_track = current_track->next)
	{
		result += current_track->asset_used(asset); 
	}
	return result;
return 0;
}

int Tracks::scale_video(int *dimension, int *offsets, int scale_data)
{
	Track *current_track;
	int result = 0;
// Tracks are scaled using a single z curve and independant x and y pans, 
// so take the lowest of the horizontal and vertical scales as the zoom factor.
	float camera_scale = 1, projector_scale = 1;
	if(scale_data)
	{
		float hscale;
		float vscale;
		hscale = dimension[0] / mwindow->track_w;
		vscale = dimension[1] / mwindow->track_h;
		camera_scale = hscale < vscale ? hscale : vscale;
		hscale = dimension[2] / mwindow->output_w;
		vscale = dimension[3] / mwindow->output_h;
		projector_scale = hscale < vscale ? hscale : vscale;
	}

	for(current_track = first; current_track; current_track = current_track->next)
	{
		if(current_track->data_type == TRACK_VIDEO)
			result += ((VTrack*)current_track)->scale_video(camera_scale, projector_scale, offsets);
	}
	return result;	
return 0;
}

int Tracks::scale_time(float rate_scale, int ignore_record, int scale_edits, int scale_autos, long start, long end)
{
	Patch *current_patch;
	Track *current_track;

	for(current_track = first, current_patch = mwindow->patches->first; 
		current_track && current_patch; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if((current_patch->record || ignore_record) && current_track->data_type == TRACK_VIDEO)
		{
			current_track->scale_time(rate_scale, scale_edits, scale_autos, start, end);
		}
	}
	return 0;
return 0;
}

