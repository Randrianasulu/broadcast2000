#ifndef LABEL_H
#define LABEL_H


#include "bcbase.h"
#include "filehtal.inc"
#include "labels.inc"
#include "mainwindow.inc"
#include "recordlabel.inc"
#include "timebar.inc"

#define LABELSIZE 15

class LabelToggle : public BC_Label
{
public:
	LabelToggle(MainWindow *mwindow, Label *label, int x, int y, long position);
	~LabelToggle();
	
	int handle_event();
	MainWindow *mwindow;
	Label *label;
};

class Label : public ListItem<Label>
{
public:
	Label();        // limitation of link list
	~Label();

	int create_objects(MainWindow *mwindow, Labels *labels, long position_); // limitation of link list
	int draw();
	int samplemovement();
	int get_pixel(long position);       // get pixel of label on screen

	MainWindow *mwindow;
	LabelToggle *toggle;
	Labels *labels;
	long position;
};

class Labels : public List<Label>
{
public:
	Labels(MainWindow *mwindow, TimeBar *timebar);
	~Labels();
	
	int toggle_label(long start, long end);
    int equivalent(long position1, long position2);
	int flip_vertical();
	int draw();
	int samplemovement();
	int resize_event(int x);
	int delete_all();
	int save(FileHTAL *htal);
	int load(FileHTAL *htal);
	
	int modify_handles(long oldposition, long newposition, int currentend);
	int cut(long start, long end, StringFile *stringfile);
	int copy(long start, long end, FileHTAL *htal);
	int copy_length(long start, long end); // return number of Labels in selection
	int insert(long start, long lengthsamples);
	int paste(long start, long end, long total_length, FileHTAL *htal);
	int paste_output(long startproject, long endproject, long startsource, long endsource, RecordLabels *labels);
// Setting follow to 1 causes labels to move forward after clear.
// Setting it to 0 implies ignoring the labels follow edits setting.
	int clear(long start, long end, int follow = 1);
	int paste_silence(long start, long end);
	int optimize();  // delete duplicates

	Label* label_of(long position); // first label on or after position
	MainWindow *mwindow;
	TimeBar *timebar;
};

#endif
