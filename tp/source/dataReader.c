#include "dataReader.h"

#include "stdio.h"

void* producerThread(void* data);

void produce(patchBuffer* b);

void setCont(patchBuffer* b, int val);
int readCont(patchBuffer* b);

void destroypatchBuffer(patchBuffer* b);


void writeTestFile(char* fileDir)
{
	int i;
	int setSize=20;
	int data[setSize];
	FILE* temp = fopen("./datafile","wb");
	for(i=0;i<setSize;++i)
		data[i]=i;
	fwrite(data,sizeof(int),setSize,temp);
	fclose(temp);
}


/*int main(void)
{
	//writeTestFile("./datafile");
	
	patchBuffer b;
	
	initpatchBuffer(&b,10,0,1,1,1,"./datafile");
	startProducer(&b);
	int temp=0;
	char continueFlag = 'b';
	while(continueFlag!='c')
	{
		consume(&b,&temp);
		printf("consumed number:%i\n",temp);
		continueFlag = getchar();

	}
	stopProduction(&b);
}/**/


void startProducer(patchBuffer* b)
{
	printf("starting producer thread.\n");
	b->cont=1;
	if(pthread_create(&(b->tid),NULL,producerThread,b))
	{
		perror("dataReader - thread create error");
		exit(EXIT_FAILURE);
	}
}

void* producerThread(void* data)
{
	patchBuffer* b = (patchBuffer*)data;
	float tempPatch[b->patchSize];
	
	while(readCont(b))	//while thread is not set to terminate
	{
		produce(b,&tempPatch);
	}
	return NULL;
}

void produce(patchBuffer* b, float* tempPatch)
{
	//fprintf(stderr, "producing...\n");
	
	//used instead of the fscanf loop - binary fread is infinitely faster
	fread(tempPatch,sizeof(float),b->patchSize,b->patchFile);
	
	//lock patchBuffer
	procure(&b->e);//dec
	procure(&b->s);
	//BEGIN CRITICAL ZONE

	if(!readCont(b))
	{//terminate production
		return;
	}
	//copy temporary patch into patchBuffer
	memcpy((b->patches)+(b->head*b->patchSize), tempPatch, b->patchSize*sizeof(float));
	//increment head pointer
	b->head++;
	b->head=b->head%b->size;
	
	//END CRITICAL ZONE
	//unlock patchBuffer
	vacate(&b->s);//inc
	vacate(&b->n);
	//fprintf(stderr, "production complete.\n");
	//free(tempPatch);
}

void consume(patchBuffer* b, float* patchDest)
{
	//fprintf(stderr,"attempting to consume data.\n");
	if(!readCont(b))
	{
		fprintf(stderr,"consume failed: patchBuffer not filled\n");
		return -1;
	}
	
	//lock patchBuffer
	procure(&b->n);
	procure(&b->s);
	//BEGIN CRITICAL ZONE
	
	//copy patchBuffer contents into patchDest
	memcpy(patchDest, (b->patches)+(b->tail*b->patchSize), b->patchSize);
	
	//increment tail pointer
	b->tail++;
	b->tail=b->tail%b->size;
	
	//END CRITICAL ZONE
	//unlock patchBuffer
	vacate(&b->s);
	vacate(&b->e);
	
}

void stopProduction(patchBuffer* b)
{
	printf("Closing producer thread...\n");
	setCont(b,0);
	vacate(&b->e);
	if(pthread_join(b->tid,NULL))
	{
		perror("dataReader - thread join error");
	}
	printf("Producer thread closed.\n");
	destroypatchBuffer(b);		
	//fprintf(stderr,"after destroy\n");
}

void setCont(patchBuffer* b, int val)
{
	//fprintf(stderr,"setting cont to %i.\n",val);fflush(stderr);
	beginCritical(&(b->contGuard));
	b->cont=val;
	endCritical(&(b->contGuard));
}

int readCont(patchBuffer* b)
{
	int temp;
	beginCritical(&(b->contGuard));
	temp=b->cont;
	endCritical(&(b->contGuard));
	//fprintf(stderr,"returning %i from readCont()\n",temp);
	return temp;
}


void initPatchBuffer(patchBuffer* b, int size, int fill, int sizeX, int sizeY, int sizeZ, char* patchDir)
{
	printf("Initializing producer/consumer patchBuffer.\n");
	b->head=0;
	b->tail=0;
	b->fill=fill;
	b->size=size;
	
	b->cont=1;
	if(pthread_mutex_init(&(b->contGuard),NULL))
	{
		perror("error creating mutex");
		exit(EXIT_FAILURE);
	}
	b->sizeX=sizeX;
	b->sizeY=sizeY;
	b->sizeZ=sizeZ;
	b->patchSize=sizeX*sizeY*sizeZ;
	
	b->patches=(int*)malloc(sizeof(int)*b->patchSize*size);
	if(b->patches==NULL)
	{
		perror("Memory for patchBuffer could not be allocated");
		exit(EXIT_FAILURE);
	}
	b->tempPatch = (int*)malloc(sizeof(int)*b->patchSize);
	
	b->patchFile=fopen(patchDir,"rb");
	

	initSem(&b->e,size);
	initSem(&b->s,1);
	initSem(&b->n,0-fill);//0);
	b->patchesLeft=0;
	
	//initSem(&b->f,1-fill);
}

void destroypatchBuffer(patchBuffer* b)
{
	//printf("destroying patchBuffer.\n");
	free(b->patches);
	destroySem(&b->e);
	destroySem(&b->s);
	destroySem(&b->n);
	
	if(pthread_mutex_destroy(&(b->contGuard)))
	{
		perror("error destroying mutex");
		exit(EXIT_FAILURE);
	}
	//destroySem(&b->f);
	if(fclose(b->patchFile))
	{
		perror("failed to close dump filestream");
	}
	//fprintf(stderr,"destroy compete\n");
}
