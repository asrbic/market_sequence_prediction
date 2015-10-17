#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>

typedef struct Semaphore
{
	pthread_mutex_t m;
	pthread_cond_t cond;
	int val;
} Semaphore;

void initSem(Semaphore* sem, int value);
void destroySem(Semaphore* sem);
void procure(Semaphore* sem);
void vacate(Semaphore* sem);
void beginCritical(pthread_mutex_t* m);
void endCritical(pthread_mutex_t* m);

