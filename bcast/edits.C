#include <string.h>
#include "aedit.h"
#include "assets.h"
#include "cache.h"
#include "edit.h"
#include "edits.h"
#include "file.h"
#include "filehtal.h"
#include "mainwindow.h"
#include "strategies.inc"
#include "track.h"
#include "transition.h"

Edits::Edits(MainWindow *mwindow, Track *track) : List<Edit>()
{
	this->track = track;
	this->mwindow = mwindow;
}

// ===================================== file operations

int Edits::save(FileHTAL *htal)
{
	copy(0, total_length(), htal);
return 0;
}

int Edits::load(FileHTAL *htal, int track_offset)
{
	while(last) remove(last);      // delete the current list

	int result = 0;
	long startproject = 0;
	
	do{
		result = htal->read_tag();
		
		if(!result)
		{
			if(!strcmp(htal->tag.get_title(), "EDIT"))
			{
				load_edit(htal, startproject, track_offset);
			}
			else
			if(!strcmp(htal->tag.get_title(), "/EDITS"))
			{
				result = 1;
			}
		}
	}while(!result);

	optimize();
return 0;
}

int Edits::load_edit(FileHTAL *htal, long &startproject, int track_offset)
{
	Edit* current;

	current = append_new_edit();

	current->load_properties(htal, startproject);

	startproject += current->length;

	int result = 0;

	do{
		result = htal->read_tag();

		if(!result)
		{
			if(htal->tag.title_is("FILE"))
			{
				char *filename = htal->tag.get_property("SRC");
				
				current->asset = mwindow->assets->update(filename ? filename : SILENCE);
			}
			else
			if(htal->tag.title_is("TRANSITION"))
			{
				current->transition = new Transition(mwindow, current, track->data_type == TRACK_AUDIO, track->data_type == TRACK_VIDEO);
				current->transition->load(htal, track_offset, "/TRANSITION");
			}
			else
			if(htal->tag.title_is(SILENCE))
			{
				current->asset = mwindow->assets->update(SILENCE);
			}
			else
			if(!strcmp(htal->tag.get_title(), "/EDIT"))
			{
				result = 1;
			}
		}
	}while(!result);

// in case of incomplete edit tag
// Transitions have no assets
//	if(!current->asset) current->asset = mwindow->assets->update(SILENCE);
return 0;
}

// ============================================= accounting

long Edits::total_length() 
{
	long total = 0;
	for(current = first; current; current = NEXT)
	{
		total += current->length;
	}
	return total; 
};

Edit* Edits::editof(long position)
{
	Edit *current;
	
	for(current = first; current; current = NEXT)
	{
		if(current->startproject <= position && current->startproject + current->length >= position)
			return current;
	}
	return current;     // return 0 on failure
}

Edit* Edits::get_render_strategy(long position, ArrayList<int> *render_strategies, int &render_strategy)
{
	Edit *current;

// Get the current edit
	for(current = first; current; current = NEXT)
	{
		if(current->startproject <= position && 
			current->startproject + current->length > position &&
			!(PREVIOUS && 
				position < PREVIOUS->feather_right + PREVIOUS->length + PREVIOUS->startproject))
			break;
	}

// Get the edit's asset
	if(current)
	{
		if(!current->asset)
			current = 0;
		else
		{
			if(current->asset->silence)
				current = 0;
		}
	}

// Get the fastest render strategy if an arraylist is supplied
	if(render_strategies)
	{
		if(!current)
		{
			render_strategy = VRENDER_VPIXEL;
		}
		else
		{
			File *file = mwindow->cache->check_out(current->asset);
			render_strategy = file->get_render_strategy(render_strategies);
			mwindow->cache->check_in(current->asset);
		}
	}

	return current;     // return 0 on failure
}

long Edits::end()
{
	if(last) return last->startproject + last->length;
	else return 0;
}

// ================================================ editing

Edit* Edits::insert(long start, long lengthsamples)
{
	Edit *old_edit, *new_edit;
	
	old_edit = editof(start);

	if(old_edit)
	{       // ========================== split the edit
		new_edit = insert_edit_after(old_edit);

		new_edit->startsource = old_edit->startsource + (start - old_edit->startproject);
// will be shifted later
		new_edit->startproject = old_edit->startproject + (start - old_edit->startproject);
		new_edit->length = old_edit->length - (start - old_edit->startproject);
		new_edit->feather_left = 0;
		new_edit->feather_right = old_edit->feather_right;

		old_edit->length -= new_edit->length;
		old_edit->feather_right = 0;

		new_edit->asset = old_edit->asset;

		if(old_edit->transition)
		{
			new_edit->transition = new Transition(old_edit->transition, new_edit);
		}
		else
			new_edit->transition = 0;

		clone_derived(new_edit, old_edit);

		new_edit = old_edit;
	}
	else
	{       // extend track with silence
		new_edit = append_new_edit();

		if(new_edit->previous)
			new_edit->startproject = new_edit->previous->startproject + new_edit->previous->length;

		new_edit->length = start - new_edit->startproject;

		new_edit->asset = mwindow->assets->update(SILENCE);
	}

	Edit* result = new_edit;

// shift everything after old_edit
	for(new_edit = new_edit->next; new_edit; new_edit = new_edit->next)
	{            // shift
		new_edit->startproject += lengthsamples;
	}

	return result;
}

int Edits::paste_transition(long startproject, 
				long endproject, 
				Transition *transition)
{
	Edit *current_edit;
	clear(startproject, endproject);
	current_edit = insert(startproject, endproject - startproject);

	current_edit = insert_edit_after(current_edit);

	current_edit->asset = 0;
	current_edit->startproject = startproject;
	current_edit->length = endproject - startproject;
	current_edit->startsource = 0;
	current_edit->feather_left = 0;
	current_edit->feather_right = 0;
	current_edit->transition = new Transition(transition, current_edit);
	if(transition->show)
	{
// Show the GUI of only the first transition pasted.
		current_edit->transition->show_gui();
		transition->show = 0;
	}

	optimize();
	return 0;
return 0;
}

Edit* Edits::paste_edit_base(long start, 
				long end, 
				long startsource, 
				long length, 
				Asset *asset)
{
	int result;
	Edit *current_edit;

// clear selection
	clear(start, end);

// Insert silence before new edit.
	current_edit = insert(start, length);

// create a new edit for the data and point *current to that edit
	current_edit = insert_edit_after(current_edit);

// put the data in the new edit
	current_edit->startsource = startsource;
	current_edit->startproject = start;
	current_edit->length = length;
	current_edit->feather_left = 0;
	current_edit->feather_right = 0;
	current_edit->asset = asset;

	return current_edit;
}


int Edits::copy(long start, long end, FileHTAL *htal)
{
	Edit *current_edit;

	htal->tag.set_title("EDITS");
	htal->append_tag();
	htal->append_newline();

	for(current_edit = first; current_edit; current_edit = current_edit->next)
	{
		current_edit->copy(start, end, htal);
	}

	htal->tag.set_title("/EDITS");
	htal->append_tag();
	htal->append_newline();
return 0;
}

int Edits::paste(long start, long end, long total_length, FileHTAL *htal)
{
	Edit *current_edit;

// clear selection
	clear(start, end);

	current_edit = insert(start, total_length);

	int result = 0;
	long startproject = start;

	do{
		result = htal->read_tag();

		if(!result)
		{
			if(!strcmp(htal->tag.get_title(), "/EDITS"))
			{
				result = 1;
			}
			else
			if(!strcmp(htal->tag.get_title(), "EDIT"))
			{
				current_edit = insert_edit_after(current_edit);

				current_edit->startsource = htal->tag.get_property("STARTSOURCE", (long)0);
				current_edit->startproject = startproject;
				current_edit->feather_left = htal->tag.get_property("FEATHERLEFT", (long)0);
				current_edit->feather_right = htal->tag.get_property("FEATHERRIGHT", (long)0);

				current_edit->load_properties_derived(htal);

				startproject += current_edit->length;
				current_edit->paste(htal);
			}
		}
	}while(!result);


// fill end manually since it can't be optimized
	current_edit = insert_edit_after(current_edit);

	current_edit->startproject = startproject;
	current_edit->length = total_length + start - startproject;

	current_edit->asset = mwindow->assets->update(SILENCE);

// optimize it out
	optimize();
	return 0;
return 0;
}

int Edits::clear(long start, long end)
{
	Edit* edit1 = editof(start);
	Edit* edit2 = editof(end);
	Edit* current_edit;

	if(end == start) return 0;        // nothing selected
	if(!edit1 && !edit2) return 0;       // nothing selected
	if(!edit2)
	{                // edit2 beyond end of track
		edit2 = last;
		end = this->end();
	}

	if(edit1 != edit2)
	{     // in different edits
		edit1->length = start - edit1->startproject;
		edit2->length -= end - edit2->startproject;
		edit2->startsource += end - edit2->startproject;
		edit2->startproject += end - edit2->startproject;
// 		edit1->feather_right = 0;
// 		edit2->feather_left = 0;

		for(current_edit = edit1->next; current_edit && current_edit != edit2;)
		{            // delete
			Edit* next = current_edit->next;
			remove(current_edit);
			current_edit = next;
		}
		for(current_edit = edit2; current_edit; current_edit = current_edit->next)
		{            // shift
			current_edit->startproject -= end - start;
		}
	}
	else
	{     // in same edit. paste_edit depends on this
		current_edit = insert_edit_after(edit1);           // create a new edit

		current_edit->startproject = start;
		current_edit->length = current_edit->previous->length - (end - current_edit->previous->startproject);
		current_edit->startsource = current_edit->previous->startsource + (end - current_edit->previous->startproject);
		current_edit->feather_left = current_edit->previous->feather_right;
		current_edit->feather_right = current_edit->previous->feather_right;
// 		current_edit->feather_left = 0;
// 		current_edit->feather_right = current_edit->previous->feather_right;
// 		current_edit->previous->feather_right = 0;
		current_edit->asset = current_edit->previous->asset;
		if(current_edit->previous->transition)
			current_edit->transition = new Transition(current_edit->previous->transition, current_edit);
		else
			current_edit->transition = 0;

		current_edit->previous->length = start - edit1->startproject;

		clone_derived(current_edit, current_edit->previous);

		for(current_edit = current_edit->next; current_edit; current_edit = current_edit->next)
		{            // shift
			current_edit->startproject -= end - start;
		}
	}

	optimize();
return 0;
}

int Edits::clear_handle(long start, long end)
{
	int result = 0;
	Edit *current_edit;

	for(current_edit = first; current_edit && current_edit->next && !result; )
	{
		if(current_edit->asset == current_edit->next->asset)
		{           // two consecutive edits
			if(current_edit->next->startproject == start)
			{            // handle selected
				result = 1;
				int length = -current_edit->length;
				current_edit->length = current_edit->next->startsource - current_edit->startsource;
				length += current_edit->length;
				
				track->paste_auto_silence(current_edit->next->startproject, current_edit->next->startproject + length);
				for(current_edit = current_edit->next; current_edit; current_edit = current_edit->next)
				{
					current_edit->startproject += length;
				}
				optimize();
			}
		}
		if(!result) current_edit = current_edit->next;
	}
return 0;
}

int Edits::modify_handles(long oldposition, long newposition, int currentend, int edit_mode)
{
	int result = 0;
	Edit *current_edit;

	if(currentend == 1)
	{            // left handle
		for(current_edit = first; current_edit && !result;)
		{
			if(identical(current_edit->startproject, oldposition))
			{
// edit matches selection
				oldposition = current_edit->startproject;
				if(newposition >= oldposition)
				{
// shift start of edit in
					current_edit->shift_start_in(edit_mode, newposition, oldposition);
				}
				else
				{
// move start of edit out
					current_edit->shift_start_out(edit_mode, newposition, oldposition);
				}
				result = 1;
			}
			if(!result) current_edit = current_edit->next;
		}
	}
	else
	{
// right handle selected
		for(current_edit = first; current_edit && !result;)
		{
			if(identical(current_edit->startproject + current_edit->length, oldposition))
			{
            	oldposition = current_edit->startproject + current_edit->length;
				if(newposition <= oldposition)
				{     // shift end of edit in
					current_edit->shift_end_in(edit_mode, newposition, oldposition);
				}
				else
				{     // move end of edit out
					current_edit->shift_end_out(edit_mode, newposition, oldposition);
				}
				result = 1;
			}
			if(!result) current_edit = current_edit->next;
		}
	}
	optimize();
return 0;
}

int Edits::paste_silence(long start, long end)
{
	paste_edit_base(start,          // start of selection to replace
				 start,            // end of selection to replace
				 0,                // start of selection in source
				 end - start,  // length in samples of source
				 mwindow->assets->update(SILENCE));

	optimize();
return 0;
}
				     
int Edits::optimize()
{
	int result = 1;
	Edit *current_edit;

//return 0;
	while(result)
	{
		result = 0;
// delete 0 length edits
		for(current_edit = first; current_edit && !result;)
		{
			if(current_edit->length == 0)
			{
				Edit* next = current_edit->next;
				delete current_edit;
				result = 1;
				current_edit = next;
			}
			else
				current_edit = current_edit->next;
		}

// merge same files or transitions
		for(current_edit = first; current_edit && current_edit->next && !result;)
		{
    		if(current_edit->asset)
			{	// assets identical
				if(current_edit->next->asset && 
					current_edit->asset == current_edit->next->asset && 
    		   		(current_edit->startsource + current_edit->length == current_edit->next->startsource ||
        			current_edit->asset->silence))
        		{       // source positions are consecutive
					if((track->data_type == TRACK_AUDIO &&
						((AEdit*)current_edit)->channel == ((AEdit*)current_edit->next)->channel) ||
						track->data_type != TRACK_AUDIO)
					{
        				current_edit->length += current_edit->next->length;
        				remove(current_edit->next);
        				result = 1;
					}
        		}
    		}
			else
			if(current_edit->transition)
			{	// Transitions are identical
				if(current_edit->next->transition && 
					*(current_edit->transition) == *(current_edit->next->transition) &&
        			current_edit->startproject + current_edit->length == current_edit->next->startproject)
        		{       // edit positions are consecutive
  					current_edit->length += current_edit->next->length;
        			remove(current_edit->next);
         			result = 1;
         		}
			}

    		current_edit = current_edit->next;
		}

// does last edit have length 0 or silence?
		if(last && last->asset && (last->asset->silence || !last->length))
		{
			delete last;
			result = 1;
		}
	}
return 0;
}
