#include <string.h>
#include "dcoffset.h"
#include "mutex.h"
#include "progressbox.h"
#include "recordgui.h"



DC_Offset::DC_Offset()
 : Thread()
{
	getting_dc_offset = 0;
	for(int i = 0; i < MAXCHANNELS; i++) dc_offset[i] = 0;
	dc_offset_lock = new Mutex;
}

DC_Offset::~DC_Offset()
{
	delete dc_offset_lock;
}

int DC_Offset::calibrate_dc_offset(long *output, RecordGUIDCOffsetText **dc_offset_text, int input_channels)
{
	this->output = output;
	this->dc_offset_text = dc_offset_text;
	this->input_channels = input_channels;
	start();
return 0;
}


void DC_Offset::run()
{
// thread out progress box
	progress = new ProgressBox("", "DC Offset", 256000);
	progress->start();

	dc_offset_lock->lock();
	dc_offset_count = 0;
	for(int i = 0; i < input_channels; i++) 
	{
		dc_offset_total[i] = 0;
		dc_offset[i] = 0;
	}
	getting_dc_offset = 1;
	
	dc_offset_lock->lock();       // wait for result
	dc_offset_lock->unlock();

// thread in progress box
	progress->stop_progress();
	delete progress;
	
	char string[256];
	int i;

// update interface
	for(i = 0; i < input_channels; i++) output[i] = dc_offset[i];
	for(i = 0; i < input_channels; i++)
	{
		sprintf(string, "%ld", dc_offset[i]);
		dc_offset_text[i]->update(string);
	}
}

