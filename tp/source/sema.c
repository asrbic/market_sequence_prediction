#include"sema.h"

/*int main(int argc, char** argv)
{
	printf("run successful");
	return(EXIT_SUCCESS);
}*/

void initSem(Semaphore* sem, int value)
{	//printf("Initializing Semaphore with initial value of: %i\n",value);
	if(pthread_cond_init(&(sem->cond),NULL))
	{
		perror("error creating condition");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_init(&(sem->m),NULL))
	{
		perror("error creating mutex");
		exit(EXIT_FAILURE);
	}
	sem->val=value;
}

void destroySem(Semaphore* sem)
{
	if(pthread_cond_destroy(&(sem->cond)))
	{
		perror("error destroying condition");
		exit(EXIT_FAILURE);
	}
	if(pthread_mutex_destroy(&(sem->m)))
	{
		perror("error destroying mutex");
		exit(EXIT_FAILURE);
	}
}

void procure(Semaphore* sem)
{
	beginCritical(&(sem->m));  // make the following concurrency-safe
	//printf("sem->val=%i\n",sem->val);
	while (sem->val <= 0)
	{
		if(pthread_cond_wait(&(sem->cond),&(sem->m)))
		{
			perror("Error waiting for cond");
			exit(EXIT_FAILURE);
		}
		//printf("cond acquired");
	}
	//	wait_for_vacate(sem);     // wait for signal from vacate()
	sem->val--;                 // claim the Semaphore
	endCritical(&(sem->m));
}

void vacate(Semaphore* sem)
{
	beginCritical(&(sem->m));  // make the following concurrency-safe
	sem->val++;                 // release the Semaphore
	if(pthread_cond_signal(&(sem->cond)))         // signal anyone waiting on this
	{
		perror("Error signalling cond");
		exit(EXIT_FAILURE);
	}
	endCritical(&(sem->m));
}

void beginCritical(pthread_mutex_t* m)
{
	if(pthread_mutex_lock(m))
	{
		perror("error locking mutex");
		exit(EXIT_FAILURE);
	}
}

void endCritical(pthread_mutex_t* m)
{
	if(pthread_mutex_unlock(m))
	{
		perror("error unlocking mutex");
		exit(EXIT_FAILURE);
	}
}

