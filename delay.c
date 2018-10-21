#include "jiffies.h"

void delay(int msec) {	//x00msec
	unsigned long extime;	
	extime = jiffies + msec*(HZ/10);
	while(extime > jiffies);
}
