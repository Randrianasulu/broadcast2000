#ifndef PLUGINBUFFER_H
#define PLUGINBUFFER_H

#include "shmemory.h"

class PluginBuffer : public SharedMem
{
public:
	PluginBuffer(long size, int wordsize);
	PluginBuffer(int mem_id, long size, int wordsize);
	~PluginBuffer();

	void* get_data();        // returns pointer to the data

// the following aren't used in preference for the message blocking
// one of these must be run before and after every buffer access
	int lock_write();             // lock for writing
	int unlock_write();           // unlock for writing
	int lock_read();              // lock for reading
	int unlock_read();            // unlock for reading
	int set_attached(int attached = 1);

	int get_lock_id();

	//Sema *lock;
 	int WRITE_LOCK;
	int READ_LOCK;

	int attached;        // inform renderer if this buffer needs to be filled
	long wordsize;
	long size;
};

#endif
