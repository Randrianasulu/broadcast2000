#ifndef RECORDLABELS_H
#define RECORDLABELS_H

#include "labels.inc"
#include "linklist.h"


class RecordLabel : public ListItem<RecordLabel>
{
public:
	RecordLabel() {  };
	RecordLabel(long position);
	~RecordLabel();
	
	long position;
	int original;           // copied from project
};


class RecordLabels : public List<RecordLabel>
{
public:
	RecordLabels();
	~RecordLabels();
	
	int delete_new_labels();
	int toggle_label(long position);
	long get_prev_label(long position);
	long get_next_label(long position);
	long goto_prev_label(long position);
	long goto_next_label(long position);
	int get_project_labels(Labels *labels, long start, long end);  // call first 
};

#endif
