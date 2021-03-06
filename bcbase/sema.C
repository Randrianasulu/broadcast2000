#include <string.h>
#include "bcipc.h"
#include "sema.h"

Sema::Sema(int id, int number)
{
	if(id == -1)
	{
		if((semid = semget(IPC_PRIVATE, number, IPC_CREAT | 0777)) < 0) perror("Sema::Sema");
		for(int i = 0; i < number; i++) unlock(i);
		client = 0;
		bc_enter_sema_id(semid);
	}
	else
	{
		client = 1;
		this->semid = id;
	}

	semas = number;
}

Sema::~Sema()
{
	if(!client)
	{
		if(semctl(semid, 0, IPC_RMID, arg) < 0) perror("Sema::~Sema");
		bc_remove_sema_id(semid);
	}
}

	
int Sema::lock(int number)
{
	struct sembuf sop;
	
// decrease the semaphore
	sop.sem_num = number;
	sop.sem_op = -1;
	sop.sem_flg = 0;
	if(semop(semid, &sop, 1) < 0) perror("Sema::lock");
return 0;
}

int Sema::unlock(int number)
{
	struct sembuf sop;
	
// decrease the semaphore
	sop.sem_num = number;
	sop.sem_op = 1;
	sop.sem_flg = 0;
	if(semop(semid, &sop, 1) < 0) perror("Sema::unlock");
return 0;
}

int Sema::get_value(int number)
{
	return semctl(semid, number, GETVAL, arg);
return 0;
}

int Sema::get_id()
{
	return semid;
return 0;
}
