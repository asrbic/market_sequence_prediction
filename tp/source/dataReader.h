#include"sema.h"

/*#ifndef 
#include<stdio.h>
#include<stdlib.h>
#include<pthread.h>
#include<unistd.h>
*/

#include<stdint.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
#include<string.h>

typedef struct patchBuffer
{
	int* patches;
	int* tempPatch;	
		 
	int head;
	int tail;
	int fill;
	int size;
	
	Semaphore e,s,n;
	
	pthread_mutex_t contGuard;
	int cont;	//flag which will terminate the writer thread
	
	int patchSize;	//num longs in each patch
	
	char currType;
	int patchesLeft;
	
	pthread_t tid;
	
	FILE* patchFile;
	
	int sizeX, sizeY, sizeZ; //dimensions of input region
} patchBuffer;

void initPatchBuffer(patchBuffer* b, int size, int fill, int sizeX, int sizeY, int sizeZ, char* patchDir);

void startProducer(patchBuffer* b);

void consume(patchBuffer* b, float* patchDest);

void stopProduction(patchBuffer* b);
