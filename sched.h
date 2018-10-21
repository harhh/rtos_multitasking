#ifndef _KURTOS_SCHED_H
#define _KURTOS_SCHED_H

#include "task.h"

#define DUMP_RR			0	//
#define SCHED_RM		1
#define SCHED_EDF		2	//


/*static inline int rm_policy(int policy)
{
	return policy == SCHED_RM;
}
*/
void dump_RR_sched(int *current);



#include "task.h"
#include "sched.h"
//#include "taskList.c"


extern int T[TASK_MAX];  	//for RM
extern int C[TASK_MAX]; 	//for RM
//INT8U TaskCtr;	define external task.h
//int policy;		define external task.h



int Check_RMnEDF();

void updateLCM();

void time_count(h_tcb* Runtask) ;

int sched_rm(h_tcb *tasks, int *current);
void dump_RR_sched(int *current);
extern int T[TASK_MAX];
extern int C[TASK_MAX]; 


#endif



