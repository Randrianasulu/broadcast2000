#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include "thread.h"


Thread::Thread(int synchronous, int realtime)
{
	this->synchronous = synchronous; 
	this->realtime = realtime;  
	tid = (pthread_t)-1;
	thread_running = 0;
}

Thread::~Thread()
{
}

void* Thread::entrypoint(void *parameters)
{
	Thread *pt = (Thread*)parameters;
// allow thread to be cancelled in the middle of something
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, 0);
	pt->run();
	pt->thread_running = 0;
	return NULL;
}

void Thread::start()
{
	pthread_attr_t  attr;
	struct sched_param param;

	pthread_attr_init(&attr);

	thread_running = 1;
// caused SEGFLT when reading from files
	if(!synchronous) pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	if(realtime)
	{
		if(pthread_attr_setschedpolicy(&attr, SCHED_RR) < 0)
			printf("Couldn't set realtime thread.\n");
		param.sched_priority = 50;
		if(pthread_attr_setschedparam(&attr, &param) < 0)
			printf("Couldn't set realtime thread.\n");;
	}

	pthread_create(&tid, &attr, Thread::entrypoint, this);
}

int Thread::end(pthread_t tid)           // need to join after this if synchronous
{
	if(tid > 0) pthread_cancel(tid);
	return 0;
}

int Thread::end()           // need to join after this if synchronous
{
	cancel();
	return 0;
}

int Thread::cancel()
{
	if(tid > 0) pthread_cancel(tid);
	if(!synchronous) tid = (pthread_t)-1;
	return 0;
}

int Thread::join()   // join this thread
{
	if(tid > 0) pthread_join(tid, 0); 
	tid = (pthread_t)-1;
	return 0;
}

int Thread::enable_cancel()
{
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
	return 0;
}

int Thread::disable_cancel()
{
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	return 0;
}

int Thread::exit_thread()
{
 	pthread_exit(0);
	if(!synchronous) tid = (pthread_t)-1;
	return 0;
}


int Thread::suspend_thread()
{
	if(tid > 0) pthread_kill(tid, SIGSTOP);
	return 0;
}

int Thread::continue_thread()
{
	if(tid > 0) pthread_kill(tid, SIGCONT);
	return 0;
}

int Thread::running()
{
	return thread_running;
}

int Thread::set_synchronous(int value)
{
	this->synchronous = value;
	return 0;
}

int Thread::set_realtime(int value)
{
	this->realtime = value;
	return 0;
}

int Thread::get_synchronous()
{
	return synchronous;
}

int Thread::get_realtime()
{
	return realtime;
}


