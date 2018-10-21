#ifndef __TASKLIST_
#define __TASKLIST_

#define USING 	1
#define IDLE 	0

#include "task.h"
#include "taskList.h"

NODE linkedList[TASK_MAX];
NODE head;
NODE tail;

void list_init() {
	int i = 0;
	for(; i < TASK_MAX; i++) {
		linkedList[i].tcb = 0;
		linkedList[i].next = 0;
		linkedList[i].state = IDLE;	
	}
	head.next = &tail;	//head->tail
}

void add_node_sort(h_tcb * ready) {		///ready list(schduling)
	NODE *p = 0;
	int i = 0;

	p = &head;
	
	for(;i<TASK_MAX; i++) {					//find empty node
		if(linkedList[i].state ==IDLE)	{
			break;	
		}
	}
	linkedList[i].state = USING;
	linkedList[i].tcb = ready;		//ready tcb input ready (list)queue
	
	if(head.next == &tail) {			//no data
		head.next = &linkedList[i];	//head -> new -> tail
		linkedList[i].next = &tail;
	}
	else {
		while(p->next != &tail) {	
			if(p->next->tcb->priority > linkedList[i].tcb->priority) {
				linkedList[i].next = p->next;
				p->next = &linkedList[i];
				break;
			}
			p = p->next;
			if(p == &tail) {	//priority is lowest
				p->next = &linkedList[i];
				linkedList[i].next = &tail;	
				break;
			}
		}
	}
}

void del_node_sort(h_tcb * notRd) {
	NODE *p = 0;
	p = &head;
	
	while(p->next != notRd) {
		p = p->next;
	}	
	p->next->state = IDLE;
	p->next = p->next->next;
}


#endif