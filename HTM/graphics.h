/*#include "SDL/SDL.h"

#pragma once
#include "tp.h"

#define NO_ACTIVITY_CHANGE 0
#define ACTIVITY_CHANGE 1
#define CLEAR_TP 2
#define FULL_RUN 5
#define NEXT_PATCH 3


typedef struct gfxData
{
	SDL_Surface* buffer;
	int width,height;
	int running;
	int pixelScale;
	int xDim,yDim;
	int size;
	
	int subXDim,subYDim;
	int subSize;
	int subWidth, subHeight;
	
	int borders;
	
	int* input;
	int* oBuffer;
	char* imgDir;
	int imgIndex;
	
	int mouseStatus;
	int prevOVal;
	int prevMouseStatus;
	char clearStatesOnClick;
	
	char renderFlag;
	
	temporalPooler* tp;
	
	FILE* codesFile;
	int inputInstanceIndex;
	
}gfxData;

void initTPgfxData(gfxData* data, int* userInput, temporalPooler* tp, int pixelScale, int borders, char* imgDir, char* codesFileDir);

void init(gfxData* data, int* input, int xDim, int yDim, int pixelScale, char* imgDir, int borders);

void initSubArrays(gfxData* data, int* input, int xDim, int yDim, int subXDim, int subYDim, int pixelScale, char* imgDir, int borders);

char handleEvents(gfxData* data);
void destroyData(gfxData* data);
void render(gfxData* data);

void readNextPatch(FILE* codesFile, int* input, int xDim, int yDim, int size);
*/
