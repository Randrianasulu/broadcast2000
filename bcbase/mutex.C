#include <string.h>
#include "mutex.h"

Mutex::Mutex()
{
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&mutex, &attr);
}

Mutex::~Mutex()
{
	pthread_mutex_destroy(&mutex);
}
	
int Mutex::lock()
{
	if(pthread_mutex_lock(&mutex)) printf("lock failed\n");
return 0;
}

int Mutex::unlock()
{
	if(pthread_mutex_unlock(&mutex)) printf("unlock failed\n");
return 0;
}

int Mutex::trylock()
{
	return pthread_mutex_trylock(&mutex);
return 0;
}

int Mutex::reset()
{
	pthread_mutex_destroy(&mutex);
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutex_init(&mutex, &attr);
return 0;
}
