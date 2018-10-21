#include "task.h"
#include "sched.h"
//#include "taskList.c"


int T[TASK_MAX];  	//for RM
int C[TASK_MAX]; 	//for RM
//INT8U TaskCtr;	define external task.h
//int policy;		define external task.h

int LCM = 0;		//the least common multiple


int Check_RMnEDF();
void updateLCM();
void time_count(h_tcb* Runtask) ;
int sched_rm(h_tcb *tasks, int *current);
void dump_RR_sched(int *current);
int T[TASK_MAX];
int C[TASK_MAX]; 

h_tcb *next;
int curTid;

int Check_RMnEDF(){
	int i;
	float cpu_use = 0;
	double invers, rm, result;
	
	//cal cpu utilization
	for(i=0;i<TaskCtr; i++) {
		cpu_use +=((float)C[i]/(float)T[i]);	
	}
	
	invers = 1/(double)TaskCtr;
	rm = pow(2.0, invers);
	result = TaskCtr * (rm-1);
	if(cpu_use <= 1)
		if(cpu_use<result)	return SCHED_RM;	//rm schedulable
		
		else	return SCHED_EDF;				//rm not schedulable but schedulable another policy
	else	return -1;
		
}

void updateLCM() {
	int i, j, a, b;
	for(i=0;i<TaskCtr; i++) {
		if(T[i]>T[i+1]) {
			a = T[i];
			b = T[i+1];	
		}	
		else {
			a = T[i+1];
			b = T[i];	
		}
		
		for(j=1; j<=b; j++) {
			if(((a*j)%b)==0)	{
				LCM=a*j;
				break;	
			}	
		}
	}	
}


void time_count(h_tcb* Runtask) {
	h_tcb* cur;
	int i = 0;
//	cur = &tasks[0];	//first task

	for(; i<TASK_MAX;i++) {
		cur = &tasks[i];
		cur->C_time++;	//C_time is passed
		
		if(policy == SCHED_EDF) cur->priority--;	//for EDF -1 priority
		
		
		//cur task C_time == period and 
		if(cur->C_time == cur->period) {
			if(cur->state == READY) {
				if(!(cur == Runtask	&& Runtask->wcet == (Runtask->R_time+1)))
				;//......not act
			}
			cur->C_time = 0;
			cur->state = READY;
		}
		while( i!=TASK_MAX-1 && tasks[i+1].state == DORMANT) i++;	//loop all tasks(wait and ready) (not dormant)
	}
	
	Runtask->R_time++;
	if(policy == SCHED_RM) {
		if(Runtask->R_time == Runtask->wcet) {
			if(Runtask->R_time <= Runtask->C_time) Runtask -> state = WAITING;
			Runtask->R_time = 0;
		}
	}else if(policy == SCHED_EDF) {
		if(Runtask->R_time == Runtask->wcet) {
			if(Runtask->R_time <= Runtask->C_time)  {
				Runtask -> state = WAITING;
				Runtask -> priority = Runtask->period;
				}
			else {
				Runtask -> priority = Runtask->period - Runtask->C_time - Runtask->R_time;
			}
			Runtask->R_time = 0;
		}
	}
	
}

int sched_rm(h_tcb *tasks, int *current)
{
	int i = 0;
	int j = 0;
	int top;
	h_tcb *run;
	h_tcb *cur;
	cur = &tasks[0];
	//Check_RMnEDF();
	//updateLCM();
	
	//for(; i<TASK_MAX;i++) {
	//	if(tasks[i].state != DORMANT) { 
	//		cur = &tasks[i];
	//		break;
	//	}
		
	
	cur = &tasks[0];
	curTid = 0;
	run = 0;
	
	for(; curTid<TASK_MAX;curTid++) {
		cur = &tasks[curTid];
		if(cur == 0) break;
		if(cur->state == READY) {
			top = cur->priority;	//top<-save prioriry	
			run = cur;
			break;
		}	
		while(curTid!=TASK_MAX-2 && tasks[curTid+1].state == DORMANT) curTid++;	//next Tid = curTid + 1
	}
	
	//if(run == 0) {
	//	time_count(run);
	//	continue;	
	//}
	
	//while(curTid!=TASK_MAX-2 && tasks[curTid+1].state == DORMANT) curTid++;	//next Tid = curTid + 1
	//next = &task[curTid+1];
	
	for(; curTid<TASK_MAX-1;curTid++) {
		while( curTid!=TASK_MAX-2 && tasks[curTid+1].state == DORMANT) i++;	//loop all tasks(wait and ready) (not dormant)
		next = &tasks[curTid+1];
		cur = next;
		if(cur == 0) break;
		if(top < cur->priority) 	continue;
		if(cur->state == WAITING)	continue;
		top = cur->priority;
		run = cur;
	}
	time_count(run);
	
	return run->tid;
	////////////////////	while(1)	
}

void dump_RR_sched(int *current) {
	*current = (++(*current) % TASK_MAX);
	while(tasks[*current].state != READY)
		*current = (++(*current) % TASK_MAX);
}



/*int admission_rm(h_tcb tasks[])
{
	int tid = 0;
	int addmission_sum = 0;
	while(tid < TASK_MAX){
		addmission_sum += tasks[tid]->wcet/tasks[tid]->priority;
		tid ++;	
	}
	if (addmission_sum < TaskCtr*(2^(1/TaskCtr)-1)) return 1;
	else return 0;
}*/