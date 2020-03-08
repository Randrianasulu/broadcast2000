#include <string.h>
#include "atrack.h"
#include "patch.h"
#include "patchbay.h"
#include "recordableatracks.h"
#include "tracks.h"

// This is only used for menu effects so use playable tracks instead.

RecordableATracks::RecordableATracks(Tracks *tracks, PatchBay *patches)
 : ArrayList<ATrack*>()
{
	Track *current_track;
	Patch *current_patch;
	
	for(current_track = tracks->first, current_patch = patches->first; 
		current_track && current_patch; 
		current_track = current_track->next, current_patch = current_patch->next)
	{
		if(current_patch->play && current_track->data_type == TRACK_AUDIO) 
			append((ATrack*)current_track);
	}
}
