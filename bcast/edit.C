#include <string.h>
#include "assets.h"
#include "edit.h"
#include "edits.h"
#include "filehtal.h"
#include "mainwindow.h"
#include "trackcanvas.h"
#include "tracks.h"
#include "transition.h"


Edit::Edit(MainWindow *mwindow, Edits *edits) : ListItem<Edit>()
{
	this->mwindow = mwindow;
	this->edits = edits;
	feather_left = 0;   
	feather_right = 0;
	startsource = 0;
	startproject = 0;
	length = 0;
	asset = 0;
	transition = 0;
}

Edit::~Edit()
{
	if(transition) delete transition;
}

int Edit::load_properties(FileHTAL *htal, long &startproject)
{
	startsource = htal->tag.get_property("STARTSOURCE", (long)0);
	this->startproject = startproject;
	feather_left = htal->tag.get_property("FEATHERLEFT", (long)0);
	feather_right = htal->tag.get_property("FEATHERRIGHT", (long)0);
	load_properties_derived(htal);
return 0;
}



// =================================================== drawing

int Edit::draw_transition(int flash, int center_pixel, int x, int w, int y, int h, int set_index_file)
{
	float view_start, view_units, zoom_units;
	long pixel1, pixel2, left_unit, right_unit;
	int vertical = mwindow->tracks_vertical;
	int view_pixels = mwindow->tracks->view_pixels();
	int row1, row2, title_x, title_y;
	int left_visible = 0, right_visible = 0;

	if(set_index_file) return 0;

	row1 = edits->track->pixel;
	row2 = row1 + mwindow->zoom_track;
	edits->track->get_dimensions(view_start, view_units, zoom_units);
	get_handle_parameters(pixel1, pixel2, left_unit, right_unit, view_start, zoom_units);

	title_x = vertical ? row1 : pixel1;
	title_y = vertical ? (pixel1 + pixel2) / 2 : (row1 + row2) / 2;

	if(pixel1 >= 0 && pixel1 < view_pixels)
	{
		left_visible = 1;
	}
	else
	if(pixel1 < 0) pixel1 = 0;
	else
	if(pixel1 >= view_pixels) pixel1 = view_pixels;

	if(pixel2 >= 0 && pixel2 < view_pixels)
	{
		right_visible = 1;
	}
	else
	if(pixel2 < 0) pixel1 = 0;
	else
	if(pixel2 >= view_pixels) pixel2 = view_pixels;

	if(pixel1 < pixel2)
	{
// Edit visible
// Clear the region
		mwindow->tracks->canvas->set_color(BLACK);
		if(vertical) 
			mwindow->tracks->canvas->draw_box(row1, pixel1, row2 - row1, pixel2 - pixel1);
		else
			mwindow->tracks->canvas->draw_box(pixel1, row1, pixel2 - pixel1, row2 - row1);

		mwindow->tracks->canvas->set_color(GREEN);

		if(left_visible)
		{
			if(vertical)
				mwindow->tracks->canvas->draw_line(row1, pixel1, row2, pixel1);
			else
				mwindow->tracks->canvas->draw_line(pixel1, row1, pixel1, row2);
		}

		if(right_visible)
		{
			if(vertical)
				mwindow->tracks->canvas->draw_line(row1, pixel2, row2, pixel2);
			else
				mwindow->tracks->canvas->draw_line(pixel2, row1, pixel2, row2);
		}

		if(vertical)
		{
			mwindow->tracks->canvas->draw_line(row1, pixel1, row1, pixel2);
			mwindow->tracks->canvas->draw_line(row2, pixel1, row2, pixel2);
		}
		else
		{
			mwindow->tracks->canvas->draw_line(pixel1, row1, pixel2, row1);
			mwindow->tracks->canvas->draw_line(pixel1, row2, pixel2, row2);
		}

		mwindow->tracks->canvas->set_color(BLUE);
		mwindow->tracks->canvas->draw_text(title_x, title_y, transition->plugin_title);

		if(flash) 
		{
// flash just this edit
			if(vertical) 
				mwindow->tracks->canvas->flash(row1, pixel1, row2 - row1, pixel2 - pixel1);
			else
				mwindow->tracks->canvas->flash(pixel1, row1, pixel2 - pixel1, row2 - row1);
		}
	}
return 0;
}

int Edit::draw_handles(BC_Canvas *canvas, float view_start, float view_units, float zoom_units, int view_pixels, int center_pixel)
{
	long pixel1, pixel2, left_unit, right_unit;

	float view_end;
	float endproject;
	int vertical;

	view_end = view_start + view_units;
	endproject = startproject + length;
	vertical = mwindow->tracks_vertical;

	get_handle_parameters(pixel1, pixel2, left_unit, right_unit, view_start, zoom_units);

	if(pixel1 >= -5 && pixel1 <= view_pixels + 5)    // range in project
	{
// start edit is visible
		if(vertical)
		canvas->draw_start_edit(center_pixel, pixel1, vertical);
		else
		canvas->draw_start_edit(pixel1, center_pixel, vertical);
	}

	if(pixel2 >= -5 && pixel2 <= view_pixels + 5)    // range in project
	{
//printf("Edit::draw_handles %d %d %d\n", pixel2, center_pixel, vertical);
// end edit is visible
		if(vertical)
			canvas->draw_end_edit(center_pixel, pixel2, vertical);
		else
			canvas->draw_end_edit(pixel2, center_pixel, vertical);
	}
return 0;
}

int Edit::draw_titles(BC_Canvas *canvas, float view_start, float zoom_units, int view_pixels, int center_pixel)
{
	float pixel;
	if(asset)
	{
		if(previous && previous->asset && !strcmp(asset->path, previous->asset->path)) return 0;  // same as previous edit
		if(asset->silence) return 0;     // silence
	}
	else
	if(transition) return 0;    // Is a transition


	pixel = (startproject - view_start) / zoom_units;
	pixel += length / zoom_units / 2;
	
	if(pixel >= 0 && pixel <= view_pixels)    // range in project
	{
// start edit is visible
		char string[256];
		FileSystem dir;    // don't need home directory
		
		dir.extract_name(string, asset->path);
		canvas->set_color(BLUE);
		if(mwindow->tracks_vertical)
			canvas->draw_text(center_pixel - mwindow->zoom_track / 2, pixel, string);
		else
			canvas->draw_vertical_text(pixel, center_pixel - mwindow->zoom_track / 2, string, BLUE, BLACK);
	}
return 0;
}

// ================================================== editing

int Edit::copy(long start, long end, FileHTAL *htal)
{
// variables
	long endproject = startproject + length;
	int result;

	if((startproject >= start && startproject <= end) ||  // startproject in range
		 (endproject <= end && endproject >= start) ||	   // endproject in range
		 (startproject <= start && endproject >= end))    // range in project
	{   // edit is in range
		long startproject_in_selection = startproject; // start of edit in selection in project
		long startsource_in_selection = startsource; // start of source in selection in source
		long endsource_in_selection = startsource + length; // end of source in selection
		long length_in_selection = length;             // length of edit in selection

		if(startproject < start)
		{         // start is after start of edit in project
			long length_difference = start - startproject;

			startsource_in_selection += length_difference;
			startproject_in_selection += length_difference;
			length_in_selection -= length_difference;
		}
		if(endproject > end)
		{         // end is before end of edit in project
			length_in_selection = end - startproject_in_selection;
		}
		
		if(htal)    // only if not counting
		{
			htal->tag.set_title("EDIT");
			htal->tag.set_property("STARTSOURCE", startsource_in_selection);
			htal->tag.set_property("FEATHERLEFT", feather_left);
			htal->tag.set_property("FEATHERRIGHT", feather_right);

			copy_properties_derived(htal, length_in_selection);

			htal->append_tag();
			htal->append_newline();

			if(asset)
			{
				if(asset->silence)
				{
					htal->tag.set_title(SILENCE);
					htal->append_tag();
				}
				else
				{
					char stored_path[1024];
					FileSystem fs;

					htal->tag.set_title("FILE");
					fs.extract_name(stored_path, asset->path);
					htal->tag.set_property("SRC", stored_path);
					htal->append_tag();
				}
			}

			if(transition)
			{
				transition->save(htal, "TRANSITION");
			}

			htal->tag.set_title("/EDIT");
			htal->append_tag();
			htal->append_newline();	
		}

		result = 1;
	}
	else
	{
		result = 0;
	}
	return result;
return 0;
}

int Edit::paste(FileHTAL *htal)
{
	load_properties_derived(htal);

	int result = 0;
	
	do{
		result = htal->read_tag();

		if(!result)
		{
			if(!strcmp(htal->tag.get_title(), "/EDIT"))
			{
				result = 1;
			}
			else
			if(!strcmp(htal->tag.get_title(), "FILE"))
			{
				char *new_filename = htal->tag.get_property("SRC");

				asset = mwindow->assets->update(new_filename ? new_filename : SILENCE);
			}
			else
			if(!strcmp(htal->tag.get_title(), "TRANSITION"))
			{
				transition = new Transition(mwindow, this, 
					edits->track->data_type == TRACK_AUDIO, 
					edits->track->data_type == TRACK_VIDEO);
				transition->load(htal, 0, "/TRANSITION");
			}
			else
			if(!strcmp(htal->tag.get_title(), SILENCE))
			{
				asset = mwindow->assets->update(SILENCE);
			}
		}
	}while(!result);
return 0;
}

int Edit::popup_transition(float view_start, float zoom_units, int cursor_x, int cursor_y)
{
	long left, right, left_unit, right_unit;
	if(!transition) return 0;
	get_handle_parameters(left, right, left_unit, right_unit, view_start, zoom_units);

	if(cursor_x > left && cursor_x < right)
	{
		transition->popup_transition(cursor_x, cursor_y);
		return 1;
	}
	return 0;
return 0;
}

int Edit::select_handle(float view_start, float zoom_units, int cursor_x, int cursor_y, long &selection)
{
	long left, right, left_unit, right_unit;
	get_handle_parameters(left, right, left_unit, right_unit, view_start, zoom_units);

	long pixel1, pixel2;
	pixel1 = left;
	pixel2 = pixel1 + 10;

// test left edit
// cursor_x is faked in acanvas
	if(cursor_x >= pixel1 && cursor_x <= pixel2)
	{
		selection = left_unit;
		return 1;     // left handle
	}

	long endproject = startproject + length;
	pixel2 = right;
	pixel1 = pixel2 - 10;

// test right edit	
	if(cursor_x >= pixel1 && cursor_x <= pixel2)
	{
		selection = right_unit;
		return 2;     // right handle
	}
	return 0;
return 0;
}

int Edit::shift_start_in(int edit_mode, long newposition, long oldposition)
{
	long cut_length = newposition - oldposition;
	long end_previous_source, end_source;
	if(edit_mode == MOVE_ALL_EDITS)
	{
		if(cut_length < length)
		{        // clear partial 
			edits->track->clear(oldposition, newposition, 0);
		}
		else
		{        // clear entire
			edits->track->clear(oldposition, startproject + length, 0);
		}
	}
	else
	if(edit_mode == MOVE_ONE_EDIT)
	{
		if(previous)
		{
			end_previous_source = previous->get_source_end();
			if(end_previous_source > 0 && previous->startsource + previous->length + cut_length > end_previous_source)
				cut_length = end_previous_source - previous->startsource - previous->length;

			if(cut_length < length)
			{		// Move in partial
				startproject += cut_length;
				startsource += cut_length;
				length -= cut_length;
				previous->length += cut_length;
			}
			else
			{		// Clear entire edit
				cut_length = length;
				previous->length += cut_length;
				for(Edit* current_edit = this; current_edit; current_edit = current_edit->next)
				{
					current_edit->startproject += cut_length;
				}
				edits->track->clear(oldposition + cut_length, startproject + cut_length, 0);
			}
		}
	}
	else
	if(edit_mode == MOVE_NO_EDITS)
	{
		end_source = get_source_end();
		if(end_source > 0 && startsource + length + cut_length > end_source)
			cut_length = end_source - startsource - length;
		
		startsource += cut_length;
	}
return 0;
}

int Edit::shift_start_out(int edit_mode, long newposition, long oldposition)
{
	long cut_length = oldposition - newposition;
	long end_source = get_source_end();

	if(end_source > 0 && startsource < cut_length)
	{
		cut_length = startsource;
	}

	if(edit_mode == MOVE_ALL_EDITS)
	{
		startsource -= cut_length;
		length += cut_length;

		if(mwindow->autos_follow_edits)
			edits->track->paste_auto_silence(startproject, startproject + cut_length);

		for(Edit* current_edit = next; current_edit; current_edit = current_edit->next)
		{
			current_edit->startproject += cut_length;
		}
	}
	else
	if(edit_mode == MOVE_ONE_EDIT)
	{
		if(previous)
		{
			if(cut_length < previous->length)
			{   // Cut into previous edit
				previous->length -= cut_length;
				startproject -= cut_length;
				startsource -= cut_length;
				length += cut_length;
			}
			else
			{   // Clear entire previous edit
				cut_length = previous->length;
				previous->length = 0;
				length += cut_length;
				startsource -= cut_length;
				startproject -= cut_length;
//				edits->track->clear(previous->startproject, previous->startproject + previous->length, 0);
			}
		}
	}
	else
	if(edit_mode == MOVE_NO_EDITS)
	{
		startsource -= cut_length;
	}

// Fix infinite length files
	if(startsource < 0) startsource = 0;
return 0;
}

int Edit::shift_end_in(int edit_mode, long newposition, long oldposition)
{
	long cut_length = oldposition - newposition;
	if(edit_mode == MOVE_ALL_EDITS)
	{
		if(newposition > startproject)
		{        // clear partial edit
			edits->track->clear(newposition, oldposition, 0);
		}
		else
		{        // clear entire edit
			edits->track->clear(startproject, oldposition, 0);
		}
	}
	else
	if(edit_mode == MOVE_ONE_EDIT)
	{
		if(next)
		{
			long end_source = next->get_source_end();

			if(end_source > 0 && next->startsource - cut_length < 0)
			{
				cut_length = next->startsource;
			}

			if(cut_length < length)
			{
				length -= cut_length;
				next->startproject -= cut_length;
				next->startsource -= cut_length;
				next->length += cut_length;
			}
			else
			{
				cut_length = length;
				next->length += cut_length;
				next->startsource -= cut_length;
				next->startproject -= cut_length;
				length -= cut_length;
//				edits->track->clear(startproject, oldposition, 0);
			}
		}
		else
		{
			if(cut_length < length)
			{
				length -= cut_length;
			}
			else
			{
				cut_length = length;
				edits->track->clear(startproject, oldposition, 0);
			}
		}
	}
	else
	if(edit_mode == MOVE_NO_EDITS)
	{
		long end_source = get_source_end();
		if(end_source > 0 && startsource < cut_length)
		{
			cut_length = startsource;
		}
		startsource -= cut_length;
	}
return 0;
}

int Edit::shift_end_out(int edit_mode, long newposition, long oldposition)
{
	long cut_length = newposition - oldposition;
	long endsource = get_source_end();

	if(endsource > 0 && startsource + length + cut_length > endsource)
		cut_length = endsource - startsource - length;

	if(edit_mode == MOVE_ALL_EDITS)
	{
// check end of edit against end of source file
		if(mwindow->autos_follow_edits)
			edits->track->paste_auto_silence(startproject + length, startproject + length + cut_length);
		length += cut_length;

		for(Edit* current_edit = next; current_edit; current_edit = current_edit->next)
		{
			current_edit->startproject += cut_length;
		}
	}
	else
	if(edit_mode == MOVE_ONE_EDIT)
	{
		if(next)
		{
			if(cut_length < next->length)
			{
				length += cut_length;
				next->startproject += cut_length;
				next->startsource += cut_length;
				next->length -= cut_length;
			}
			else
			{
				cut_length = next->length;
				next->length = 0;
				length += cut_length;
//				edits->track->clear(oldposition, oldposition + next->length, 0);
			}
		}
		else
		{
			length += cut_length;
		}
	}
	else
	if(edit_mode == MOVE_NO_EDITS)
	{
		startsource += cut_length;
	}
return 0;
}

int Edit::dump()
{
	printf("	edit %x\n", this); fflush(stdout);
	printf("		asset %x\n", asset); fflush(stdout);
	printf("		transition %x\n", transition); fflush(stdout);
	printf("		startproject %ld length %ld\n", startproject, length);
	if(transition) transition->dump();
return 0;
}
