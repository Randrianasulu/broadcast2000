#include <string.h>
#include "pluginbuffer.h"

PluginBuffer::PluginBuffer(long size, int wordsize) : SharedMem(size * wordsize)
{
// semas aren't being used
	//lock = new Sema(2);
	//lock_read();
	WRITE_LOCK = 0;
	READ_LOCK = 1;
	attached = 0;
	this->wordsize = wordsize;
	this->size = size;
}

PluginBuffer::PluginBuffer(int mem_id, long size, int wordsize) : SharedMem(mem_id, size * wordsize)
{
// semas aren't being used
	//lock = new Sema(lock_id, 2);
	attached = 0;
	this->wordsize = wordsize;
	this->size = size;
}

PluginBuffer::~PluginBuffer()
{
// semas aren't being used
	//delete lock;
}

int PluginBuffer::set_attached(int attached)
{
	this->attached = attached;
}

void* PluginBuffer::get_data()
{
	return (void*)data;
}

int PluginBuffer::lock_write()
{
	//lock->lock(WRITE_LOCK);
}

int PluginBuffer::unlock_write()
{
	//lock->unlock(WRITE_LOCK);
}

int PluginBuffer::lock_read()
{
	//lock->lock(READ_LOCK);
}

int PluginBuffer::unlock_read()
{
	//lock->unlock(READ_LOCK);
}

int PluginBuffer::get_lock_id()
{
// locks aren't used
	//return lock->get_id();
}
