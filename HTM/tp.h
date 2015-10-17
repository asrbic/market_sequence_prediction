#pragma once

#define NUM_CELLS 4 //should be 30 (2012 presentation)

#define INACTIVE_STATE 0

#define ACTIVE_STATE 1
#define PREDICTIVE_STATE 2
#define LEARN_STATE 4
#define SEQUENCE_SEGMENT 8

#define TIME_STEPS 2



//#define INITIAL_SEGMENT_SIZE 20
#define MAX_SEGMENT_SIZE 40 //from 2012 presentation
#define MAX_NUM_SEGMENTS 128 //from 2012 presentation
#define MAX_SEGMENT_UPDATES_PER_CELL 30//3


struct cell;

typedef struct synapse
{
	struct cell* connectedCell;
	float perm;
}synapse;

typedef struct dendriteSegment
{
	synapse synapses[MAX_SEGMENT_SIZE];
	//synapse* activeSynapses[MAX_SEGMENT_SIZE][TIME_STEPS];
	int numSynapses;
	char state[TIME_STEPS];
	int activeSynapseCount[TIME_STEPS];	//synapses with perm above connectedPerm
	int activeCellSynapseCount[TIME_STEPS];	//as above and also connected to active cell
	char sequenceSegment;
}dendriteSegment;

typedef struct segmentUpdate
{
	//int timeIndex;
	int segIndex;
	char sequenceSegment;
	char newSynapses;
	int numActiveSynapses;
	int activeSynapseIndices[MAX_SEGMENT_SIZE];
	synapse activeSynapses[MAX_SEGMENT_SIZE];
}segmentUpdate;

typedef struct cell
{
	char state[TIME_STEPS];
	int numSegments;
	dendriteSegment dSeg[MAX_NUM_SEGMENTS];
	int numSegmentUpdates;
	segmentUpdate segmentUpdates[MAX_SEGMENT_UPDATES_PER_CELL];
	
	//x and y position of containing column and i index of this cell in that column
	int x,y,i;	
}cell;

typedef struct column
{
	cell cells[NUM_CELLS];
	//int* input;
}column;




typedef struct temporalPooler
{
	int xDim, yDim;
	int numCols;
	int tIndex;
	int ptIndex;
	char learnFlag;
	int numLearningCells[TIME_STEPS];
	cell** learningCells;
	column* cols;

	int* input;	
}temporalPooler;


//************FUNCTION PROTOTYPES**********************
dendriteSegment* getActiveSegment(cell* i, int timeIndex, char state);
char segmentActive(dendriteSegment* s, int timeIndex, char state);
int getBestMatchingCell(column* c, int timeIndex);
int getBestMatchingSegment(cell* tempCell, int timeIndex);
void addSegmentActiveSynapses(temporalPooler* tp, cell* tempCell, int segIndex, int timeIndex, char newSynapses, char sequenceSegment);
void adaptSegments(cell* tempCell, char positiveReinforcement);

void calcActiveState(temporalPooler* tp, int CPUCoreOffset);
void calcPredictiveState(temporalPooler* tp, int CPUCoreOffset);
void learn(temporalPooler* tp, int CPUCoreOffset);
void clearTP(temporalPooler* tp, int CPUCoreOffset);

void prepareIteration(temporalPooler* tp, int CPUCoreOffset);

const char* byte_to_binary(int x);

void saveTP(temporalPooler* tp, char* saveDir);
void readTP(temporalPooler* tp, char* readDir, int* input);

void initTP(temporalPooler* tp, int xDim, int yDim, int* input, char learnFlag);
void destroyTP(temporalPooler* tp);
