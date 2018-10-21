
typedef struct{
	int value;
	int mutex;
	int hold;
}semaphore;

int TS(int *memory);

void initSEM(semaphore *s);
void P(semaphore *s);
 
void V(semaphore *s);