#ifndef SEMA_H
#define SEMA_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>

union semun {
   int val; /* used for SETVAL only */
   struct semid_ds *buf; /* for IPC_STAT and IPC_SET */
   ushort *array;  /* used for GETALL and SETALL */
};

class Sema
{
public:
	Sema(int id = -1, int number = 1);
	~Sema();

	int lock(int number = 0);
	int unlock(int number = 0);
	int get_value(int number = 0);
	int get_id();

	int semid;
	int client;
	int semas;
	semun arg;
};







#endif
