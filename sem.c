#include "cpu.h"
#include "sem.h"

int TS(int *memory) {
	if(*memory) return TRUE;
	else {
		*memory = TRUE;
		return FALSE;	
	}	
}

void P(semaphore *s){
	while(TS(&(s->mutex)));		//WAit
	s->value--;
}
 
void V(semaphore *s) {
//	if(&(s->mutex)) {
		s->value++;	
		s->mutex = FALSE;
//	}
}


void initSEM(semaphore *s){
	s->mutex = FALSE;	
	s->value = 0;
	s->hold  = TRUE;
}

void freeSEM(semaphore *s){
	s->mutex = FALSE;	
	s->value = 0;
	s->hold  = TRUE;
}

