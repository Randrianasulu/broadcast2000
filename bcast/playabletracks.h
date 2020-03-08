#ifndef PLAYABLETRACKS_H
#define PLAYABLETRACKS_H

#include "arraylist.h"
#include "datatype.h"
#include "mainwindow.inc"
#include "track.h"

class PlayableTracks : public ArrayList<Track*>
{
public:
	PlayableTracks(MainWindow *mwindow, long position, int reverse, int data_type = TRACK_AUDIO);

// return 1 if the track is playable at the position
	int is_playable(Track *current_track, Patch *current_patch, long position, int reverse);
// return 1 if the track is in the list
	int is_listed(Track *track);
	
	int data_type;
	MainWindow *mwindow;
};



#endif
