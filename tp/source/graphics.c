#include "graphics.h"


#include "stdlib.h"
#include "unistd.h"
#include "time.h"
#include "sys/time.h"
#include "sys/types.h"
#include "math.h"
#include "string.h"

#include "SDL/SDL_ttf.h"
#include "SDL/SDL_image.h"
#include "SDL/SDL.h"

#if defined(__MACH__) && defined(__APPLE__)
    // Allow SDL main hack, because of the OS X Cocoa binding
#else
    // SDL main hackery disabled for Windows and Linux
    #define SDL_main main           
#endif

#define ACTIVE_SYNAPSE_COLOR 255
#define POTENTIAL_SYNAPSE_COLOR 0

typedef struct Point
{
	int x;
	int y;
	int z;
	Uint8 r;
	Uint8 g;
	Uint8 b;
	int o;
}Point;



void run(gfxData* data);
void putpixel(gfxData* data, int x, int y, Uint8 r, Uint8 g, Uint8 b, int o);
void makeLine(gfxData* data, Point p0, Point p1);
void scanline(gfxData* data, Point p0, Point p1);
void drawArray(gfxData* data, int* arr, int x, int y, int oStart);
void drawSquare(gfxData* data, Point origin, int width, int height);
void drawSeperatedArray(gfxData* data, int* arr, int x, int y, int oStart);

void drawSeperatedSubArrays(gfxData* data, int* arr, int x, int y, int oStart);

void drawTPSubArrays(gfxData* data);

void clearActiveStates(int* input, int size);

void initTPgfxData(gfxData* data, int* userInput, temporalPooler* tp, int pixelScale, int borders, char* imgDir, char* codesFileDir)
{
	data->tp=tp;
	data->input=userInput;
	data->running=1;
	data->xDim=tp->xDim;
	data->yDim=tp->yDim;
	data->size=tp->xDim*tp->yDim;
	data->pixelScale=pixelScale;
	data->borders=borders;
	
	data->subXDim=sqrt(NUM_CELLS);
	data->subYDim=data->subXDim;
	data->subSize=NUM_CELLS;//for column data
	//printf("subXDim: %i\n", data->subXDim);
	data->pixelScale=pixelScale;
	
	data->subWidth=data->subXDim*pixelScale;
	data->subHeight=data->subYDim*pixelScale;
	
	int borders2=borders*2;
	data->width=data->xDim*(data->subWidth+borders2);
	data->height=data->yDim*(data->subHeight+borders2);
	
	data->oBuffer=(int*)malloc(sizeof(int)*data->width*data->height);
	for(int i=0;i<data->width*data->height;++i)
		*((data->oBuffer)+i)=-1;
	data->imgDir=imgDir;
	data->imgIndex=0;
	data->mouseStatus=0;
	data->prevOVal=-1;
	
	data->clearStatesOnClick=1;
	
	data->renderFlag=1;
	
	if(codesFileDir!=NULL)
	{
		if((data->codesFile=fopen(codesFileDir,"rb")) == NULL)
		{
			fprintf(stderr, "Error, could not open file %s for reading\n", codesFileDir);
			exit(1);
		}
	}
	else
		data->codesFile=NULL;
	data->inputInstanceIndex=0;
	
	// Initialise SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr,"%s",SDL_GetError());
		SDL_Quit();
	}

	// Create a framebuffer
	Uint32 flags = SDL_SWSURFACE | SDL_DOUBLEBUF;

	data->buffer = SDL_SetVideoMode(data->width, data->height, 32, flags);
	if (!data->buffer)
	{
		fprintf(stderr,"%s",SDL_GetError());
		SDL_Quit();
	}
	//FOR PAUSE ON CLICK
}

void init(gfxData* data, int* input, int xDim, int yDim, int pixelScale, char* imgDir, int borders)
{
	data->input=input;
	data->running=1;
	data->xDim=xDim;
	data->yDim=yDim;
	data->size=xDim*yDim;
	data->pixelScale=pixelScale;
	data->borders=borders;
	
	data->width=xDim*(pixelScale+borders)+borders;
	data->height=yDim*(pixelScale+borders)+borders;
	
	data->oBuffer=(int*)malloc(sizeof(int)*data->width*data->height);
	for(int i=0;i<data->width*data->height;++i)
		*((data->oBuffer)+i)=-1;
	data->imgDir=imgDir;
	data->mouseStatus=0;
	data->prevOVal=-1;
	
	// Initialise SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr,"%s",SDL_GetError());
		SDL_Quit();
	}

	// Create a framebuffer
	Uint32 flags = SDL_SWSURFACE | SDL_DOUBLEBUF;

	data->buffer = SDL_SetVideoMode(data->width, data->height, 32, flags);
	if (!data->buffer)
	{
		fprintf(stderr,"%s",SDL_GetError());
		SDL_Quit();
	}
	//FOR PAUSE ON CLICK
}

void initSubArrays(gfxData* data, int* input, int xDim, int yDim, int subXDim, int subYDim, int pixelScale, char* imgDir, int borders)
{
	data->input=input;
	data->running=1;
	data->xDim=xDim;
	data->yDim=yDim;
	data->size=xDim*yDim;
	
	data->subXDim=subXDim;
	data->subYDim=subYDim;
	data->subSize=(subXDim*subYDim)+1;//for column data
	
	data->pixelScale=pixelScale;
	data->borders=borders;
	
	data->width=xDim*(pixelScale+borders)+borders;
	data->height=yDim*(pixelScale+borders)+borders;
	
	data->oBuffer=(int*)malloc(sizeof(int)*data->width*data->height);
	for(int i=0;i<data->width*data->height;++i)
		*((data->oBuffer)+i)=-1;
	data->imgDir=imgDir;
	data->mouseStatus=0;
	data->prevOVal=-1;
	
	// Initialise SDL
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		fprintf(stderr,"%s",SDL_GetError());
		SDL_Quit();
	}

	// Create a framebuffer
	Uint32 flags = SDL_SWSURFACE | SDL_DOUBLEBUF;

	data->buffer = SDL_SetVideoMode(data->width, data->height, 32, flags);
	if (!data->buffer)
	{
		fprintf(stderr,"%s",SDL_GetError());
		SDL_Quit();
	}
	//FOR PAUSE ON CLICK
}

void destroyData(gfxData* data)
{
	//free data
	free(data->oBuffer);
	if (SDL_WasInit(SDL_INIT_VIDEO))
		SDL_Quit();
	fclose(data->codesFile);
}

void render(gfxData* data)
{
	if(data->renderFlag)
	{
		//printf("%li",CLOCKS_PER_SEC);
		SDL_LockSurface(data->buffer);
		//SDL_FillRect(data->buffer, NULL, 0);
	
		drawTPSubArrays(data);
		//drawSeperatedArray(data,data->input,0,0,0);
		//drawSeperatedSubArrays(data,data->input,0,0,0);
		SDL_UnlockSurface(data->buffer);
		// Flip the buffers
		SDL_Flip(data->buffer);
	}
}


void drawTPSubArrays(gfxData* data)
{
	int borders=data->borders;
	int borders2=borders*2;
	
	int i,j,k,l;
	int colIndex, cellIndex;
	int tempState;
	Point p0,p1;
	Point lp;
	lp.r=255;
	lp.g=255;
	lp.b=255;
	
	lp.o=-1;
	lp.x=lp.y=0;
	drawSquare(data, lp, data->width, data->height);
	
	
	lp.r=0;
	lp.b=0;
	for(j=0, colIndex=0;j<data->yDim;++j)
	{
		
		p0.y=j*(data->subHeight+borders2)+borders;
		for(i=0;i<data->xDim;++i, ++colIndex)
		{
			//printf("colIndex: %i\n",colIndex);	
			p0.x=i*(data->subWidth+borders2)+borders;
			p1.o=colIndex;
			
			if(data->input[colIndex]&=ACTIVE_STATE)
			{//outline active column
				lp.x=p0.x-data->borders;
				lp.y=p0.y-data->borders;
				drawSquare(data, lp, data->subWidth+borders2, data->subHeight+borders2);
			}
			
			for(l=0, cellIndex=0;l<data->subYDim;++l)
			{
				p1.y=p0.y+(l*data->pixelScale);
				for(k=0;k<data->subXDim;++k, ++cellIndex)
				{
					//printf("\tcellIndex: %i\n",cellIndex);
					p1.x=p0.x+(k*data->pixelScale);
					p1.r=p1.g=p1.b=0;
					tempState=data->tp->cols[colIndex].cells[cellIndex].state[data->tp->tIndex];
					//printf("state of col,cell %i,%i : %s\n",colIndex,cellIndex, byte_to_binary(tempState));
					if(tempState&ACTIVE_STATE)
					{
						p1.g=255;
					}
					if(tempState&PREDICTIVE_STATE)
					{
						p1.r=127;
					}
					if(tempState&SEQUENCE_SEGMENT)
					{
						p1.r=255;
					}
					if(tempState&LEARN_STATE)
					{
						p1.b=255;
					}
					drawSquare(data, p1, data->pixelScale, data->pixelScale);
					
				}
			}
		}
	}
	
	//printf("\n");
}

void drawSeperatedSubArrays(gfxData* data, int* arr, int x, int y, int oStart)
{
	fprintf(stderr,"start render\n");
	int i,j,k,l;
	int colWidth=data->pixelScale*data->subXDim;
	int colHeight=data->pixelScale*data->subYDim;
	Point p0,p1;
	if(data->borders)
	{
		p0.r=255;
		p0.g=255;
		p0.b=255;
		
		p0.o=p1.o=-1;
		
		p0.y=0;
		p1.y=data->yDim*(colHeight+1);
		for(i=0;i<=data->xDim;i++)
		{
			p0.x=i*(colWidth+1);
			p1.x=i*(colWidth+1);
			makeLine(data,p0,p1);
		}
		p0.x=0;
		p1.x=data->xDim*(colWidth+1);
		for(j=0;j<=data->yDim;++j)
		{
			p0.y=j*(colHeight+1);
			p1.y=j*(colHeight+1);
			makeLine(data,p0,p1);
		}
		
	}
	fprintf(stderr,"post borders\n");
	p0.r=0;
	p0.g=0;
	p0.b=255;
	
	Point colPoint,cellPoint;
	int colIndex;
	for(i=0;i<data->yDim;++i)
	{
		colPoint.y=i*(colHeight+data->borders)+data->borders;
		for(j=0;j<data->xDim;++j)
		{
			cellPoint.o=(i*data->yDim+j)*data->subSize;
			colPoint.x=j*(colWidth+data->borders)+data->borders;
			colIndex=(i*data->yDim+j)*data->subSize;
			if(arr[colIndex]&ACTIVE_STATE)//column active then draw outline
			{
				p0.o=p1.o=-1;
		
				p0.y=colPoint.y-data->borders;
				p0.x=colPoint.x-data->borders;
				for(int m=0;m<2;++m)
				{
					p1.y=colPoint.y+colHeight;
					p1.x=p0.x;
					makeLine(data,p0,p1);
					p1.y=p0.y;
					p1.x=colPoint.x+colWidth;
					makeLine(data,p0,p1);
					
					p0.y=colPoint.y+colHeight;
				}
				
			}
			for(k=0;k<data->subYDim;++k)
			{
				cellPoint.y=colPoint.y+k*data->pixelScale;
				for(l=0;l<data->subXDim;++l)
				{
					colIndex+=1;
					cellPoint.x=colPoint.x+l*data->pixelScale;
			
					cellPoint.r=cellPoint.g=cellPoint.b=0;
					if(arr[colIndex]&ACTIVE_STATE)
					{
						cellPoint.b=255;
					}
					if(arr[colIndex]&PREDICTIVE_STATE)
					{
						cellPoint.r=255;
					}
					if(arr[colIndex]&SEQUENCE_SEGMENT)
					{
						cellPoint.g=255;
					}
					
					drawSquare(data,cellPoint,data->pixelScale,data->pixelScale);
				}
			}
		}
	}
		fprintf(stderr,"end render\n");

}


void drawSeperatedArray(gfxData* data, int* arr, int x, int y, int oStart)
{
	if(data->borders)
	{
		Point p0,p1;
		p0.r=0;
		p0.g=0;
		p0.b=255;
		
		p0.o=p1.o=-1;
		
		p0.y=0;
		p1.y=data->yDim*(data->pixelScale+1);
		for(int i=0;i<=data->xDim;i++)
		{
			p0.x=i*(data->pixelScale+1);
			p1.x=i*(data->pixelScale+1);
			makeLine(data,p0,p1);
		}
		p0.x=0;
		p1.x=data->xDim*(data->pixelScale+1);
		for(int j=0;j<=data->yDim;++j)
		{
			p0.y=j*(data->pixelScale+1);
			p1.y=j*(data->pixelScale+1);
			makeLine(data,p0,p1);
		}
		
	}

	Point p;
	for(int i=0;i<data->yDim;++i)
		for(int j=0;j<data->xDim;++j)
		{
			for(int k=0;k<NUM_CELLS;++k)
			{
				p.x=j*(data->pixelScale+data->borders)+data->borders;
				p.y=i*(data->pixelScale+data->borders)+data->borders;
			
				p.r=p.g=p.b=0;
				if(arr[i*data->yDim+j]&ACTIVE_STATE)
				{
					p.b=255;
				}
				if(arr[i*data->yDim+j]&PREDICTIVE_STATE)
				{
					p.r=255;
				}
				if(arr[i*data->yDim+j]&SEQUENCE_SEGMENT)
				{
					p.g=255;
				}
				p.o=i*data->yDim+j;
				drawSquare(data,p,data->pixelScale,data->pixelScale);
			}
		}	
		

}

void drawArray(gfxData* data, int* arr, int x, int y, int oStart)
{
	for(int i=0;i<data->xDim;++i)
		for(int j=0;j<data->yDim;++j)
		{
			Point p;
			p.x=x+i*data->pixelScale;
			p.y=y+j*data->pixelScale;
			p.r=p.g=p.b=arr[i*data->yDim+j];
			p.o=oStart+ i*data->yDim+j;
			drawSquare(data,p,data->pixelScale,data->pixelScale);
		}
}


void drawSquare(gfxData* data, Point origin, int width, int height)
{
	int yMax=origin.y+height;
	int xMax=origin.x+width;
	int i,j;
	for(i=origin.y;i<yMax;++i)
	{
		for(j=origin.x;j<xMax;++j)
			putpixel(data, j, i, origin.r, origin.g, origin.b, origin.o);
	}
}


void makeLine(gfxData* data, Point p0, Point p1)
{
	//distance along the x or y axis

	float dx = (p1.x-p0.x);
	float dy = (p1.y-p0.y);
	//float dz = (p1.z-p0.z);

	int steps;
	if(abs(dy)>abs(dx))
		steps=abs(dy);
	else
		steps=abs(dx);

	float xInc = dx/steps;  
	float yInc = dy/steps;
	//float zInc = dz/steps;

	float x = p0.x;	
	float y = p0.y;
	//float z = p0.z;
	
	float r = p0.r;
	float g = p0.g;
	float b = p0.b;
	
	for (int i=0;i<steps;++i)
	{
		putpixel(data,x,y,r,g,b,p0.o);
		x+=xInc;
		y+=yInc;
	}
	
}

void scanline(gfxData* data, Point p0, Point p1)
{
	for(int i=p0.x;i<p1.x;++i)
	{			
		putpixel(data,i,p0.y,p0.r,p0.g,p0.b,p0.o);
	}
}

char handleEvents(gfxData* data)
{

	//sleep(1);
	SDL_Event e;
	int temp=data->prevOVal;
	while (SDL_PollEvent(&e))
	{
		switch (e.type)
		{  
			case SDL_QUIT:  
				data->running = 0;
				return -1;  
			break;  
			case SDL_KEYUP:
				switch (e.key.keysym.sym)
				{
					case SDLK_ESCAPE:
						data->running = 0;
						return -1;
					break;
					case SDLK_RIGHT:
					{
						//				++data->instanceInc;
					}
					break;
					case SDLK_LEFT:
					{
						rewind(data->codesFile);//				--data->instanceInc;
						clearTP(data->tp,0);
					}
					break;
					case SDLK_UP:
					{
						//				data->instanceInc-=10;
					}
					break;
					case SDLK_DOWN:
					{
						//				data->instanceInc+=10;
					}
					break;
					case SDLK_SPACE:
					{
						char dir[200];
						sprintf(dir,"%s%i.bmp",data->imgDir,(data->imgIndex)++);//data->instanceIndex);
						fprintf(stdout,"saveImage return code: %i dir: \"%s\"\n",SDL_SaveBMP(data->buffer,dir),dir);
					}
					break;
					case SDLK_s:
					{
						char* saveDir = "/media/Data/datasets/KTH_Source/tp_state/stateTest.bin";
						saveTP(data->tp,saveDir);
					}
					break;
					case SDLK_a:
					{
						/*	if(data->allSynapses)
						{
						data->allSynapses=0;
						printf("Showing only active synapses.\n");
						}
						else
						{
						data->allSynapses=1;
						printf("Showing active and potential synapses.\n");
						}
					*/
					}
					break;
					case SDLK_l:
					{
						char* saveDir = "/media/Data/datasets/KTH_Source/tp_state/stateTest.bin";
						destroyTP(data->tp);
						readTP(data->tp,saveDir,data->input);

					}
					break;
					case SDLK_t:
					{
						data->tp->learnFlag^=1;
					}
					break;
					case SDLK_c:
					{
						memset(data->input,0,sizeof(int)*data->size);
						data->mouseStatus=0;
						data->prevMouseStatus=0;
						data->prevOVal=-1;
						
						return CLEAR_TP;
					}
					case SDLK_r:
					{
						data->renderFlag^=1;
						
					}
					break;
					case SDLK_f:
					{//SPECIAL FUNCTION
						return FULL_RUN;
						
					}
					break;
					case SDLK_n:
					{
						//readNextPatch(data->codesFile,data->input,data->xDim,data->yDim,data->size);
						return NEXT_PATCH;
						
					}
					break;
					case SDLK_m:
					{
						//readNextPatch(data->codesFile,data->input,data->xDim,data->yDim,data->size);
						data->clearStatesOnClick^=1;
						
					}
					break;
					default: break;
				}
			break;
			
			case SDL_MOUSEBUTTONDOWN:
			{
				data->prevMouseStatus=data->mouseStatus;
				data->mouseStatus=e.button.button;
				//printf("x:%i y:%i\n",e.button.x,e.button.y);
				temp=*(data->oBuffer+(e.button.y*data->width)+e.button.x);
				//printf("down temp: %i\n",temp);
			}
			break;
			
			case SDL_MOUSEBUTTONUP:
			{
				data->mouseStatus=0;
			}
			break;
			case SDL_MOUSEMOTION:
			{
				if(data->mouseStatus)
				{
					//printf("x:%i y:%i\n",e.button.x,e.button.y);
					temp=*(data->oBuffer+(e.button.y*data->width)+e.button.x);
					//printf("%i\n",temp);
					//printf("move temp: %i\n",temp);
				}
			}
			break;
			default: break;  
		}  
	}
	if(data->mouseStatus)
	{
		//=*(data->oBuffer+(e.button.y*data->width)+e.button.x);
		//printf("oVal:%i\n",temp);
		if(temp!=-1 && (data->prevMouseStatus!=data->mouseStatus || data->prevOVal!=temp))
		{
			if(data->mouseStatus==SDL_BUTTON_LEFT && !(data->input[temp]&ACTIVE_STATE)) //&& temp!=data->prevOVal)
			{//LEFT MOUSE BUTTON
				if(data->clearStatesOnClick)
					clearActiveStates(data->input,data->size);
				data->input[temp]|=ACTIVE_STATE;
				//printf("oBuffer value: %i\n",temp);
				//data->prevOVal=temp;
				return ACTIVITY_CHANGE;
			}
			else if(data->mouseStatus==SDL_BUTTON_RIGHT && data->input[temp]&ACTIVE_STATE)
			{//RIGHT MOUSE BUTTON
				data->input[temp]&=!ACTIVE_STATE;
				//printf("oBuffer value: %i\n",temp);
				//data->prevOVal=temp;
				return ACTIVITY_CHANGE;	
			}
		}
		data->prevOVal=temp;
		
	}
	return NO_ACTIVITY_CHANGE;
}

inline void readNextPatch(FILE* codesFile, int* input, int xDim, int yDim, int size)
{
	if(codesFile!=NULL)
	{
		unsigned char activeIndices[256];
		unsigned char numCols, tempColIndex;
		fread(&numCols,1,1,codesFile);
		//printf("%i [",numCols);
		memset(input,0,sizeof(int)*size);
		fread(activeIndices,numCols,1,codesFile);
		for(int i=0;i<numCols;++i)
		{
			//fread(&tempColIndex,1,1,data->codesFile);
			tempColIndex=activeIndices[i];
			//printf("%i(%i,%i) ",tempColIndex,tempColIndex%xDim,tempColIndex/xDim);
			input[tempColIndex]|=ACTIVE_STATE;
		}
		//printf("]\n");
	}
}

void clearActiveStates(int* input, int size)
{
	memset(input,0,size*sizeof(int));
}

void putpixel(gfxData* data, int x, int y, Uint8 r, Uint8 g, Uint8 b, int o)
{
	Uint8 a=255;
	//if(o!=-3)
	*(data->oBuffer+(y*data->width)+x)=o;
	Uint8 *p = (Uint8 *)data->buffer->pixels + y * data->buffer->pitch + x * 4;
    		

#if defined(__MACH__) && defined(__APPLE__)
	*(Uint32 *)p = b << 24 | g << 16 | r << 8 | a; // Big endian
#else
	*(Uint32 *)p = b | g << 8 | r << 16 | a << 24; // Lil endian
#endif	
}
