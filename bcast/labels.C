#include <string.h>
#include "filehtal.h"
#include "labels.h"
#include "mainwindow.h"
#include "mainwindowgui.h"
#include "patchbay.h"
#include "recordlabel.h"
#include "timebar.h"

Labels::Labels(MainWindow *mwindow, TimeBar *timebar) : List<Label>()
{
	this->mwindow = mwindow;
	this->timebar = timebar;
}

Labels::~Labels()
{
	delete_all();
}

int Labels::flip_vertical()
{
	Label *current;

	if(mwindow->gui)
	{	
		if(mwindow->tracks_vertical)
		{
			for(current = first; current; current = NEXT)
			{
				current->toggle->set_x(timebar->gui->get_x() + 2);
			}
		}
		else
		{
			for(current = first; current; current = NEXT)
			{
				current->toggle->set_y(timebar->gui->get_y() + 2);
			}
		}
		samplemovement();
	}
return 0;
}

int Labels::equivalent(long position1, long position2)
{
	if((mwindow->cursor_on_frames && labs(position2 - position1) < mwindow->sample_rate / mwindow->frame_rate) ||
    	(position1 == position2))
    	return 1;
    else
        return 0;
return 0;
}


int Labels::toggle_label(long start, long end)
{
	Label *current;
	
// handle selection start
// find label the selectionstart is after
	for(current = first; 
		current && current->position < start; 
		current = NEXT)
	{
		;
	}

	if(current)
	{
		if(equivalent(current->position, start))
		{        // remove it
			remove(current);
		}
		else
		{        // insert before it
			current = insert_before(current);
			current->create_objects(mwindow, this, start);
		}
	}
	else
	{           // insert after last
		current = append();
		current->create_objects(mwindow, this, start);
	}

// handle selection end
	if(start != end)
	{
// find label the selectionend is after
		for(current = first; 
			current && current->position < end; 
			current = NEXT)
		{
			;
		}

		if(current)
		{
			if(equivalent(current->position, end))
			{
				remove(current);
			}
			else
			{
				current = insert_before(current);
				current->create_objects(mwindow, this, end);
			}
		}
		else
		{
			current = append();
			current->create_objects(mwindow, this, end);
		}
	}
return 0;
}

int Labels::draw()
{
	Label *current;
	
	if(mwindow->gui)
	{
		for(current = first; current; current = NEXT)
		{
			current->draw();
		}
	}
return 0;
}

int Labels::samplemovement()
{
	Label *current;
	
	if(mwindow->gui)
	{
		for(current = first; current; current = NEXT)
		{
			current->samplemovement();
		}
	}
return 0;
}

int Labels::resize_event(int x)
{
	if(mwindow->gui && mwindow->tracks_vertical)
	{
		for(current = first; current; current = NEXT)
		{
			current->toggle->set_x(x);
		}
	}
return 0;
}

int Labels::delete_all()
{
	while(last)
		remove(last);
return 0;
}

int Labels::cut(long start, long end, StringFile *stringfile)
{
return 0;
}

int Labels::copy(long start, long end, FileHTAL *htal)
{
	htal->tag.set_title("LABELS");
	htal->append_tag();
	htal->append_newline();

	Label *current;

	for(current = label_of(start); current && current->position <= end; current = NEXT)
	{
		htal->tag.set_title("LABEL");
		htal->tag.set_property("SAMPLE", (long)current->position - start);
		htal->append_tag();
	}
	
	htal->tag.set_title("/LABELS");
	htal->append_tag();
	htal->append_newline();
	htal->append_newline();
return 0;
}

int Labels::copy_length(long start, long end) // return number of Labels in selection
{
	int result = 0;
	Label *current;
	
	for(current = label_of(start); current && current->position <= end; current = NEXT)
	{
		result++;
	}
	return result;
return 0;
}

int Labels::paste(long start, long end, long total_length, FileHTAL *htal)
{
	if(mwindow->labels_follow_edits) clear(start, end);
	if(mwindow->labels_follow_edits) insert(start, total_length -  (end - start));
	
	int result = 0;
	Label *current;
	long position;

	do{
		result = htal->read_tag();
		if(!result)
		{
			if(!strcmp(htal->tag.get_title(), "/LABELS"))
			{
				result = 1;
			}
			else
			if(!strcmp(htal->tag.get_title(), "LABEL") && mwindow->labels_follow_edits)
			{
				position = htal->tag.get_property("SAMPLE", (long)-1);
				position += start;
				if(position > -1)
				{
					current = label_of(position);
					current = insert_before(current);
					current->create_objects(mwindow, this, position);
				}
			}
		}
	}while(!result);
	
	optimize();
	if(mwindow->gui) samplemovement();
return 0;
}

int Labels::paste_output(long startproject, long endproject, long startsource, long endsource, RecordLabels *labels)
{
	if(mwindow->labels_follow_edits) clear(startproject, endproject);
	if(mwindow->labels_follow_edits) insert(startproject, endsource - startsource);

	RecordLabel *reclabel;
	long position;
	Label *current;

	for(reclabel = labels->first; reclabel; reclabel = reclabel->next)
	{
		if(!reclabel->original && reclabel->position >= startsource)
		{
			position = reclabel->position - startsource;
			position += startproject;
			current = label_of(position);
			current = insert_before(current);
			current->create_objects(mwindow, this, position);
		}
	}
	optimize();
return 0;
}

int Labels::save(FileHTAL *htal)
{
	htal->tag.set_title("LABELS");
	htal->append_tag();
	htal->append_newline();

	Label *current;

	for(current = first; current; current = NEXT)
	{
		htal->tag.set_title("LABEL");
		htal->tag.set_property("SAMPLE", (long)current->position);
		htal->append_tag();
	}
	
	htal->tag.set_title("/LABELS");
	htal->append_tag();
	htal->append_newline();
	htal->append_newline();
return 0;
}

int Labels::load(FileHTAL *htal)
{
	int result = 0;
	Label *current;
	long position;

	do{
		result = htal->read_tag();
		if(!result)
		{
			if(!strcmp(htal->tag.get_title(), "/LABELS"))
			{
				result = 1;
			}
			else
			if(!strcmp(htal->tag.get_title(), "LABEL"))
			{
				position = htal->tag.get_property("SAMPLE", (long)-1);
				if(position > -1)
				{
					current = label_of(position);
					current = insert_before(current);
					current->create_objects(mwindow, this, position);
				}
			}
		}
	}while(!result);
return 0;
}





int Labels::clear(long start, long end, int follow)
{
	Label *current;
	Label *next;
	
	if(mwindow->labels_follow_edits || !follow)
	{
		current = label_of(start);
		while(current && current->position < end)
		{
			next = NEXT;
			remove(current);              // remove selected labels
			current = next;
		}
		while(current && follow)
		{
			current->position -= end - start;   // shift labels forward
			current = NEXT;
		}
		optimize();
		samplemovement();
	}
return 0;
}

int Labels::insert(long start, long lengthsamples)
{      // shift every label including the first one back
	Label *current;

	for(current = label_of(start); current; current = NEXT)
	{
		current->position += lengthsamples;
	}
	current = label_of(start);
	samplemovement();
return 0;
}

int Labels::paste_silence(long start, long end)
{
	if(mwindow->labels_follow_edits)
	{
		insert(start, end - start);
		optimize();
		samplemovement();
	}
return 0;
}

int Labels::modify_handles(long oldposition, long newposition, int currentend)
{
	if(mwindow->labels_follow_edits)
	{
		if(currentend == 1)          // left handle
		{
			if(newposition < oldposition)
			{
				insert(oldposition, oldposition - newposition);    // shift all labels right
			}
			else
			{
				clear(oldposition, newposition);   // clear selection
			}
		}
		else
		{                            // right handle
			if(newposition < oldposition)
			{
				clear(newposition, oldposition);
			}
			else
			{
				insert(oldposition, newposition - oldposition);
			}
		}
		samplemovement();
	}
return 0;
}

int Labels::optimize()
{
	int result = 1;
	Label *current;

	while(result)
	{
		result = 0;
		
		for(current = first; current && NEXT && !result;)
		{
			Label *next = NEXT;
			if(current->position == next->position)
			{
				remove(current);
				result  = 1;
			}
			current = next;
		}
	}
return 0;
}

Label* Labels::label_of(long position)
{
	Label *current;

	for(current = first; current; current = NEXT)
	{
		if(current->position >= position) return current;
	}
	return 0;
}

Label::Label() : ListItem<Label>()
{
	toggle = 0;
}

Label::~Label()
{
	if(toggle) delete toggle;
}

int Label::create_objects(MainWindow *mwindow, Labels *labels, long position)
{
	this->mwindow = mwindow;
	this->position = position;
	this->labels = labels;
	int x, y;
	
	if(mwindow->gui)
	{
		if(mwindow->tracks_vertical)
		{
			x = labels->timebar->gui->get_x() + 2;
			y = get_pixel(position) + labels->timebar->gui->get_y();
		}
		else
		{
			x = get_pixel(position);
			y = labels->timebar->gui->get_y() + 2;
		}

		mwindow->gui->add_tool(toggle = new LabelToggle(mwindow, this, x, y, position));
	}
return 0;
}

int Label::samplemovement()
{
	if(mwindow->gui)
	{
		if(mwindow->tracks_vertical)
		{
			toggle->set_y(get_pixel(position) + labels->timebar->gui->get_y());
		}
		else
		{
			toggle->set_x(get_pixel(position));
		}
	}
return 0;
}

int Label::draw()
{
	if(mwindow->gui)
	{
		if(mwindow->selectionstart != position && mwindow->selectionend != position && toggle->down) toggle->set_status(0);

		toggle->draw();
	}
return 0;
}

int Label::get_pixel(long position)
{
	if(mwindow->gui)
	{
		long value = (position - mwindow->view_start) / mwindow->zoom_sample - 8;
		value += mwindow->tracks_vertical ? PATCHBAYHEIGHT : PATCHBAYWIDTH;

// fix value so positions don't byte wrap
		if(value < -100) value = -100;
		if(value > 32767) value = 32767;

// don't draw labels over the menu or other things
//printf(" %d %d %d\n", value, mwindow->gui->menu_h() + TIMEBAR_PIXELS, mwindow->gui->get_h() - mwindow->gui->menu_h() - STATUSBAR_PIXELS - TIMEBAR_PIXELS);
		if(mwindow->tracks_vertical)
		{
			if(value < TIMEBAR_PIXELS) value = -100;
			else
			if(value > mwindow->gui->get_h() - mwindow->gui->menu_h() - STATUSBAR_PIXELS - TIMEBAR_PIXELS - LABELSIZE) 
				value = mwindow->gui->get_h() + 100;
		}
		else
		{
			if(value < TIMEBAR_PIXELS) value = -100;
			else
			if(value > mwindow->gui->get_w() - TIMEBAR_PIXELS - LABELSIZE) value = mwindow->gui->get_w() + 100;
		}

		return value;
	}
	else
	return 0;
return 0;
}

LabelToggle::LabelToggle(MainWindow *mwindow, Label *label, int x, int y, long position)
 : BC_Label(x, y, LABELSIZE, LABELSIZE, 0)
{
	this->mwindow = mwindow;
	this->label = label;
}

LabelToggle::~LabelToggle() { }

int LabelToggle::handle_event()
{
	set_status(1);
	long position;
	if(mwindow->cursor_on_frames)
		position = mwindow->align_to_frames(label->position);
	else
		position = label->position;

	mwindow->stop_playback(1);
	if(shift_down())
	{
		if(position > mwindow->selectionend / 2 + mwindow->selectionstart / 2)
		{
		
			mwindow->set_selectionend(position);
		}
		else
		{
			mwindow->set_selectionstart(position);
		}
	}
	else
	{
		mwindow->set_selection(position, position);
	}
	mwindow->stop_playback();
// pop up all the unselected labels
	label->labels->timebar->draw();
return 0;
}

