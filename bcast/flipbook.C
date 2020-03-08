#include <string.h>
#include "flipbook.h"
#include "mainwindow.h"


FlipBook::FlipBook(MainWindow *mwindow)
 : BC_MenuItem("Flipbook...")
{
	this->mwindow = mwindow;
	thread = new FlipBookThread(mwindow);
}

FlipBook::~FlipBook()
{
	delete thread;
}

FlipBook::handle_event()
{
	thread->start();
}





FlipBookThread::FlipBookThread(MainWindow *mwindow)
 : Thread()
{
	this->mwindow = mwindow;
}

FlipBookThread::~FlipBookThread()
{
}

void FlipBookThread::::run()
{
	
	ArrayList<FlipBookItem*> flipbooklist;
	FlipBookGUI gui(this, mwindow);
	
	int result = gui.run_window();
}





FlipBookGUI::FlipBookGUI(FlipBookThread *thread, MainWindow *mwindow)
 : BC_Window()
{
	this->thread = thread;
	this->mwindow = mwindow;
}

FlipBookGUI::~FlipBookGUI()
{
}
