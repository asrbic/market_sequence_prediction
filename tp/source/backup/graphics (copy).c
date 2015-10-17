#include "graphics.h"

#include "tp.h"


#include "stdlib.h"
#include "unistd.h"
#include "time.h"
#include "sys/time.h"
#include "sys/types.h"
#include "math.h"

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

void clearActiveStates(int* input, int size);

/*int main()
{
	gfxData data;
	run(&data);
}*/

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

void destroyData(gfxData* data)
{
	//free data
	free(data->oBuffer);
	if (SDL_WasInit(SDL_INIT_VIDEO))
		SDL_Quit();
}

void render(gfxData* data)
{
	//printf("%li",CLOCKS_PER_SEC);
	SDL_LockSurface(data->buffer);
	SDL_FillRect(data->buffer, NULL, -1);
	
	drawSeperatedArray(data,data->input,0,0,0);
	
	SDL_UnlockSurface(data->buffer);
	// Flip the buffers
	SDL_Flip(data->buffer);
}

void drawSeperatedArray(gfxData* data, int* arr, int x, int y, int oStart)
{
	Point p;
	for(int i=0;i<data->yDim;++i)
		for(int j=0;j<data->xDim;++j)
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
	for(int i=origin.y;i<yMax;++i)
	{
		for(int j=origin.x;j<xMax;++j)
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
						//				--data->instanceInc;
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
						char dir[100];
						sprintf(dir,"%s%i.bmp",data->imgDir,1);//data->instanceIndex);
						SDL_SaveBMP(data->buffer,dir);
					}
					break;
					case SDLK_s:
					{
						/*				if(data->showSynapses==-2)
						{
						data->showSynapses=-1;
						printf("Showing all synapses.\n");
						}
						else
						{
						data->showSynapses=-2;
						printf("Showing no synapses.\n");
						}
					*/}
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
					*/}
					break;
					case SDLK_d:
					{/*
						if(data->dualPatch)
						data->dualPatch=0;
						else
						data->dualPatch=1;
					*/}
					break;
					case SDLK_c:
					{
						memset(data->input,0,sizeof(int)*data->size);
						data->mouseStatus=0;
						data->prevMouseStatus=0;
						data->prevOVal=-1;
						
						return CLEAR_TP;
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

void clearActiveStates(int* input, int size)
{
	for(int i=0;i<size;++i)
	{
		input[i]&=!(ACTIVE_STATE);
	}
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
