#include <string.h>
#include "labels.h"
#include "recordlabel.h"


RecordLabel::RecordLabel(long position)
 : ListItem<RecordLabel>()
{ this->position = position; original = 0; }

RecordLabel::~RecordLabel()
{
}


RecordLabels::RecordLabels()
 : List<RecordLabel>()
{
}

RecordLabels::~RecordLabels()
{
}

int RecordLabels::delete_new_labels()
{
	RecordLabel *current, *next_;
	for(current = first; current; current = next_)
	{
		next_ = NEXT;
		if(!current->original) remove(current);
	} 
return 0;
}

int RecordLabels::toggle_label(long position)
{
// find label the position is after
	for(current = first; 
		current && current->position < position; 
		current = NEXT)
	{
		;
	}
	
	if(current)
	{
//printf("position %ld current->position %ld current->original %d\n", position, current->position,  current->original);
		if(current->position == position && !current->original)
		{        // remove it
			remove(current);
		}
		else
		if(!current->original)
		{        // insert before it
			insert_before(current);
			current->position = position;
		}
	}
	else
	{           // insert after last
//printf("position %ld\n", position);
		append(new RecordLabel(position));
	}
return 0;
}

long RecordLabels::get_prev_label(long position)
{
	RecordLabel *current;
	
	for(current = last; 
		current && current->position > position; 
		current = PREVIOUS)
	{
		;
	}
//printf("%ld\n", current->position);
	if(current && current->position <= position) return current->position; else return -1;
}

long RecordLabels::get_next_label(long position)
{
	RecordLabel *current;

	for(current = first; 
		current && current->position <= position; 
		current = NEXT)
	{
		;
	}
	if(current && current->position >= position) return current->position; else return -1;
}

long RecordLabels::goto_prev_label(long position)
{
	RecordLabel *current;
	
	for(current = last; 
		current && current->position >= position; 
		current = PREVIOUS)
	{
		;
	}
//printf("%ld\n", current->position);
	if(current && current->position <= position) return current->position; else return -1;
}

long RecordLabels::goto_next_label(long position)
{
	RecordLabel *current;

	for(current = first; 
		current && current->position <= position; 
		current = NEXT)
	{
		;
	}
	if(current && current->position >= position) return current->position; else return -1;
}

int RecordLabels::get_project_labels(Labels *labels, long start, long end)
{
	Label *current;
	RecordLabel *new_label;
	
	for(current = labels->label_of(start); current; current = NEXT)
	{
		append(new_label = new RecordLabel(current->position - start));
		new_label->original = 1;
	}
return 0;
}
