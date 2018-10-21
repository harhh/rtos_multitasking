
#define CHANNEL		2
#define LENGTH		4
#define FREE 		0
#define USING 		2

typedef struct{
	int rec;
	int sen;
	int messageCtr;
}MQueueINT;

int openMQ(int channel) ;
void MQsen(char input, int channel);

char MQrec(int channel);
