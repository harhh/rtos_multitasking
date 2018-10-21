#ifndef __LIST_H
#define __LIST_H
typedef struct tagNode
{	h_tcb *tcb;
	int state;	
	struct tagNode *next;
}NODE;

extern NODE linkedList[TASK_MAX];

void list_init() ;

void add_node_sort(h_tcb * ready);

void del_node_sort(h_tcb * notRd);
#endif