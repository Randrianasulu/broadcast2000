#ifndef BCIPC_H
#define BCIPC_H

#include "arraylist.h"
#include "mutex.h"

extern ArrayList<int> global_shmem_db;
extern ArrayList<int> global_sema_db;
extern ArrayList<int> global_msg_db;

extern Mutex global_ipc_lock;

#include "bcipc.h"
#include <signal.h>

// These must be atomic routines

extern ArrayList<int> global_shmem_db;
extern ArrayList<int> global_sema_db;
extern ArrayList<int> global_msg_db;
extern Mutex global_ipc_lock;

int bc_init_ipc();
int bc_enter_shmem_id(int id);
int bc_remove_shmem_id(int id);
int bc_enter_sema_id(int id);
int bc_remove_sema_id(int id);
int bc_enter_msg_id(int id);
int bc_remove_msg_id(int id);

#endif
