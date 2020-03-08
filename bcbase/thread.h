#ifndef THREAD_H
#define THREAD_H

#include <pthread.h>

class Thread
{
private:
	static void* entrypoint(void *parameters);
protected:
	virtual void run() = 0;
public:
	Thread(int synchronous = 0, int realtime = 0);
	virtual ~Thread();
	void start();
	int end(pthread_t tid);           // end another thread
	int end();    // end this thread
	int join();   // join this thread
	int suspend_thread();   // suspend this thread
	int continue_thread();     // continue this thread
	int exit_thread();   // exit this thread
	int set_realtime();          // set to to schedule realtime
	int enable_cancel();
	int disable_cancel();
	int synchronous;         // set to 1 to force join() to end
	int realtime;            // set to 1 to schedule realtime
  	pthread_t tid;
};

#endif
