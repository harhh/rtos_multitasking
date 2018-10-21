#ifndef _KURTOS_TASK_H
#define _KURTOS_TASK_H

#include "cpu.h"

#define SAVE_ISP(x)\
		{	_asm("STC FB, $$[FB]", x);}
#define RESTORE_ISP(x)\
		{_asm("LDC $$[FB], ISP",x);\
		_asm("POPM FB");\
		_asm("POPM R0,R1,R2,R3,A0,A1");\
		_asm("REIT");}

#define TASK_MAX 20
#define STACK_MAX 400	//stack for task

typedef enum T_STATE {DORMANT, READY, RUNNING, WAITING} T_STATE;

typedef struct h_tcb_{
	INT16U *isp;
	INT8U tid;
	
	void (*task)(void);
	int st_size;
	
	T_STATE state;
	
	INT8U priority;	//(0 == highest)
	int period;//period
	int wcet; //wcet
	int C_time;	//
	int R_time;	//
	
	struct h_tcb_ *next;
}h_tcb;


extern INT8U TaskCtr;
extern h_tcb tasks[TASK_MAX];
extern int policy;

/*#define SAVE_ISP(x)\
		{	_asm("fclr i");\
			_asm("STC FB, $$[FB]", x);\
			_asm("fset i");}
#define RESTORE_ISP(x)\
		{_asm("fclr i");\
		_asm("LDC $$[FB], ISP",x);\
		_asm("POPM FB");\
		_asm("POPM R0,R1,R2,R3,A0,A1");\
		_asm("fset i");\
		_asm("REIT");}
		//_asm("POPM FB");\*/
		
		//		_asm("POPM R0,R1,R2,R3,A0,A1,FB");\
		//
void sched_init();
//#pragma INTERRUPT ta2_irq 
//void ta2_irq (void);
#pragma INTERRUPT ta1_irq 	//for 1second
void ta1_irq (void);
void lcd_init(void);
void led_init(void);
void task_init(void (*task)(void), INT16U *stbase, int stack_size, int tid, int priority, int wcet, int period);
void itoa(unsigned short num, char *buf);

void Task_run(INT8U tid);

void Task_resume(INT8U tid);
void Task_terminate(INT8U tid);
void Task_pause(INT8U tid);
int getReadyTask();

int Task_create(void (*task)(void), int stack_size,INT16U *stbase, int priority, int wcet, int period);
void cpp(INT16U* p);
void init_stack_arr();
#endif
