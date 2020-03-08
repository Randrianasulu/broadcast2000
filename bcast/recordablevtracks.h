#ifndef RECORDABLEVTRACKS_H
#define RECORDABLEVTRACKS_H

#include "arraylist.h"
#include "vtrack.inc"
#include "patchbay.inc"
#include "tracks.inc"

class RecordableVTracks : public ArrayList<VTrack*>
{
public:
	RecordableVTracks(Tracks *tracks, PatchBay *patches);
};



#endif
