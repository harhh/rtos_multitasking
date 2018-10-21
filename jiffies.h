#ifndef _KURTOS_JIFFIES_H
#define _KURTOS_JIFFIES_H

#define HZ 100

#if HZ == 500
	#define CYCLE 2		//2ms
#elif HZ == 250
	#define CYCLE 4		//4ms
#elif HZ == 200
	#define CYCLE 5		//5ms
#elif HZ == 100
	#define CYCLE 10	//10ms
#endif	

extern unsigned long volatile jiffies;

void delay(int);//x00ms

#endif