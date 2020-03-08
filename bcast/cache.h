#ifndef CACHE_H
#define CACHE_H

// cache for quickly reading data that is hard to decompress yet used
// over and over
// actual caching is done in the File object
// the Cache keeps files open while rendering

#include "assets.inc"
#include "cache.inc"
#include "file.inc"
#include "linklist.h"
#include "mainwindow.inc"
#include "mutex.h"

class CacheItem : public ListItem<CacheItem>
{
public:
	CacheItem(Cache *cache, Asset *asset);
	CacheItem() { };
	~CacheItem();

	long get_memory_usage();

	File *file;
	long counter;     // number of age calls ago this asset was last needed
	                  // assets used in the last render have counter == 1
	Asset *asset;
	Mutex item_lock;
	int checked_out;
private:
	Cache *cache;
};

class Cache : public List<CacheItem>
{
public:
	Cache(MainWindow *mwindow);
	~Cache();

// get a file from the cache 

// open it, lock it and add it to the cache if it isn't here already
	File* check_out(Asset *asset);

// unlock a file from the cache
	int check_in(Asset *asset);

// delete an entry from the cache
// before deleting an asset, starting a new project or something
	int delete_entry(Asset *asset);

// increment counters after rendering a buffer length
// since you can't know how big the cache is until after rendering the buffer
// deletes oldest assets until under the memory limit
	int age_audio();
	int age_video();

	int dump();

// Number of items to store in cache.
// Bytes was the original measuring unit.
	MainWindow *mwindow;

private:
	int delete_oldest();        // returns 0 if successful
	                        // 1 if nothing was old
	int age_type(int data_type);
	long get_memory_usage();

// for deleting items
	int lock_all();
	int unlock_all();

// to prevent one from checking the same asset out before it's checked in
// yet without blocking the asset trying to get checked in
// use a seperate mutex for checkouts and checkins
	Mutex check_in_lock, check_out_lock, total_lock;
};








#endif
