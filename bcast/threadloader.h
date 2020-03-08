#ifndef THREADLOADER_H
#define THREADLOADER_H

#include "mainwindow.inc"
#include "thread.h"

// ================================= loads files as a thread

class ThreadLoader : public Thread
{
public:
	ThreadLoader(MainWindow *mwindow);
	~ThreadLoader();
	
	int set_paths(ArrayList<char *> *paths);
	void run();
	MainWindow *mwindow;
	ArrayList<char *> *paths;
};

#endif
