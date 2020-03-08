#include <string.h>
#include "mainwindow.h"
#include "toggleauto.h"
#include "toggleautos.h"
#include "patchbay.h"
#include "playabletracks.h"
#include "preferences.h"
#include "tracks.h"

PlayableTracks::PlayableTracks(MainWindow *mwindow, long position, int reverse, int data_type)
 : ArrayList<Track*>()
{
	this->mwindow = mwindow;
	this->data_type = data_type;
	Track *current_track;
	Patch *current_patch;
	
	for(current_track = mwindow->tracks->first, 
		current_patch = mwindow->patches->first; 
		current_track && current_patch; 
		current_track = current_track->next, 
		current_patch = current_patch->next)
	{
		if(is_playable(current_track, current_patch, position, reverse))
		{
			append(current_track);
		}
	}
}

int PlayableTracks::is_playable(Track *current_track, Patch *current_patch, long position, int reverse)
{
	Auto *current_auto;
	int result = 0;

	if(current_patch->play && current_track->data_type == data_type)
	{
// Test edit under the current position.
		if(mwindow->preferences->test_playback_edits)
		{
			result = current_track->playable_edit(position);
		}
		else
		{
			result = 1;
		}


// Test auto right before or on the current position.
		if(current_patch->automate && result)
		{
			result = 0;

			if(reverse)
			{
				current_auto = current_track->play_autos->nearest_before(position);

				if(!current_auto) current_auto = current_track->play_autos->first;
			}
			else
			{
				current_auto = current_track->play_autos->last;
				while(current_auto && current_auto->position > position)
					current_auto = current_auto->previous;

				if(!current_auto) current_auto = current_track->play_autos->first;
			}

			if(!current_auto || current_auto->value > 0)
				result = 1;
		}
	}

	return result;
return 0;
}


int PlayableTracks::is_listed(Track *track)
{
	for(int i = 0; i < total; i++)
	{
		if(values[i] == track) return 1;
	}
	return 0;
return 0;
}
