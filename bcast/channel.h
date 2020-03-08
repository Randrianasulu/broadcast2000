#ifndef CHANNEL_H
#define CHANNEL_H

#include "filehtal.inc"

class Channel
{
public:
	Channel();
	Channel(Channel *channel);
	~Channel();
	
	Channel& operator=(Channel &channel);
	int load(FileHTAL *file);
	int save(FileHTAL *file);

	char title[1024];
	int entry;  // Number of the table entry in the appropriate freqtable
	int freqtable;    // Table to use
	int fine_tune;    // Fine tuning offset
	int input;        // Input source
	int norm;
};


#endif
