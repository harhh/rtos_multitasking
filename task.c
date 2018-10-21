#include "task.h"
#include "jiffies.h"
#include "qsk_bsp.h"
#include "sched.h"
#include "taskList.h"

#define EVENTCOUNT_CONFIG	0x01		
#define TIMER_CONFIG		0x40  
				/*  01000000 value to load into timer mode register
                    ||||||||_  TMOD1,TMOD0: TIMER MODE SELECTED
                    ||||||____ MR0:         NO PULSE OUTPUT   
                    |||||_____ MR2,MR1:     GATE FUNCTION NOT SELECTED
                    |||_______ MR3:         SET TO 0 IN TIMER MODE     
                    ||________ TCK1,TCK0:   F DIVIDED BY 8 SELECTED */
#define  CNTR_IPL 0x03   // TA1, TA2 priority interrupt level				

#define STACKSLICE 100

h_tcb tasks[TASK_MAX];
INT8U TaskCtr;
INT16U *_stk;

static int topTid=0;

int time_cnt;
//int count;		// Global count value, incremented every second
int current = -1;
static using_stsize = 0;
static int stack_arr[STACK_MAX/STACKSLICE];	//0~STACKSLICE, STACKSLICE~2*STACKSLICE

int policy;

unsigned long volatile jiffies;
/*
typedef struct{
	int stack;
	int tid;
}stackntid;
*/
void cpp(INT16U* p);
void sched_init();
void ta2_irq (void);
void ta1_irq (void);
void lcd_init(void);
void led_init(void);
void sched();
void task_A();
void task_B();
void task_C();
void task_D();
void itoa(unsigned short num, char *buf);
int absi(int i);
long absl(long i);
void do_timer();

void cpp(INT16U* p) {
	_stk = p;
}
	
void do_timer()
{
	jiffies++;	
}

void sched_init()
{
	//TA1 
	ta1 = 1-1;	// 5ms * 200 = 1second
	DISABLE_IRQ				// disable irqs before setting irq registers - macro defined in skp_bsp.h
	ta1ic = CNTR_IPL;		//set priority
	ta1mr |= EVENTCOUNT_CONFIG;	//set Mode Register
	ta1tgh = 1;	ta1tgl = 1;		//Trigger Select Register(TA2 overflow count)
	ENABLE_IRQ				// enable interrupts macro defined in skp_bsp.h
	
	//TA2
	ta2 = (unsigned int) (((f1_CLK_SPEED/8)*CYCLE*1e-3) - 1); //set data register (5ms)
	DISABLE_IRQ				// disable irqs before setting irq registers - macro defined in skp_bsp.h
	ta2ic = CNTR_IPL;		//set priority
	ta2mr |= TIMER_CONFIG;	//set Mode Register	
	ENABLE_IRQ				// enable interrupts macro defined in skp_bsp.h
	/*Count Start Register*/
	ta1s = 1;
	ta2s = 1;		
}
/*
#pragma INTERRUPT ta2_irq 
void ta2_irq (void)		//for 5ms
{
	

  	if ((time_cnt += 5) > (1000)){   	// = 1 second
  	//	YLW_LED ^= 1;	                // toggle LED ON-OFF-ON
    //	count++;			    		// example 1 second "clock" 
    //	time_cnt = 0;
		
	
	\
}
*/

#pragma INTERRUPT ta1_irq 	//for 1second
void ta1_irq (void)
{
		
/*	if(current == -1) {
		current = 0;
		tasks[current].state = RUNNING;	
	}else {
		tasks[current].state = READY;
		SAVE_ISP(s_stk);
		tasks[current].isp = s_stk;
		
		current = (++current % TASK_MAX);
		while(tasks[current].state != READY)
			current = (++current % TASK_MAX);
				
		tasks[current].state = RUNNING;
	}
	s_stk = tasks[current].isp;
	RESTORE_ISP(s_stk);*/

	INT16U *s_stk = 0;
	DISABLE_IRQ
	do_timer();
	
	if(current == -1) {
		current = 0;
	}else {
		tasks[current].state = READY;
		SAVE_ISP(s_stk);
		tasks[current].isp = s_stk;
		//if(policy == DUMP__RR)
		dump_RR_sched(&current);
		//elseif(policy==SCHED_RM)//
		//elseif(policy==SCHED_EDF)//
	}
	tasks[current].state = RUNNING;
	s_stk = tasks[current].isp;

	RESTORE_ISP(s_stk);
	ENABLE_IRQ
	
/////////////////////////////	
/*	if(current == -1) {
		current = 0;
	}else {
		tasks[current].state = READY;
		SAVE_ISP(s_stk);
		tasks[current].isp = s_stk;
		
		current = (++current % TASK_MAX);
		while(tasks[current].state != READY)
			current = (++current % TASK_MAX);
	}
	tasks[current].state = RUNNING;
	s_stk = tasks[current].isp;
	RESTORE_ISP(s_stk);*/
//////////////////////////////////

}

void lcd_init(void)
{	
	InitDisplay();				// Initialize LCD
	DisplayString(LCD_LINE2 + 8,"  RR X");

}  

void led_init(void)
{
	ENABLE_LEDS	
}


void itoa(unsigned short num, char *buf)
{
	unsigned short num_a = num%10;
	unsigned short num_b = num/10;
	char a = num_a +'0';
	char b = num_b +'0';

	buf[0] = a;	
	buf[1] = b;
	buf[2] = 0;
}

int absi(int i)
{
	int t = i >> 15;
	return (i ^ t) + (t & 1);
}

long absl(long i)
{
	long t = i >> 31;
	return (i ^ t) + (t & 1);
}

void Task_run(INT8U tid) 
{
	T_STATE stat = tasks[tid].state;
	if(stat == WAITING) tasks[tid].state = READY;
}

void Task_resume(INT8U tid) 
{
	T_STATE stat = tasks[tid].state;
	if(stat == WAITING) {
		tasks[tid].state = READY;
		add_node_sort(&tasks[tid]);	//input ready queue(list);
	}
}

void Task_terminate(INT8U tid)
{
	int i = 0;
	T_STATE stat = tasks[tid].state;	
	if(stat != DORMANT) {			//possible dormant
		tasks[tid].state = DORMANT;
		TaskCtr--;
		using_stsize -= tasks[tid].st_size;
		for(; i < STACK_MAX/STACKSLICE; i++) {
			if(stack_arr[i] == tid) stack_arr[i] = -1;
		}
	//	del_node_sort(&tasks[tid]);
		//....
	 	T[tid] = 0;  	//for RM
		C[tid] = 0; 	//for RM
	}
}

void Task_pause(INT8U tid) {
	T_STATE stat = tasks[tid].state;
	if(stat == READY) tasks[tid].state = WAITING;
}
 
int Task_create(void (*task)(void), int stack_size,INT16U *stbase, int priority, int wcet, int period) {
	int tid = FALSE;
	int st_size = 0;
	int stack_offset = 0;
	st_size = stackSize(stack_size);	//sampling
		
	if(TaskCtr >= TASK_MAX) return FALSE;	//Under MAX_TASK
	else if(using_stsize+stack_size > STACK_MAX || stack_size <= 0) return FALSE;	//stak overflow detected
	 
	else{	//possible init_task
		for(; tid < topTid + 1; tid++) {
			if(tasks[tid].state == DORMANT) break;	//get Tid
		}	
		stack_offset = (stack_arr_check(st_size, tid)+1)*STACKSLICE;
		task_init(task, _stk-(stack_offset), st_size, tid, priority, wcet, period);//	
		using_stsize += st_size;	//stack++
		TaskCtr++;					//taskctr++
	//	add_node_sort(&tasks[tid]);
		if(topTid < tid) topTid = tid;
	}
	 return tid;
}


void getTask(INT8U tid) {
	int i = 0;
//	task[tid].
}
 
void task_init(void (*task)(void), INT16U *stbase, int stack_size, int tid, int priority, int wcet, int period)
{
	h_tcb *tcb = 0;
	INT16U flag = 0x0040;
	stbase--;
	*stbase-- = (flag & 0x00FF)
				| (((INT32U)task >> 8) & 0x00000F00)
				| ((flag << 4) & 0xF000);			//(flag) -- (flag << 4)
	*stbase-- = (((INT32U)task) & 0x0000FFFF);				
	*stbase-- = (INT16U) 0; //A1
	*stbase-- = (INT16U) 0; //A0
	*stbase-- = (INT16U) 0; //R3
	*stbase-- = (INT16U) 0; //R2
	*stbase-- = (INT16U) 0; //R1
	*stbase-- = (INT16U) 0; //R0
	*stbase = (INT16U) 0; //FB
	
	tcb = &tasks[tid];
	tcb->isp = stbase;
	tcb->tid = tid;
	tcb->state = READY;
	tcb->st_size = stack_size;
	tcb->task = task;
	tcb->priority = priority;
	tcb->wcet = wcet;
	tcb->period = period;
	C[tid] = wcet;
	T[tid] = period;
	tcb->C_time = 0;
	tcb->R_time = 0;
	
///	add_node_sort(&tasks[tid]);	//input ready queue(list);
}

int stackSize(int size){	//1~50 ->50//51~100->100 ......
	if(size < 0) return FALSE;
	return (((size-1)/STACKSLICE)+1) *STACKSLICE; 
	
}

int stack_arr_check(int stackSize, int tid) {
	int i =0;
	for(; i < STACK_MAX/STACKSLICE; i++) {
		if(stack_arr[i] == -1) {
			stack_arr[i] = tid;
			break;
		}
	}
	return i;	
}

/*	//stack  스택 위치 설정 for each size
int stack_arr_check(int stackSize, int tid){
	int i = 0;
	int count = 0;
	int start = 0;
	int st = 0;
	
	st = stackSize / STACKSLICE;
	for(; i < STACK_MAX/STACKSLICE; ) {
		if(stack_arr[i] == -1) {	
			start = i;
			count ++;
			if(count == st) break;
			i++;
			for(; i < st; i++) {
				if(stack_arr[i] == -1) {
					count ++;	
				}else {count = 0; break;}
			}	
		}
		i++;
	}
	if(count == st && count != 0) {
		i = 0;
		if(st == 1) stack_arr[start] = tid;
		else {
			for(; i < st; i++) {
				stack_arr[start+i] = tid;	
			}	
		}	
	}
	return start;
}*/
void init_stack_arr() {
	int i = 0;
	for(; i < STACK_MAX/STACKSLICE; i++) {
		stack_arr[i] = -1;
	}	
}

INT8U getTaskCtr(){
	return TaskCtr;
}

int getReadyTask() {	//////0 0000 0000
						//////I HGFE DCBA  task
	int	bitmask = 0;
	int tid = 1;
	for(; tid <= 10; tid++) {
		if(tasks[tid].state == READY) {
			bitmask |= (1 << (tid-1));	
		}
	}
	return bitmask;	
}


