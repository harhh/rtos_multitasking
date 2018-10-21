#include "qsk_bsp.h"
#include "task.h"
#include "jiffies.h"
#include "app.h"
#include "sched.h"
#include "sem.h"
#include "messageQueue.h"

#pragma INTERRUPT Switch1IntHndl;
#pragma INTERRUPT Switch2IntHndl;

#define PATTERN		0
#define INFO	 	3

static int app_state;

int patternTerm = 2;

void Switch1IntHndl(void);
void Switch2IntHndl(void);

static T_STATE volatile state = 0;	//DORMANT, READY, RUNNING, WAITING 
static int task_cursor = 0;
static taskCtn = 9;// 9개 테스크

//for bottom half
static unsigned char bool_int1 = 0;
static unsigned char bool_int2 = 0;
static unsigned char bool_S3 = 0;

typedef struct {
	int tid;
	void (*fp)(void);
}tidnfp;

char ranchar[20] = {'!','@','#','$','%','^','&','*','(',')'
					,'-','=','+','A','B','C','D','E','F','G'};
static cursor_A = 0;
static cursor_B = 2;
static cursor_C = 4;
static cursor_D = 6;

const char num_to_char[10] = {'0','1','2','3','4','5','6','7','8','9'};

extern INT16U *_stk;

semaphore s1;	//for 1&2
semaphore s3;	//for 3&4
	
tidnfp app_task[9]; //[A, B, C, D, E, F, G, H, I]task's tid;

void app_init() {
	app_task[0].fp = &task_A;
	app_task[1].fp = &task_B;
	app_task[2].fp = &task_C;
	app_task[3].fp = &task_D;
	app_task[4].fp = &task_E;
	app_task[5].fp = &task_F;
	app_task[6].fp = &task_G;
	app_task[7].fp = &task_H;
	app_task[8].fp = &task_I;
}

void next_task()
{
	 task_cursor++;
	 if(task_cursor > taskCtn)
	 task_cursor = 1;
}

void Switch1IntHndl(void)
{
	bool_int1 =	1;
}

void Switch2IntHndl(void)
{
	bool_int2 = 1;
}

void switch_init()
{
	pd8_1 = 0;
	pd8_2 = 0;
	pd8_3 = 0;

	ENABLE_IRQ
	ilvl0_int0ic = 1;	//int0 ->priority : 001
	ilvl2_int1ic = 1;	//int1 ->priority
	ilvl1_int1ic = 1;	//int1 ->priority : 110
	pol_int0ic = 0;		//falling edge
	pol_int1ic = 0;		//falling edge
}	

void deviceDriver()	//switch event handling
{
	int tid;
	char buf[3];
	int i = 0;
	int j = 0;
	int ontask = 0;
	int readytask = 0;

	while(1) {
		//-------------------------S1
		if(bool_int1 > 0)
		{
			if(app_state == PATTERN) {	//goto INFO state
				for(;i<8; i++) {
					DISABLE_IRQ;
					LCD_write(CTRL_WR, 0x18);
					ENABLE_IRQ;			
				}
				i=0;
		
				for(; i < 8; i++){
					LCD_Char_write(1, i+8, ' ');
				}
				i=0;
				
				readytask = getReadyTask();//get Tid 0000 0000 0000
											//	   ..2109 8765 4321
				for(;i<taskCtn;i++) {
					if((readytask >> i) & 1) {
						for(;j<taskCtn;j++) {
							if(tasks[i+1].task == app_task[j].fp) {
								LCD_Char_write(1, ontask+8, num_to_char[j+1]);	
								ontask++;
								break;
							} 		
						}
						j=0;
					}
				}
				i=0;
				ontask = 0;
				app_state = INFO;
			}else if(app_state == INFO) {						//goto PATTERN state
				for(;i<8; i++) {
					DISABLE_IRQ;
					LCD_write(CTRL_WR, 0x1C);
					ENABLE_IRQ;	
				}
				i = 0;
				app_state = PATTERN;
			}			
			
			DISABLE_IRQ;
			bool_int1 = 0;	//TODO Sync
			ENABLE_IRQ;
		}	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		if(S3 == 0)	//priority up as soft
		{
			bool_S3 = 1;
		}
	
		//-------------------------S2
		if(bool_int2 > 0)
		{	
			if(app_state == PATTERN) {
				if(app_task[task_cursor-1].tid != 0){
					Task_terminate(app_task[task_cursor-1].tid);	
					app_task[task_cursor-1].tid = 0;	
				}else {
					tid = Task_create(app_task[task_cursor-1].fp, 100, _stk, 2, 1, 10);
					app_task[task_cursor-1].tid = tid;
				}
		//	Task_resume(task_cursor);
			}else if(app_state == INFO){
					if(	policy ==  DUMP_RR) {
					
						LCD_Char_write(2, 1+8, ' ');
						LCD_Char_write(2, 2+8, 'R');
						LCD_Char_write(2, 3+8, 'M');
						policy = SCHED_RM;
					}
					else if(policy == SCHED_RM) {
						LCD_Char_write(2, 1+8, 'E');
						LCD_Char_write(2, 2+8, 'D');
						LCD_Char_write(2, 3+8, 'F');
						policy = SCHED_EDF;
					}
					else if(policy == SCHED_EDF) {
						LCD_Char_write(2, 1+8, ' ');
						LCD_Char_write(2, 2+8, 'R');
						LCD_Char_write(2, 3+8, 'R');
						policy = DUMP_RR;
					
					}						
			}
			DISABLE_IRQ;	
			bool_int2 = 0;
			ENABLE_IRQ;
		}
	
		//-------------------------S3
		if(S3 == 0)
		{
			bool_S3 = 1;
		}
		else if(bool_S3 == 1){	//falling edge as soft
			if(app_state == PATTERN){
				next_task();
				DISABLE_IRQ
				LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + 7));
				LCD_write(DATA_WR, num_to_char[task_cursor]);	
				ENABLE_IRQ		
			}else {	///info
			//	.....
				
			}
			bool_S3 = 0;
		}
	}
}


void LCD_Char_write(int line, int offset, char p)
{
	DISABLE_IRQ
	if(line == 1) {
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L1 + offset));
		LCD_write(DATA_WR, p);	
	}else if(line == 2) {
		LCD_write(CTRL_WR, (unsigned char)(LCD_HOME_L2 + offset));
		LCD_write(DATA_WR, p);
	}else ;//false
	ENABLE_IRQ
}	

void task_A()
{
	int i = 0;
	char buf[2];
	int MQch = 0;
	
	initSEM(&s1);
	MQch = openMQ(1);

	while(1){
	i = rand()%21;
	
	buf[0] = ranchar[i];
	if(MQch) {
		MQsen(buf[0], MQch);	
	}
	buf[1] = 0;
	/////// *a*a*a*			display * every 0.3seconds  (*=random char)
	/////// a*a*a*a			
	P(&s1);
	LCD_Char_write(1, 0, buf[0]);
	delay(patternTerm);
	RED_LED ^= 1;	                // toggle LED ON-OFF-ON
	LCD_Char_write(2, 1, buf[0]);
	delay(patternTerm);	
	RED_LED ^= 1;	                // toggle LED ON-OFF-ON
	LCD_Char_write(1, 2, buf[0]);
	delay(patternTerm);
	RED_LED ^= 1;	                // toggle LED ON-OFF-ON
	LCD_Char_write(2, 3, buf[0]);
	delay(patternTerm);	
	RED_LED ^= 1;	                // toggle LED ON-OFF-ON
	LCD_Char_write(1, 4, buf[0]);
	delay(patternTerm);
	RED_LED ^= 1;	                // toggle LED ON-OFF-ON
	LCD_Char_write(2, 5, buf[0]);
	delay(patternTerm);	
	RED_LED ^= 1;	                // toggle LED ON-OFF-ON
	LCD_Char_write(1, 6, buf[0]);
	V(&s1);
	delay(patternTerm);
	RED_LED ^= 1;	                // toggle LED ON-OFF-ON	
	}
}

void task_B()
{
	
	initSEM(&s1);
	while(1){
	///////  a a a 			erase task_A pattern every 0.3seconds  (*=random char)
	/////// a a a a		
	P(&s1);	
	delay(patternTerm-1);
	LCD_Char_write(1, 0, ' ');
	YLW_LED ^= 1;	
	delay(patternTerm-1);
	LCD_Char_write(2, 1, ' ');
	YLW_LED ^= 1;	
	delay(patternTerm-1);	
	LCD_Char_write(1, 2, ' ');
	YLW_LED ^= 1;	
	delay(patternTerm-1);
	LCD_Char_write(2, 3, ' ');
	YLW_LED ^= 1;	
	delay(patternTerm-1);	
	LCD_Char_write(1, 4, ' ');
	YLW_LED ^= 1;	
	delay(patternTerm-1);
	LCD_Char_write(2, 5, ' ');
	YLW_LED ^= 1;	
	delay(patternTerm-1);	
	LCD_Char_write(1, 6, ' ');
	V(&s1);
	YLW_LED ^= 1;	
	delay(patternTerm-1);
	}
}


void task_C()
{
	int i = 0;
	char buf[2];
	
	int MQch = 0;
	
	MQch = openMQ(1);	
	
	initSEM(&s3);
	while(1){
		i = rand()%21;
	
		if(MQch == 0) {			//not open MQueue
			buf[0] = ranchar[i];
			buf[1] = 0;		
		}else{					//gain MQueue channel
			buf[0] = (char)MQrec(MQch);
			if(buf[0] == FALSE) {
				buf[0] = ranchar[i];
			}
			buf[1] = 0;		
		}
	
	/////// a*a*a*a			display * every 0.3seconds  (*=random char)
	/////// *a*a*a*			
	P(&s3);
	LCD_Char_write(2, 0, buf[0]);
	delay(patternTerm);
	LCD_Char_write(1, 1, buf[0]);
	delay(patternTerm);	
	LCD_Char_write(2, 2, buf[0]);
	delay(patternTerm);
	LCD_Char_write(1, 3, buf[0]);
	delay(patternTerm);	
	LCD_Char_write(2, 4, buf[0]);
	delay(patternTerm);
	LCD_Char_write(1, 5, buf[0]);
	delay(patternTerm);	
	LCD_Char_write(2, 6, buf[0]);
	V(&s3);
	delay(patternTerm);	
	}
}


void task_D()
{
	initSEM(&s3);
	while(1){
	/////// a a a a			erase task_A pattern every 0.3seconds  (*=random char)
	///////  a a a 	
			P(&s3);	
		delay(patternTerm-1);
		LCD_Char_write(2, 0, ' ');
		delay(patternTerm-1);
		LCD_Char_write(1, 1, ' ');
		delay(patternTerm-1);	
		LCD_Char_write(2, 2, ' ');
		delay(patternTerm-1);
		LCD_Char_write(1, 3, ' ');
		delay(patternTerm-1);	
		LCD_Char_write(2, 4, ' ');
		delay(patternTerm-1);
		LCD_Char_write(1, 5, ' ');
		delay(patternTerm-1);	
		LCD_Char_write(2, 6, ' ');
		delay(patternTerm-1);
		V(&s3);
		delay(patternTerm-1);
	}
}

void task_E()
{
	int i = 0;
	char buf[2];
	while(1){
	i = rand()%21;
	buf[0] = ranchar[i];
	buf[1] = 0;
	/////// ----->l				every 0.2seconds  (random char)
	/////// <-----l	
	LCD_Char_write(1, 0, buf[0]);
	delay(patternTerm-1);		
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(1, 1, buf[0]);
	delay(patternTerm-1);
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(1, 2, buf[0]);
	delay(patternTerm-1);	
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(1, 3, buf[0]);
	delay(patternTerm-1);
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(1, 4, buf[0]);
	delay(patternTerm-1);	
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(1, 5, buf[0]);
	delay(patternTerm-1);
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(1, 6, buf[0]);
	delay(patternTerm-1);	
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(2, 6, buf[0]);
	delay(patternTerm-1);	
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(2, 5, buf[0]);
	delay(patternTerm-1);	
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(2, 4, buf[0]);
	delay(patternTerm-1);	
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(2, 3, buf[0]);
	delay(patternTerm-1);	
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(2, 2, buf[0]);
	delay(patternTerm-1);	
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(2, 1, buf[0]);
	delay(patternTerm-1);	
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	LCD_Char_write(2, 0, buf[0]);
	delay(patternTerm-1);
	GRN_LED ^= 1;	                // toggle LED ON-OFF-ON	
	}
}

void task_F()
{
	int i = 0;
	char buf[2];
	while(1){
	i = rand()%21;
	buf[0] = ranchar[i];
	buf[1] = 0;
	/////// <-----l			every 0.2seconds  (random char)
	/////// ----->l	

	LCD_Char_write(2, 0, buf[0]);
	delay(patternTerm-1);		
	LCD_Char_write(2, 1, buf[0]);
	delay(patternTerm-1);
	LCD_Char_write(2, 2, buf[0]);
	delay(patternTerm-1);	
	LCD_Char_write(2, 3, buf[0]);
	delay(patternTerm-1);
	LCD_Char_write(2, 4, buf[0]);
	delay(patternTerm-1);	
	LCD_Char_write(2, 5, buf[0]);
	delay(patternTerm-1);
	LCD_Char_write(2, 6, buf[0]);
	delay(patternTerm-1);	
	LCD_Char_write(1, 6, buf[0]);
	delay(patternTerm-1);	
	LCD_Char_write(1, 5, buf[0]);
	delay(patternTerm-1);	
	LCD_Char_write(1, 4, buf[0]);
	delay(patternTerm-1);	
	LCD_Char_write(1, 3, buf[0]);
	delay(patternTerm-1);	
	LCD_Char_write(1, 2, buf[0]);
	delay(patternTerm-1);	
	LCD_Char_write(1, 1, buf[0]);
	delay(patternTerm-1);	
	LCD_Char_write(1, 0, buf[0]);
	
	delay(patternTerm-1);
	}
}


void task_G()
{
	int i = 0;
	char buf[2];

	while(1){
	i = rand()%21;
	buf[0] = ranchar[i];
	buf[1] = 0;
	/////// l<-l<-l<-l			every 0.1seconds  (random char)
	/////// l->l->l->l	

	LCD_Char_write(1, 6, buf[0]);
	delay(patternTerm-2);		
	LCD_Char_write(1, 5, buf[0]);
	delay(patternTerm-2);
	LCD_Char_write(2, 5, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(2, 6, buf[0]);
	
	delay(patternTerm-2);
	LCD_Char_write(1, 4, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(1, 3, buf[0]);
	delay(patternTerm-2);
	LCD_Char_write(2, 3, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(2, 4, buf[0]);
	
	delay(patternTerm-2);	
	LCD_Char_write(1, 2, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(1, 1, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(2, 1, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(2, 2, buf[0]);
	delay(patternTerm-2);	
	
	LCD_Char_write(1, 0, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(2, 0, buf[0]);
	delay(patternTerm-2);

	}
}

void task_H()
{
	int i = 0;
	char buf[2];

	while(1){
	i = rand()%21;
	buf[0] = ranchar[i];
	buf[1] = 0;
	/////// l->l->l->l			every 0.1seconds  (random char)
	/////// l<-l<-l<-l	
		
	LCD_Char_write(1, 0, buf[0]);
	delay(patternTerm-2);		
	LCD_Char_write(1, 1, buf[0]);
	delay(patternTerm-2);
	LCD_Char_write(2, 1, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(2, 0, buf[0]);
	
	delay(patternTerm-2);
	LCD_Char_write(1, 2, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(1, 3, buf[0]);
	delay(patternTerm-2);
	LCD_Char_write(2, 3, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(2, 2, buf[0]);
	
	delay(patternTerm-2);	
	LCD_Char_write(1, 4, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(1, 5, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(2, 5, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(2, 4, buf[0]);
	
	delay(patternTerm-2);	
	LCD_Char_write(1, 6, buf[0]);
	delay(patternTerm-2);	
	LCD_Char_write(2, 6, buf[0]);
	delay(patternTerm-2);
	}
}


void task_I()
{
	int i = 0;
	while(1){
	/////// 	black & white		every 5seconds 
	/////// 	
	LCD_write(CTRL_WR,LCD_CLEAR);
	LCD_Char_write(1, 7, num_to_char[task_cursor]);
	delay(patternTerm+47);		
	for(; i < 14; i++){
		if(i<7) LCD_Char_write(1, i, -1);
		if(i>6 && i<14) LCD_Char_write(2, i-7, -1);
	}
	i = 0;
	delay(patternTerm+47);	
	}	
}
