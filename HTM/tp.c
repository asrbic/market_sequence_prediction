#include "tp.h"
#include "stdlib.h"
#include "stdio.h"
#include "string.h"
//**********STRUCT DECLARATIONS********************



//************GLOBAL VARIABLES**************************
float connectedPerm=0.2;
float initialPerm=0.21;
int activationThreshold=2;
int minThreshold=1;
int newSynapseCount=3;	//DEFINITELY NEEDS TO BE TWEAKED
float permInc = 0.04;
float permDec = 0.06;
int iteration=0;
int learningRadius=5;



//************INITIALISER FUNCTIONS*********************
void initTP(temporalPooler* tp, int xDim, int yDim, int* input, char learnFlag)
{
	tp->xDim=xDim;
	tp->yDim=yDim;
	tp->numCols=xDim*yDim;
	tp->cols=(column*)calloc(tp->numCols,sizeof(column));
	tp->learningCells=(cell**)calloc(tp->numCols*NUM_CELLS*TIME_STEPS,sizeof(cell*)); //stores cells in learn state for each iteration
	tp->input=input;
	tp->tIndex=0;
	tp->ptIndex=1;
	tp->numLearningCells[tp->tIndex]=0;
	tp->numLearningCells[tp->ptIndex]=0;
	
	tp->learnFlag=learnFlag;
	
	//FOR LEARNING RADUIS
	int colIndex,cellIndex;
	column* tempCol;
	cell* tempCell;
	int x,y;
	for(colIndex=0, tempCol=tp->cols;colIndex<tp->numCols;++colIndex, ++tempCol)
	{
		x=colIndex%xDim;
		y=colIndex/xDim;
		//printf("col %i (%i,%i)\n",colIndex,x,y);
		for(cellIndex=0, tempCell=tempCol->cells;cellIndex<NUM_CELLS;++cellIndex, ++tempCell)
		{
			tempCell->x=x;
			tempCell->y=y;
			tempCell->i=cellIndex;
		}
	}
}

//*************DESTRUCTOR FUNCTIONS**********************
void destroyTP(temporalPooler* tp)
{
	free(tp->cols);
	free(tp->learningCells);
}

//********CONVENIENCE FUNCTIONS****************
void runIteration(temporalPooler* tp)
{
	calcActiveState(tp, 0);
	calcPredictiveState(tp,0);
	learn(tp,0);
}

//*************CORE FUNCTIONS****************************

//calculates the active and learn states cells for each column
void calcActiveState(temporalPooler* tp, int CPUCoreOffset)
{
	//printf("phase 1 start\n");
	//printf("tIndex:%i ptIndex:%i\n",tp->tIndex,tp->ptIndex);
	prepareIteration(tp, CPUCoreOffset);	
	
	char buPredicted;
	char lcChosen;
	column* tempCol;
	cell* tempCell;
	dendriteSegment* tempSeg;
	int colIndex, cellIndex;
	int tIndex=tp->tIndex;
	int ptIndex=tp->ptIndex;
		
	for(colIndex=0, tempCol=(tp->cols);colIndex<tp->numCols;++colIndex, ++tempCol)
	{//for all active columns
		if(tp->input[colIndex]&ACTIVE_STATE)
		{
			//printf("col %i active\n",colIndex);
			buPredicted=0;
			lcChosen=0;
			for(cellIndex=0, tempCell=tempCol->cells;cellIndex<NUM_CELLS;++cellIndex, ++tempCell)
			{//MAY NEED TO CLEAR STATE DATA
				if(tempCell->state[ptIndex]&PREDICTIVE_STATE)
				{//cell was predicting activity in future
				//printf("pred state\n");
					tempSeg = getActiveSegment(tempCell,ptIndex,ACTIVE_STATE);
					if(tempSeg->sequenceSegment)
					{//cell was predicting activity in this iteration
						buPredicted=1;
						(tempCell->state[tIndex])|=ACTIVE_STATE;
						if(tp->learnFlag && segmentActive(tempSeg,ptIndex,LEARN_STATE))
						{//printf("LEARNING CELL %i CHOSEN\n", cellIndex);
							lcChosen=1;
							(tempCell->state[tIndex])|=LEARN_STATE;
							tp->learningCells[(tp->numLearningCells[tIndex]*TIME_STEPS)+tIndex]=tempCell;
							++(tp->numLearningCells[tIndex]);
						}
					}
				}
			}
		
			if(!buPredicted)
				for(cellIndex=0, tempCell=tempCol->cells;cellIndex<NUM_CELLS;++cellIndex, ++tempCell)
					(tempCell->state[tIndex])|=ACTIVE_STATE;
				
			if(tp->learnFlag && !lcChosen)
			{	
				//printf("col %i lc not chosen\n",colIndex);
				cellIndex = getBestMatchingCell(tempCol,ptIndex);
				tempCell=(tempCol->cells)+cellIndex;
				(tempCell->state[tIndex])|=LEARN_STATE;
				(tp->learningCells[(tp->numLearningCells[tIndex]*TIME_STEPS)+tIndex])=tempCell;
				++(tp->numLearningCells[tIndex]);
				//ADD NEW SEQUENCE SEGMENT TO CELL WITH THE HIGHEST ACTIVATION LEVEL - (ESTABLISH NEW SEQUENCE PATH IF IT WAS NOT PREDICTED)
				addSegmentActiveSynapses(tp,tempCell,-1,ptIndex,1,1);
			}
		}
	}
	//printf("phase 1 end\n");
}

void calcPredictiveState(temporalPooler* tp, int CPUCoreOffset)
{
	//printf("phase 2 start\n");
	int colIndex, cellIndex, segIndex, synapseIndex;
	column* tempCol;
	cell* tempCell;
	dendriteSegment* tempSeg;
	synapse* tempSynapse;
	
	int tIndex=tp->tIndex;
	int ptIndex=tp->ptIndex;
	
	int learningCellSynapseCount;
	
	int predSeg;
	
	for(colIndex=0, tempCol=tp->cols;colIndex<tp->numCols;++colIndex, ++tempCol)
	{
		//fprintf(stderr,"colIndex: %i\n",colIndex);
		for(cellIndex=0, tempCell=tempCol->cells;cellIndex<NUM_CELLS;++cellIndex, ++tempCell)
		{
			//fprintf(stderr,"\tcolIndex:%i cellIndex:%i\n",colIndex, cellIndex);
			for(segIndex=0, tempSeg=tempCell->dSeg;segIndex<tempCell->numSegments;++segIndex, ++tempSeg)
			{
				tempSeg->activeSynapseCount[tIndex]=0;
				tempSeg->activeCellSynapseCount[tIndex]=0;
				
				//CLEAR SEGMENT STATE DATA
				tempSeg->state[tIndex]=INACTIVE_STATE;//&=!(LEARN_STATE);

				learningCellSynapseCount=0;
				
				//fprintf(stderr,"\t\tsegIndex: %i sequence:%i\n",segIndex,tempSeg->sequenceSegment);
				for(synapseIndex=0, tempSynapse=(tempSeg->synapses);synapseIndex<tempSeg->numSynapses;++synapseIndex, ++tempSynapse)
				{
					if(tempSynapse->connectedCell->state[tIndex]&ACTIVE_STATE)
					{
						++(tempSeg->activeCellSynapseCount[tIndex]);
						if(tempSynapse->perm>connectedPerm)
						{
							++(tempSeg->activeSynapseCount[tIndex]);
							if(tempSynapse->connectedCell->state[tIndex]&LEARN_STATE)
								++learningCellSynapseCount;	
						}
					}
					//printf("end iteration\n");
				}

				//fprintf(stderr,"\t\tsegIndex: %i sequence:%i synCount:%i activeSynCount:%i\n",segIndex,tempSeg->sequenceSegment,tempSeg->numSynapses,tempSeg->activeSynapseCount[tIndex]);
				if(tempSeg->activeSynapseCount[tIndex]>=activationThreshold)
				{
					tempSeg->state[tIndex]|=ACTIVE_STATE;
					tempCell->state[tIndex]|=PREDICTIVE_STATE;
					
					if(learningCellSynapseCount>=activationThreshold)
						tempSeg->state[tIndex]|=LEARN_STATE;
					
					//tp->input[colIndex]|=PREDICTIVE_STATE;
					if(tempSeg->sequenceSegment)
					{
						tempCell->state[tIndex]|=SEQUENCE_SEGMENT;
					}
					//printf("state of col,cell %i,%i : %s\n",colIndex,cellIndex, byte_to_binary(tempCell->state[tIndex]));
					
					
					if(tp->learnFlag)
					{
						//reinforce predictive segment to be more predictive
						//REINFORCE SEGMENT IN ACTIVE_STATE WHICH CAUSED CELL TO BE IN A PREDICTIVE_STATE
						addSegmentActiveSynapses(tp,tempCell,segIndex,tIndex,0,0); //ADD IN FOLLOWING ITERATION (only segUpdate for tIndex)
					
					
						//FOR NEGATIVE REINFORCEMENT?
						predSeg = getBestMatchingSegment(tempCell,ptIndex);
						//TRY TO MAKE A SEGMENT IN THIS CELL BECOME ACTIVE ONE ITERATION EARLER BY REINFORCING ITS CONNECTIONS WITH CELLS IN LEARN STATE IN PREVIOUS ITERATION
					
					
						//if something in this iteration caused it to be predictive, then try to make it be predictive from input in the previous iteration
						//reinforce and add to a segment which could have predicted this input in the previous time interval

						addSegmentActiveSynapses(tp,tempCell,predSeg,ptIndex,1,0);
					}
				}
				//printf("state of col,cell %i,%i : %s\n",colIndex,cellIndex, byte_to_binary(tempCell->state[tIndex]));
			}
			//printf("state of col,cell %i,%i : %s\n",colIndex,cellIndex, byte_to_binary(tempCell->state[tIndex]));
		}
	}
	//printf("phase 2 end\n");	
}

void learn(temporalPooler* tp, int CPUCoreOffset)
{
	if(!(tp->learnFlag))
		return;
	//printf("phase 3 start\n");
	
	int tIndex=tp->tIndex;
	int ptIndex=tp->ptIndex;
	int colIndex, cellIndex;
	column* tempCol;
	cell* tempCell;
	
	for(colIndex=0, tempCol=(tp->cols);colIndex<tp->numCols;++colIndex, ++tempCol)
	{
		for(cellIndex=0, tempCell=(tempCol->cells);cellIndex<NUM_CELLS;++cellIndex, ++tempCell)
		{
			if(tempCell->state[tIndex]&LEARN_STATE)
			{//printf("\tpositively adapting col,cell: %i,%i\n",colIndex,cellIndex);
				adaptSegments(tempCell,1);
			}
			else if((!(tempCell->state[tIndex]&PREDICTIVE_STATE)) && tempCell->state[ptIndex]&PREDICTIVE_STATE)
			{//printf("\tnegatively adapting col,cell: %i,%i\n",colIndex,cellIndex);
				adaptSegments(tempCell,0);
			}
			//else
				//adaptSegments(tp,tempCell,1);
		}
	}
	++iteration;
	//printf("phase 3 end\n\n");
}

void prepareIteration(temporalPooler* tp, int CPUCoreOffset)
{
	tp->ptIndex=tp->tIndex;
	tp->tIndex=(++tp->tIndex)%TIME_STEPS;
	tp->numLearningCells[tp->tIndex]=0;


	int i,j;
	int tIndex=tp->tIndex;
	column* tempCol;
	cell* tempCell;
	
	for(i=0, tempCol=tp->cols;i<tp->numCols;++i, ++tempCol)
	{
		
		for(j=0, tempCell=tempCol->cells;j<NUM_CELLS;++j, ++tempCell)
		{
			tempCell->state[tIndex]=INACTIVE_STATE;
		}
	}
}

void clearTP(temporalPooler* tp, int CPUCoreOffset)
{
	int colIndex, cellIndex, segIndex;
	column* tempCol;
	cell* tempCell;
	dendriteSegment* tempSeg;
	
	for(colIndex=0;colIndex<tp->numCols;++colIndex)
	{
		
		tempCol=(tp->cols)+colIndex;
		for(cellIndex=0;cellIndex<NUM_CELLS;++cellIndex)
		{
			tempCell=(tempCol->cells)+cellIndex;
			tempCell->state[tp->tIndex]=INACTIVE_STATE;
			tempCell->state[tp->ptIndex]=INACTIVE_STATE;
			//NEW
			tempCell->numSegmentUpdates=0;
			for(segIndex=0;segIndex<tempCell->numSegments;++segIndex)
			{
				tempSeg=(tempCell->dSeg)+segIndex;
				tempSeg->state[tp->tIndex]=0;
				tempSeg->state[tp->ptIndex]=0;
				tempSeg->activeCellSynapseCount[tp->tIndex]=0;
				tempSeg->activeCellSynapseCount[tp->ptIndex]=0;
				tempSeg->activeSynapseCount[tp->tIndex]=0;
				tempSeg->activeSynapseCount[tp->ptIndex]=0;
			}
		}
	}
	tp->numLearningCells[tp->tIndex]=0;
	tp->numLearningCells[tp->ptIndex]=0;
}

//*********SUPPORTING FUNCTIONS******************************
void adaptSegments(cell* tempCell, char positiveReinforcement)
{
	int segUpdateIndex, synapseIndex, synapseUpdateIndex;
	segmentUpdate* tempSegUpdate;
	dendriteSegment* tempSeg;
	synapse* tempSynapse;
	//printf("\t%i segment updates queued\n",tempCell->numSegmentUpdates);
	for(segUpdateIndex=0, tempSegUpdate=(tempCell->segmentUpdates);segUpdateIndex<tempCell->numSegmentUpdates;++segUpdateIndex, ++tempSegUpdate)
	{
		if(tempSegUpdate->segIndex<0)
		{//add new segment
			
			if(tempCell->numSegments==MAX_NUM_SEGMENTS)
			{
				printf("Cell (%2i,%2i,%i) is full.\n",tempCell->x,tempCell->y,tempCell->i);
				continue;
			}
			tempSegUpdate->segIndex=(tempCell->numSegments)++;
			tempSeg=(tempCell->dSeg)+(tempSegUpdate->segIndex);
			tempSeg->sequenceSegment=tempSegUpdate->sequenceSegment;
		}
		else
		{//segment should have existing active synapses, reinforce them
			tempSeg=(tempCell->dSeg)+(tempSegUpdate->segIndex);			
			synapseUpdateIndex=0;
			for(synapseIndex=0;synapseIndex<tempSeg->numSynapses;++synapseIndex)
			{
				tempSynapse=(tempSeg->synapses)+synapseIndex;
				//if synapse index == synapseUpdate index
				if(synapseUpdateIndex < tempSegUpdate->numActiveSynapses && synapseIndex==tempSegUpdate->activeSynapseIndices[synapseUpdateIndex])
				{
					if(positiveReinforcement)
						tempSynapse->perm+=permInc;					
					else
						tempSynapse->perm-=permDec;
					++synapseUpdateIndex;
				}
				else
				{
					if(positiveReinforcement)
						tempSynapse->perm-=permDec;
				}
			
			}
			
		}
		if(tempSegUpdate->newSynapses)
		{//add (newSynapseCount - numActiveSynapses) synapses to segment randomly chosen from the set of cells with learnState=1 at time timeIndex
			for(synapseUpdateIndex=0;synapseUpdateIndex < tempSegUpdate->numActiveSynapses;++synapseUpdateIndex)
			{
				//if new synapse and segment not full
				if(tempSegUpdate->activeSynapseIndices[synapseUpdateIndex]<0 && tempSeg->numSynapses<MAX_SEGMENT_SIZE)
				{//add new synapse
					tempSeg->synapses[tempSeg->numSynapses].connectedCell=tempSegUpdate->activeSynapses[synapseUpdateIndex].connectedCell;
					tempSeg->synapses[tempSeg->numSynapses].perm=tempSegUpdate->activeSynapses[synapseUpdateIndex].perm;//could change to initialPerm
					tempSegUpdate->activeSynapseIndices[synapseUpdateIndex]=(tempSeg->numSynapses)++;				
				}
			}			
		}
	}
	tempCell->numSegmentUpdates=0;
}

void addSegmentActiveSynapses(temporalPooler* tp, cell* tempCell, int segIndex, int timeIndex, char newSynapses, char sequenceSegment)
{


	if(tempCell->numSegmentUpdates>=MAX_SEGMENT_UPDATES_PER_CELL)
		return;
	segmentUpdate* tempSegUpdate=(tempCell->segmentUpdates)+tempCell->numSegmentUpdates;
	tempSegUpdate->numActiveSynapses=0;
	tempSegUpdate->segIndex=segIndex;
	tempSegUpdate->sequenceSegment=sequenceSegment;
	tempSegUpdate->newSynapses=1;

	synapse* tempSyn;
	dendriteSegment* tempSeg;
	if(segIndex>=0)
	{
		tempSeg=(tempCell->dSeg)+segIndex;
		for(int synapseIndex=0;synapseIndex<tempSeg->numSynapses;++synapseIndex)
		{
			tempSyn=(tempSeg->synapses)+synapseIndex;
			if(tempSyn->perm>=connectedPerm)
				if(tempSyn->connectedCell->state[timeIndex]&ACTIVE_STATE)
				{
					tempSegUpdate->activeSynapseIndices[tempSegUpdate->numActiveSynapses]=synapseIndex;
					++tempSegUpdate->numActiveSynapses;
				}
		}
	}

	if(newSynapses)
	{//add (newSynapseCount - numActiveSynapses) synapses to segmentUpdate randomly chosen from the set of cells with learnState=1 at time timeIndex
		if(segIndex<0 && tp->numLearningCells[timeIndex]==0)
		{
			//printf("\tno learn state cells, aborting empty segment creation\n");
			return;
		}
		
		int diff=newSynapseCount-tempSegUpdate->numActiveSynapses;
		int sizeDiff=MAX_SEGMENT_SIZE - tempSegUpdate->numActiveSynapses;
		if(sizeDiff < diff)
			diff=sizeDiff;
		if(tp->numLearningCells[timeIndex]>0)
		{
			
			//printf("adding %tempCell synapses to segment %tempCell\n",diff,segIndex);
			for(;diff>0;--diff)
			{//MULTIPLE CONNECTIONS TO THE SAME LEARN STATE CELL CAN BE MADE
				
				tempSegUpdate->activeSynapses[tempSegUpdate->numActiveSynapses].connectedCell = tp->learningCells[(rand()%tp->numLearningCells[timeIndex])*TIME_STEPS+timeIndex];
				tempSegUpdate->activeSynapses[tempSegUpdate->numActiveSynapses].perm=initialPerm;
				tempSegUpdate->activeSynapseIndices[tempSegUpdate->numActiveSynapses]=-1;
				++(tempSegUpdate->numActiveSynapses);
			}
		}
	}
	++(tempCell->numSegmentUpdates);
}

int getBestMatchingCell(column* c, int timeIndex)
{
	int best=-1;
	int bestIndex=-1;
	int bestSegIndex;
	dendriteSegment* tempSeg;
	cell* tempCell;
	int tempIndex;
	for(int i=0;i<NUM_CELLS;++i)
	{
		tempCell=(c->cells)+i;
		tempIndex=getBestMatchingSegment(tempCell,timeIndex);
		
		
		if(tempIndex>-1)
		{
			tempSeg=(tempCell->dSeg)+tempIndex;
			if(tempSeg->activeCellSynapseCount[timeIndex] > best)
			{
				best=tempSeg->activeCellSynapseCount[timeIndex];
				bestIndex=i;
			}
		}
		
	}
	if(best<0)
	{//TODO optimise this nested loop out by calculating it in phase 2
		//printf("\tNo matching segment found\n");
		best=MAX_NUM_SEGMENTS+1;
		//printf("\tseg counts: ");
		for(int i=0;i<NUM_CELLS;++i)
		{
			
			if(c->cells[i].numSegments < best)
			{
				bestIndex=i;
				best=c->cells[i].numSegments;
			}
			//printf(" %i",c->cells[i].numSegments);
		}
		//printf("\n\tcellIndex: %i numSegs: %i\n",bestIndex,best);
	}
	return bestIndex;
}

int getBestMatchingSegment(cell* tempCell, int timeIndex)
{
	int bestIndex=-1;
	dendriteSegment* tempSeg;
	int bestCount=minThreshold-1;
	int segIndex;
	for(segIndex=0, tempSeg=(tempCell->dSeg);segIndex < tempCell->numSegments;++segIndex, ++tempSeg)
	{
		if(tempSeg->activeCellSynapseCount[timeIndex]>bestCount)
		{
			bestIndex=segIndex;
			bestCount=tempSeg->activeCellSynapseCount[timeIndex];
		}
	}

	return bestIndex;
}

inline char segmentActive(dendriteSegment* s, int timeIndex, char state)
{//ASSUMES SEGMENT ACTIVE STATUS FLAGS WILL BE SET WHEN NECESSARY (PROBABLY PHASE 2)
	return(s->state[timeIndex]&state);
}

dendriteSegment* getActiveSegment(cell* i, int timeIndex, char state)
{//ASSUMES activeSynapseCount WILL BE CALCULATED IN PHASE 2
	dendriteSegment* best=i->dSeg;
	dendriteSegment* tempSeg=(dendriteSegment*)((i->dSeg)+1);
	for(int segIndex=1;segIndex < i->numSegments;++segIndex, ++tempSeg)
	{
		if(tempSeg->state[timeIndex]&state)
		{
			if(best->sequenceSegment ^ tempSeg->sequenceSegment)	//10 OR 01
			{//if only one is a sequenceSegment AND if the sequenceSegment is tempSeg them make tempSeg the best
				if(tempSeg->sequenceSegment)
					best=tempSeg;
			}
			else							//11 OR 00
			{//if either both are or are not sequenceSegments then make best the one with the highest activeSynapseCount
				if(tempSeg->activeSynapseCount[timeIndex]>best->activeSynapseCount[timeIndex])
					best=tempSeg;
			}
		}
	}
	return best;
}

//************************************save/load methods ************************************

//Current temporal sequence information will not be saved - only the testable state of the tp. The current sequence is essentially erased along with all queued segment updates. 
void saveTP(temporalPooler* tp, char* saveDir)
{
	FILE* out;
	if((out=fopen(saveDir,"wb")) == NULL)
	{
		fprintf(stderr,"could not open save file for writing: \"%s\"\n",saveDir);
		exit(EXIT_FAILURE);
	}
	fwrite(&(tp->xDim),sizeof(int),1,out);
	fwrite(&(tp->yDim),sizeof(int),1,out);
	fwrite(&(tp->learnFlag),sizeof(char),1,out);
	

	printf("%lu %lu %lu\n",sizeof(tp->cols),sizeof(column),sizeof(cell));

	//writes all column data - connectedCell pointers on synapses need to be updated to correct new memory addresses though. (done in loop below)
	//TODO remove the fwrite tp->cols below and replace it with a complete fwrite of required vars in the big nested loop below. (should make save file size smaller)
	fwrite((tp->cols),sizeof(column),tp->numCols,out);
		
	column* tempCol;
	cell* tempCell;
	dendriteSegment* tempSeg;
	synapse* tempSyn;
	int colIndex,cellIndex,segIndex,synIndex;
	for(colIndex=0, tempCol=tp->cols;colIndex<tp->numCols;++colIndex,++tempCol)
	{

		for(cellIndex=0,tempCell=tempCol->cells;cellIndex<NUM_CELLS;++cellIndex,++tempCell)
		{
			for(segIndex=0,tempSeg=tempCell->dSeg;segIndex<tempCell->numSegments;++segIndex,++tempSeg)
			{
				for(synIndex=0,tempSyn=tempSeg->synapses;synIndex<tempSeg->numSynapses;++synIndex,++tempSyn)
				{
					fwrite(&(tempSyn->connectedCell->x),sizeof(int),1,out);
					fwrite(&(tempSyn->connectedCell->y),sizeof(int),1,out);
					fwrite(&(tempSyn->connectedCell->i),sizeof(int),1,out);					
				}
					
			}
		}
	}
	fclose(out);
	
}

void readTP(temporalPooler* tp, char* readDir, int* input)
{
	FILE* in;
	if((in=fopen(readDir,"rb")) == NULL)
	{
		fprintf(stderr,"could not open save file for reading: \"%s\"\n",readDir);
		exit(EXIT_FAILURE);
	}
	fread(&(tp->xDim),sizeof(int),1,in);
	fread(&(tp->yDim),sizeof(int),1,in);
	fread(&(tp->learnFlag),sizeof(char),1,in);
	initTP(tp,tp->xDim,tp->yDim,input,tp->learnFlag);//initialise struct members in a uniform fashion
	fread(tp->cols,sizeof(column),tp->numCols,in);
	
	column* tempCol;
	cell* tempCell;
	dendriteSegment* tempSeg;
	synapse* tempSyn;
	int colIndex,cellIndex,segIndex,synIndex;
	int x,y,i;
	for(colIndex=0, tempCol=tp->cols;colIndex<tp->numCols;++colIndex,++tempCol)
	{

		for(cellIndex=0,tempCell=tempCol->cells;cellIndex<NUM_CELLS;++cellIndex,++tempCell)
		{
			for(segIndex=0,tempSeg=tempCell->dSeg;segIndex<tempCell->numSegments;++segIndex,++tempSeg)
			{
				for(synIndex=0,tempSyn=tempSeg->synapses;synIndex<tempSeg->numSynapses;++synIndex,++tempSyn)
				{
					fread(&(x),sizeof(int),1,in);
					fread(&(y),sizeof(int),1,in);
					fread(&(i),sizeof(int),1,in);
					tempSyn->connectedCell=&(tp->cols[x+(y*tp->xDim)].cells[i]);
				}
					
			}
		}
	}
	
	fclose(in);
	clearTP(tp,0);
	
}

//*************************PRINT FORMATTING METHODS ************************************
const char *byte_to_binary(int x)
{
    static char b[9];
    b[0] = '\0';

    int z;
    for (z = 256; z > 0; z >>= 1)
    {
        strcat(b, ((x & z) == z) ? "1" : "0");
    }

    return b;
}

