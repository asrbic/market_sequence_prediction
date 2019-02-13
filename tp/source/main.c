#include "graphics.h"
#include "tp.h"
#include "string.h"
#include "stdio.h"

#ifdef _WIN32
	#include <windows.h>
#elif MACOS
	#include <sys/param.h>
	#include <sys/sysctl.h>
#else
	#include <unistd.h>
#endif


int NUM_CORES=1;

int getNumCPUCores();

//*************MAIN FUNCTION****************************
int main()
{
	int temporalSize=100;
	int sideSize=16;
	int colPixelSize=20;
	int numCols=sideSize*sideSize;
	gfxData data;
	int input[numCols];
	memset(input,0,numCols*sizeof(int));
	//initSubArrays(&data,input,sideSize,sideSize,2,2,25,"./imageDumps",1);
	NUM_CORES=getNumCPUCores();
	printf("CPU cores: %i\n",NUM_CORES);
	temporalPooler tp;
	initTP(&tp,sideSize,sideSize,input,1);
	//fprintf(stdout,"codes file will not be read - modoify main!\n");
	//initTPgfxData(&data, input, &tp, colPixelSize, 2,"/home/meecham/Desktop/thesis_data/img/","/media/Data/datasets/KTH_Source/training_data/colCodes_wang_s0_100x20K_randTranches.bin");
	initTPgfxData(&data, input, &tp, colPixelSize, 2,NULL,NULL);
	int eventResult;
	render(&data);
	
	srand(7);
	
	while(data.running)
	{
		
		eventResult=handleEvents(&data);
		if(eventResult==ACTIVITY_CHANGE)
		{
			calcActiveState(&tp,0);
			calcPredictiveState(&tp,0);
			learn(&tp,0);
			render(&data);
		}
		else if(eventResult==CLEAR_TP)
		{
			clearTP(&tp,0);
			render(&data);
		}
		else if(eventResult==NEXT_PATCH)
		{
			readNextPatch(data.codesFile, data.input, sideSize, sideSize, numCols);
			calcActiveState(&tp,0);
			calcPredictiveState(&tp,0);
			learn(&tp,0);
			render(&data);
		}
		else if(eventResult==FULL_RUN)
		{
			int iteration=0;
			while(!feof(data.codesFile) && data.running)
			{
				eventResult=handleEvents(&data);
				if(eventResult==FULL_RUN)
				{
					printf("iteration:%i\n",iteration);
					break;
				}
				if(iteration%temporalSize==0)
				{
					//rewind(data.codesFile);
					clearTP(&tp,0);
					if(iteration%100==0)
						printf("%i\n",iteration);
				}
				readNextPatch(data.codesFile, data.input, sideSize, sideSize, numCols);
				calcActiveState(&tp,0);
				calcPredictiveState(&tp,0);
				learn(&tp,0);
				render(&data);
				++iteration;
			}
			
		}
		else
		{
			//sleep(1);
		}
	}
	destroyTP(&tp);
	destroyData(&data);
	return 0;
}



int getNumCPUCores()
{
	//**********WINDOWS*************
	#if defined WIN32	
		SYSTEM_INFO sysinfo;
		GetSystemInfo(&sysinfo);
		return sysinfo.dwNumberOfProcessors;
	
	//***********OSX*****************
	#elif MACOS
		int nm[2];
		size_t len = 4;
		uint32_t count;

		nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
		sysctl(nm, 2, &count, &len, NULL, 0);

		if(count < 1)
		{
			nm[1] = HW_NCPU;
			sysctl(nm, 2, &count, &len, NULL, 0);
			if(count < 1) 
				count = 1;
		}
		return count;
		
	//***************LINUX*************
	#else	
		return sysconf(_SC_NPROCESSORS_ONLN);	
	#endif
	return 1;
}

