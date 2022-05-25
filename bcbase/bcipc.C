#include <string.h>
#include "bcipc.h"
#include "sema.h"
#include <signal.h>
#include <sys/sem.h>
#include <sys/msg.h>
#include <sys/shm.h>

ArrayList<int> global_shmem_db;
ArrayList<int> global_sema_db;
ArrayList<int> global_msg_db;
Mutex global_ipc_lock;

// These must be atomic routines

int bc_enter_id(ArrayList<int>* list, int id);
int bc_remove_id(ArrayList<int>* list, int id);

void bc_ipc_termination(int signum)
{
	int i;
	union semun arg;

	global_ipc_lock.lock();
	for(i = 0; i < global_shmem_db.total; i++)
	{
		if(!shmctl(global_shmem_db.values[i], IPC_RMID, NULL))
		{
//			printf("Deleted shared memory %d\n", global_shmem_db.values[i]);
		}
	}

	for(i = 0; i < global_sema_db.total; i++)
	{
		if(!semctl(global_sema_db.values[i], 0, IPC_RMID, arg)) 
		{
//			printf("Deleted semaphore %d\n", global_sema_db.values[i]);
		}
	}

	for(i = 0; i < global_msg_db.total; i++)
	{
		if(!msgctl(global_msg_db.values[i], IPC_RMID, NULL)) 
		{
//			printf("Deleted message %d\n", global_msg_db.values[i]);
		}
	}

	if(global_shmem_db.total || global_sema_db.total || global_msg_db.total)
		printf("Crash\n");
	global_shmem_db.remove_all();
	global_sema_db.remove_all();
	global_msg_db.remove_all();
	global_ipc_lock.unlock();

	exit(0);
}

int bc_init_ipc()
{
	if(signal(SIGSEGV, bc_ipc_termination) == SIG_IGN)
		signal(SIGSEGV, SIG_IGN);
	if(signal(SIGBUS, bc_ipc_termination) == SIG_IGN)
		signal(SIGBUS, SIG_IGN);
	if(signal(SIGKILL, bc_ipc_termination) == SIG_IGN)
		signal(SIGKILL, SIG_IGN);
	if(signal(SIGINT, bc_ipc_termination) == SIG_IGN)
		signal(SIGINT, SIG_IGN);
	if(signal(SIGHUP, bc_ipc_termination) == SIG_IGN)
		signal(SIGHUP, SIG_IGN);
	if(signal(SIGTERM, bc_ipc_termination) == SIG_IGN)
		signal(SIGTERM, SIG_IGN);
return 0;
}


int bc_enter_shmem_id(int id)
{
	return bc_enter_id(&global_shmem_db, id);
}

int bc_remove_shmem_id(int id)
{
	return bc_remove_id(&global_shmem_db, id);
}

int bc_enter_sema_id(int id)
{
	return bc_enter_id(&global_sema_db, id);
}

int bc_remove_sema_id(int id)
{
	return bc_remove_id(&global_sema_db, id);
}

int bc_enter_msg_id(int id)
{
	return bc_enter_id(&global_msg_db, id);
}

int bc_remove_msg_id(int id)
{
	return bc_remove_id(&global_msg_db, id);
}

int bc_enter_id(ArrayList<int>* list, int id)
{
	int i, result = 0;
	global_ipc_lock.lock();
	for(i = 0; i < list->total; i++)
	{	
		if(list->values[i] == id) result = 1;
	}
	if(!result) list->append(id);
	global_ipc_lock.unlock();
	return 0;
}

int bc_remove_id(ArrayList<int>* list, int id)
{
	int i;
	global_ipc_lock.lock();
	for(i = 0; i < list->total; i++)
	{
		if(list->values[i] == id) list->remove_number(i);
	}
	global_ipc_lock.unlock();
	return 0;
}

