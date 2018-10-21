#include "cpu.h"
#include "messageQueue.h"
#include "qsk_bsp.h"

MQueueINT index[CHANNEL];
char MQueue[CHANNEL][LENGTH+1];


int openMQ(int channel) {
	if(channel > CHANNEL) return FALSE;//not correct request
	
	if(MQueue[channel][0] < USING) {	//
		if(MQueue[channel][0]== FREE) {
			index[channel].rec = 1;
			index[channel].sen = 1;	
		}
		MQueue[channel][0]++;
		
		return TRUE;
	}
	index[channel].rec = 1;//
	index[channel].sen = 1;	//
	return TRUE;
}

void MQsen(char input, int channel){
	
	if(index[channel].messageCtr==LENGTH) return;
	DISABLE_IRQ;
	MQueue[channel][index[channel].sen] = (char)input;

	index[channel].sen++;
	if(index[channel].sen == LENGTH) index[channel].sen = 1;
	index[channel].messageCtr++;
	ENABLE_IRQ;
}

char MQrec(int channel){
	char ret;
	if(index[channel].messageCtr == 0) return FALSE;
	//if(--index[channel].sen == 0) {
	//	index[channel].sen == 1;
	//}
	DISABLE_IRQ;
	ret =  MQueue[channel][index[channel].rec];
	index[channel].rec++;
	if(index[channel].rec == LENGTH) index[channel].rec = 1;
	
	index[channel].messageCtr--;
	ENABLE_IRQ;
	return (char)ret;
	
}