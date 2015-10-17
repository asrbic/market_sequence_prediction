/**************************************************/
/* SELF-ORGANISING SPARSE CODING                  */
/* Based on the Numenta spatial pooling algorithm */
/* Published in the Hierarchical Temporal Memory  */
/* document: HTM Cortical Learning Algorithms     */
/* Version 0.2, December 10, 2010                 */
/*												  */
/* This code written by John Thornton			  */
/* 23 April 2011								  */
/* Griffith University							  */
/* Cognitive Computing Unit						  */
/* j.thornton@griffith.edu.au					  */
/*												  */
/* The important additions to the Numenta ideas   */
/* are the handling of greyscale images which     */
/* includes the non-binary updating of the		  */
/* permanence values of the synapses and the      */
/* working out of how to project and inhibit      */
/* synapses given a square input array			  */
/**************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/times.h>

//#define GRAPHICS 1

#ifdef GRAPHICS
#include "graphics.h"
#endif


/**************************************************/
/* SELF-ORGANISING SPARSE CODING                  */
/* Based on the Numenta spatial pooling algorithm */
/* Published in the Hierarchical Temporal Memory  */
/* document: HTM Cortical Learning Algorithms     */
/* Version 0.2, December 10, 2010                 */
/*												  */
/* This code written by John Thornton			  */
/* 23 April 2011								  */
/* Griffith University							  */
/* Cognitive Computing Unit						  */
/* j.thornton@griffith.edu.au					  */
/*												  */
/* The important additions to the Numenta ideas   */
/* are the handling of greyscale images which     */
/* includes the non-binary updating of the		  */
/* permanence values of the synapses and the      */
/* working out of how to project and inhibit      */
/* synapses given a square input array			  */
/**************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <sys/times.h>

/**************************************************/
/* Data Structure Constants                       */
/**************************************************/
// Numenta settings for char 16-01 to 05, 10 runs per problem, seed 0:

// perm_inc 0.0010, perm_dec 0.0040, min_overlap  2.0, connect  10, living 100, range   2
// perm_inc 0.0010, perm_dec 0.0040, min_overlap  4.0, connect  10, living 100, range   2

// perm_inc 0.0005, perm_dec 0.0025, min_overlap  2.0, connect  10, living 100, range   2
// perm_inc 0.0005, perm_dec 0.0025, min_overlap  5.0, connect  10, living 100, range   2

// perm_inc 0.0010, perm_dec 0.0010, min_overlap  4.0, connect  25, living  50, range   2
// perm_inc 0.0010, perm_dec 0.0010, min_overlap 10.0, connect  25, living  50, range   2

// perm_inc 0.0010, perm_dec 0.0010, min_overlap 10.0, connect 100, living  50, range   2
// perm_inc 0.0010, perm_dec 0.0010, min_overlap 10.0, connect 100, living  50, range 256

// perm_inc 0.0010, perm_dec 0.0010, min_overlap 50.0, connect 100, living  50, range   2
// perm_inc 0.0010, perm_dec 0.0010, min_overlap 50.0, connect 100, living  50, range 256

// Did not converge:
// perm_inc 0.0010, perm_dec 0.0010, min_overlap 25.0, connect  50, living  50, range   2
// perm_inc 0.0010, perm_dec 0.0010, min_overlap 25.0, connect  50, living  50, range 256

// Numenta settings for patches01, 10 runs per problem, seed 0:

// perm_inc 0.0005, perm_dec 0.0025, min_overlap  1.0, connect  10, living 100, range   2
// perm_inc 0.0005, perm_dec 0.0025, min_overlap  5ß.0, connect  10, living 100, range   2

#define SIDE 16

// Numenta settings for char 32-01 to 05, 10 runs per problem, seed 0:

// perm_inc 0.0010, perm_dec 0.0010, min_overlap 50.0, connect  50, living  50, range   2
// perm_inc 0.0010, perm_dec 0.0010, min_overlap 50.0, connect  50, living  50, range 256

// #define SIDE 32

//#define INSTANCES 2000000
#define INTENSITY_RANGE 256
#define PIXELS (SIDE*SIDE)
#define COLUMNS PIXELS
#define SYNAPSES PIXELS
#define FILTER_SCALE 1000
#define P_SCALE 3
#define SCREEN ((PIXELS*P_SCALE)+SIDE)
#define X 0
#define Y 1

/**************************************************/
/* Utility Macros								  */
/**************************************************/
#define X_VAL(X) ((X)/SIDE)
#define Y_VAL(X) ((X)%SIDE)
#define SQ(X) ((X)*(X))
#define PI 3.1416

/**************************************************/
/* Numenta Parameter Defaults					  */
/**************************************************/
// NUMENTA START
#define NUMENTA_PERMANENCE_INC 0.0005 // 0.001  // Try 0.0005 or 0.001
#define NUMENTA_PERMANENCE_DEC 0.0025 // 0.001  // Try 0.0025 or 0.004
#define NUMENTA_INTENSITY_RANGE 2
#define	NUMENTA_MIN_OVERLAP 2.0
// ELSE
#define PERMANENCE_INC 0.02
#define PERMANENCE_DEC 0.02
// NUMENTA END
#define P_CONNECT 10
#define DESIRED_LOCAL_ACTIVITY 0.05 // Was 0.1
#define PERMANENCE 0.2
#define PERMANENCE_SCALE_FACTOR 0.1
#define BOOST_INC 0.005
#define MIN_DUTY_THRESHOLD 0.01
#define ACTIVITY_DECAY 100
#define MAX_BOOST 4.0
#define P_LIVING 100
#define MIN_OVERLAP -1.0

/**************************************************/
/* Numenta Parameters							  */
/**************************************************/
float pDesiredLocalActivity;
float pConnectedPerm;
float pPermanenceInc;
float pPermanenceDec;
float pPermanenceMultiplier;
float pPermanenceScaleFactor;
float pPermanenceMinimum;
float pBoostInc;
float pMinDutyThreshold;
float pMaxBoost;
float pMinOverlap;
char pInFileName[100];
int pConnect;
int pCycleWindow;
int pIntensityRange;
int pActivityDecay;
int pLiving;

/**************************************************/
/* General Parameter Defaults					  */
/**************************************************/
#define SEED 0 
#define TESTS 1
#define CYCLES 500
#define MAX_INTENSITY 1000000
const char *IN_FILE_NAME = "./rTxt7Files";

/**************************************************/
/* General Parameters							  */
/**************************************************/
int pSeed;
int pCycles;
int pTests;
int pImage;

/**************************************************/
/* Control Switches								  */
/**************************************************/
int sRandomise;
int sDrawColumns;
int sPrintStats;
int sReportProgress;
int sOutputCodes;
int sCalcKurtosis;
int sNumenta;
int sEntropy;
int sReconstruct;
int sReadCoords;
int sInvert;
int sDisplayColumns;

/**************************************************/
/* Struct Declarations							  */
/**************************************************/
typedef struct Column	{
	int numConnected; 
	int numPotential;
	int neighbour[COLUMNS];
	int potentialSynapses[SYNAPSES];
	int synapseInverted[SYNAPSES];
	int receptiveFieldSize;
	int* activityCycle;
	int* overlapCycle;
	int activeDutyCycle;
	int overlapDutyCycle;
	int synapsesOn;
	int synapsesOff;
	int activityDecay;
	float boost;
	float overlap;
	float permanence[SYNAPSES];
} column;

typedef struct Dendrite	{
	int length;
	int x;
	int y;
} dendrite;

/**************************************************/
/* Global Stores								  */
/**************************************************/
int** input;
int** overlapStore;
int** activityStore;
int** activeColumns;

float** normalisedOverlap;
float lastPermanence[COLUMNS][SYNAPSES];
int** filter;
int* filterMean;
int* maxFilter;
int* minFilter;
int* upperMean;
int* lowerMean;
long double entropySum[PIXELS];

int* activeCount;
int* minInput;
int* maxInput;
int* meanInput;
int maxTotalInput;
int minTotalInput;
long double meanTotalInput;


/**************************************************/
/* Graphics Variables							  */
/**************************************************/
//int 2DColumns[SCREEN][SCREEN];
int displayColumns[1][SCREEN*SCREEN];
int displayImage[2][PIXELS*PIXELS];
//int displayPatch[INSTANCES][PIXELS]; commented out - not needed
int numImages;

/**************************************************/
/* Global Variables								  */
/**************************************************/
int image[PIXELS];
int** imageCoordinates;
int* inputIndex;
int iIndex;
int instance;
int numInstance;
int totalConnections;
int totalReceptiveFieldSize;
dendrite sortedDendrites[COLUMNS];
column columns[COLUMNS];
int numActive;
int numBoost;
int numInc;
int averageReceptiveFieldSize;
int desiredLocalActivity;
int meanConnections;
int activityCounter;
int overlapCounter;
int test;
int cycle;
int changedCodes;
int stillActive;
int stillLearning;

int numLiving;
int livingColumn[COLUMNS];

float minOverlap;
float meanIntensity; // CHANGE 2 from int to float
float totalLifetimeKurtosis;
float totalPopulationKurtosis;
int synapticImage[SIDE][SIDE];
int reconstructedImage[PIXELS][PIXELS];
int reconstructionCount[PIXELS][PIXELS];
int reconstructedFilterImage[PIXELS][PIXELS];
int filterCount[PIXELS][PIXELS];

/**************************************************/
/* Forward Declarations							  */
/**************************************************/

// Initialisation functions
void initialiseParameters(int, char *[]);
//void initialiseInput(void);
void initialiseDendrites(void);
void initialiseNeighbours(void);
void initialiseColumns(void);
void initialiseSynapses(void);

// Spatial pooling functions
void senseInput(void);
void calculateOverlap(void);
void inhibitColumns(void);
void performLearning(void);
void updateAverageReceptiveFieldSize(void);

// Utility functions
long double calculateLifetimeKurtosis(void);
long double calculatePopulationKurtosis(void);
double elapsedSeconds(void);
void standardiseColumnOverlaps(int **);//prototype changed from array to int **
void calculateKurtosis(void);
void calculateEntropy(void);
void calculateError(void);
void performReconstruction(void);
void reconstructOriginalImage(void);
void reconstructFilterImage(void);
void testForConvergence(void);
void testReceptiveFieldSize(void);
void randomiseIndex(void);
void invertInput(void);
void printStats(void);
void drawColumns(void);
void outputCodes(void);
void drawImage(int);
void drawInstance(int);
void drawFilter(int);
void drawNeighbours(int);
void drawReconstructedInstance(int);
void drawReconstructedFilter(int);
void scanInt(int, char *[], int, int *);
void scanFloat(int, char *[], int, float *);
void scanString(int, char *[], int, char *);
int compDendrites(const dendrite *, const dendrite *);

//begin modified prototypes
int INSTANCES;
void initialiseInput(unsigned char* inputData);
void train();
void allocateGlobals(int numInstances);
void freeGlobals(int numInstances);
//end modified prototypes

//Graphics functions
#ifdef GRAPHICS
void displayArray();
void updateArrayInput(gfxData* data, int arraySize);
void createColumnDisplay(void);
void drawBox(void);
#endif


/**************************************************/
/* Main Function								  */
/**************************************************/
int main(int argc, char *argv[])
{
	initialiseParameters(argc, argv);
	train();
}

//BEGIN EDITS
void train()
{
	int totalCycles = 0, totalTimeOuts = 0;
	
	elapsedSeconds();
	
	//initialiseParameters(argc, argv); dones in main
	
	//************ begin additions ********************
	char* homeSamplePatchDir = "/media/Data/datasets/KTH_Source/training_data/trainDataSample2.bin";
	char* homePatchDir = "/media/Data/datasets/KTH_Source/training_data/data.bin";
	char* homeRandPatchDir = "/media/Data/datasets/KTH_Source/training_data/randData.bin";
	char* ccuSamplePatchDir = "/media/data/datasets/KTH_Source/training_data/trainDataSample.bin";
	char* ccuPatchDir = "/media/data/datasets/KTH_Source/training_data/data.bin";
	char* targetPatchDir = homeRandPatchDir;
	fprintf(stdout,"using training patches from: %s\n",targetPatchDir);
	
	int sizeX = 16;
	int sizeY = 16;
	int sizeZ = 10;
	int patchSize = sizeX*sizeY;
	FILE* trainFile;
	if((trainFile = fopen(targetPatchDir, "rb")) == NULL)
	{
		fprintf(stderr,"could not open file for reading\n");
		exit(EXIT_FAILURE);
	}
	fseek(trainFile,0L,SEEK_END);
	int fileSize = ftell(trainFile);
	int numInstances = (fileSize/patchSize);
	INSTANCES = numInstances;
	allocateGlobals(numInstances);
	fseek(trainFile, 0L, SEEK_SET);
	fprintf(stdout,"fileSize: %i\n",fileSize);
	fprintf(stdout,"numInstances: %i\n",numInstances);
	unsigned char* patches = calloc(fileSize,1);
	
	size_t bytesRead = fread(patches,1,fileSize,trainFile);		
	fclose(trainFile);
	initialiseInput(patches);
	randomiseIndex();
	//**************** end additions ******************
	
	//initialiseInput(); now called with patches array as a paramater
	if(sInvert == 1) invertInput();
	initialiseDendrites();
	initialiseNeighbours();
	
	for(test = 0; test < pTests; ++test)	{	
		
		printf("\nTest %3d of %3d\n", test + 1, pTests);
		initialiseColumns();
		initialiseSynapses();
		//testReceptiveFieldSize();
		for(cycle = 0; cycle < pCycles && stillActive; ++cycle)
		{
			//fprintf(stderr,"before numInstance loop\n");
			for(iIndex = 0; iIndex < numInstance/10; ++iIndex)
			{
				//fprintf(stderr,"0 ");
				senseInput();
				//fprintf(stderr,"1 ");
	
				calculateOverlap();
				//fprintf(stderr,"2 ");
				inhibitColumns();
				//fprintf(stderr,"3 ");
				performLearning();
				//fprintf(stderr,"4 ");
				updateAverageReceptiveFieldSize();
				//fprintf(stderr,"5\n");
				//testReceptiveFieldSize();
			}
			testForConvergence();
			if(sRandomise) randomiseIndex();
		}
		printf("test complete.\n");
		if(sDrawColumns) drawColumns();
		if(pImage >= 0) drawFilter(pImage);
		if(sPrintStats)  printStats();
		if(sOutputCodes) outputCodes();
		if(sCalcKurtosis) calculateKurtosis();
	
		if(!stillActive)	{
			totalCycles += cycle;
			printf("Converged in %d cycles\n", cycle);
			printf("Average cycles %.2f\n", (float)totalCycles/(float)(test + 1));
		}
		else	{
			++totalTimeOuts;
			printf("Failed to converge in %d cycles\n", cycle);
		}
	}
	printf("Time to complete %d successful cycles and %d time out cycles: %.2f seconds\n", 
		totalCycles, totalTimeOuts * pCycles, elapsedSeconds());
	if(sCalcKurtosis) printf("Averaged kurtosis: Lifetime %.2f Population %.2f\n", 
							totalLifetimeKurtosis/(float)pTests, totalPopulationKurtosis/(float)pTests);
	if(sEntropy) calculateEntropy();
	
	//begin modifications
	freeGlobals(numInstances);
	free(patches);
	//end modifications
}


void allocateGlobals(int numInstances)
{

	imageCoordinates = (int**)calloc(sizeof(int*)*numInstances,1);
	for(int i =0; i < numInstances; ++i)
		imageCoordinates[i] = calloc(sizeof(int)*2,1);
	
	inputIndex = (int*)calloc(sizeof(int)*numInstances,1);
	
	activeColumns = (int**)calloc(sizeof(int*)*numInstances,1);
	for(int i = 0; i < numInstances; ++i)
		activeColumns[i]=(int*)calloc(sizeof(int)*COLUMNS,1);
	
	filter=(int**)calloc(sizeof(int*)*numInstances,1);
	for(int i =0; i < numInstances; ++i)
		filter[i]=(int*)calloc(sizeof(int)*PIXELS,1);
		
	activeCount = (int*)calloc(sizeof(int)*numInstances,1);
	minInput = (int*)calloc(sizeof(int)*numInstances,1);
	maxInput = (int*)calloc(sizeof(int)*numInstances,1);
	meanInput = (int*)calloc(sizeof(int)*numInstances,1);

	for(int i = 0; i < COLUMNS;++i)
	{
		columns[i].activityCycle = (int*)calloc(sizeof(int)*numInstances,1);
		columns[i].overlapCycle = (int*)calloc(sizeof(int)*numInstances,1);
	}
	
	
	
	input = (int**)calloc(sizeof(int*)*numInstances,1);
	for(int i =0; i < numInstances; ++i)
		input[i] = calloc(sizeof(int)*PIXELS,1);
	
	overlapStore = (int**)calloc(sizeof(int*)*numInstances,1);
	for(int i =0; i < numInstances; ++i)
		overlapStore[i] = calloc(sizeof(int)*COLUMNS,1);
		
	activityStore = (int**)calloc(sizeof(int*)*numInstances,1);
	for(int i =0; i < numInstances; ++i)
		activityStore[i] = calloc(sizeof(int)*COLUMNS,1);
		
	normalisedOverlap = (float**)calloc(sizeof(float*)*numInstances,1);
	for(int i =0; i < numInstances; ++i)
		normalisedOverlap[i] = calloc(sizeof(float)*COLUMNS,1);
	
	filterMean = (int*)calloc(sizeof(int)*numInstances,1);
	maxFilter = (int*)calloc(sizeof(int)*numInstances,1);
	minFilter = (int*)calloc(sizeof(int)*numInstances,1);
	upperMean = (int*)calloc(sizeof(int)*numInstances,1);
	lowerMean = (int*)calloc(sizeof(int)*numInstances,1);
	
}

void freeGlobals(int numInstances)
{

	for(int i =0; i < numInstances; ++i)
		free(imageCoordinates[i]);
	free(imageCoordinates);
	
	free(inputIndex);
	
	for(int i = 0; i < numInstances; ++i)
		free(activeColumns[i]);
	free(activeColumns);
	
	for(int i =0; i < numInstances; ++i)
		free(filter[i]);
	free(filter);	
	
	free(activeCount);
	free(minInput);
	free(maxInput);
	free(meanInput);
	
	for(int i = 0; i < COLUMNS;++i)
	{
		free(columns[i].activityCycle);
		free(columns[i].overlapCycle);
	}
	
	for(int i = 0; i < numInstances; ++i)
		free(input[i]);
	free(input);

	for(int i = 0; i < numInstances; ++i)
		free(overlapStore[i]);
	free(overlapStore);

	for(int i = 0; i < numInstances; ++i)
		free(activityStore[i]);
	free(activityStore);

	for(int i = 0; i < numInstances; ++i)
		free(normalisedOverlap[i]);
	free(normalisedOverlap);
	
	
	free(filterMean);
	free(maxFilter);
	free(minFilter);
	free(upperMean);
	free(lowerMean);

}

//END EDITS

void initialiseParameters(int argc, char *argv[])
{
	int i, incSet = 0, decSet = 0, rangeSet = 0, olapSet = 0;
	
	pSeed = SEED;
	pCycles = CYCLES;
	pTests = TESTS;
	pImage = -1;
	pDesiredLocalActivity = DESIRED_LOCAL_ACTIVITY;
	pConnectedPerm = PERMANENCE;
	pPermanenceInc = PERMANENCE_INC;
	pPermanenceDec = PERMANENCE_DEC;
	pPermanenceScaleFactor = PERMANENCE_SCALE_FACTOR;
	pPermanenceMinimum = pPermanenceScaleFactor * pConnectedPerm;
	pPermanenceMultiplier = 1.0 + pPermanenceMinimum;
	pBoostInc = BOOST_INC;
	pMinDutyThreshold = MIN_DUTY_THRESHOLD;
	pConnect = P_CONNECT;
	pLiving = P_LIVING;
	pCycleWindow = INSTANCES;
	pIntensityRange = INTENSITY_RANGE;
	pActivityDecay = ACTIVITY_DECAY;
	pMaxBoost = MAX_BOOST;
	pMinOverlap = MIN_OVERLAP;
	strcpy(pInFileName, IN_FILE_NAME);
	sRandomise = 0;
	sReconstruct = sEntropy = sNumenta = 0;
	sDrawColumns = sPrintStats = sReportProgress = sOutputCodes = sCalcKurtosis = 0;
	
	for(i = 1; i < argc; ++i)	{
		if(strcmp(argv[i], "-seed") == 0) scanInt(argc, argv, ++i, &pSeed);
		else if(strcmp(argv[i], "-tests") == 0) scanInt(argc, argv, ++i, &pTests);
		else if(strcmp(argv[i], "-cycles") == 0) scanInt(argc, argv, ++i, &pCycles);
		else if(strcmp(argv[i], "-activity") == 0) scanFloat(argc, argv, ++i, &pDesiredLocalActivity);
		else if(strcmp(argv[i], "-perm") == 0) scanFloat(argc, argv, ++i, &pConnectedPerm);
		else if(strcmp(argv[i], "-inc") == 0) { scanFloat(argc, argv, ++i, &pPermanenceInc); incSet = 1; }
		else if(strcmp(argv[i], "-dec") == 0) { scanFloat(argc, argv, ++i, &pPermanenceDec); decSet = 1; }
		else if(strcmp(argv[i], "-scale") == 0) scanFloat(argc, argv, ++i, &pPermanenceScaleFactor);
		else if(strcmp(argv[i], "-boost") == 0) scanFloat(argc, argv, ++i, &pBoostInc);
		else if(strcmp(argv[i], "-maxboost") == 0) scanFloat(argc, argv, ++i, &pMaxBoost);
		else if(strcmp(argv[i], "-minoverlap") == 0) { scanFloat(argc, argv, ++i, &pMinOverlap); olapSet = 1; }
		else if(strcmp(argv[i], "-duty") == 0) scanFloat(argc, argv, ++i, &pMinDutyThreshold);
		else if(strcmp(argv[i], "-connect") == 0) scanInt(argc, argv, ++i, &pConnect);
		else if(strcmp(argv[i], "-living") == 0) scanInt(argc, argv, ++i, &pLiving);
		else if(strcmp(argv[i], "-window") == 0) scanInt(argc, argv, ++i, &pCycleWindow);
		else if(strcmp(argv[i], "-range") == 0) { scanInt(argc, argv, ++i, &pIntensityRange); rangeSet = 1; }
		else if(strcmp(argv[i], "-decay") == 0) scanInt(argc, argv, ++i, &pActivityDecay);
		else if(strcmp(argv[i], "-image") == 0) scanInt(argc, argv, ++i, &pImage);
		else if(strcmp(argv[i],"-file") == 0) scanString(argc, argv, ++i, pInFileName);
		else if(strcmp(argv[i],"-norand") == 0) sRandomise = 0;
		else if(strcmp(argv[i],"-draw") == 0) sDrawColumns = 1;
		else if(strcmp(argv[i],"-print") == 0) sPrintStats = 1;
		else if(strcmp(argv[i],"-report") == 0) sReportProgress = 1;
		else if(strcmp(argv[i],"-codes") == 0) sOutputCodes = 1;
		else if(strcmp(argv[i],"-kurt") == 0) sCalcKurtosis = 1;
		else if(strcmp(argv[i],"-numenta") == 0) sNumenta = 1;
		else if(strcmp(argv[i],"-entropy") == 0) sEntropy = 1;
		else if(strcmp(argv[i],"-reconstruct") == 0) sReconstruct = 1;
		
		else {
			fprintf(stderr, "\nGeneral Parameters:\n");
			fprintf(stderr, "  -seed N (integer, range >= 0, default: %d)\n", pSeed);
			fprintf(stderr, "	Seed for random number generator\n");
			fprintf(stderr, "  -tests N (integer, range > 0, default: %d)\n", pTests);
			fprintf(stderr, "	Number of convergence tests\n");
			fprintf(stderr, "  -cycles N (integer, range > 0, default: %d)\n", pCycles);
			fprintf(stderr, "	Maximum iterations through data file for each convergence test\n");
			fprintf(stderr, "  -activity N (float, range 0.01 to 1.0, default: %.4f)\n", pDesiredLocalActivity);
			fprintf(stderr, "	Desired proportion of columns active within receptive field\n");
			fprintf(stderr, "  -perm N (float, range 0.0 to 1.0, default: %.4f)\n", pConnectedPerm);
			fprintf(stderr, "	Threshold value at which a synapse becomes connected\n");
			fprintf(stderr, "  -inc N (float, range 0.0 to 1.0, default: %.4f)\n", pPermanenceInc);
			fprintf(stderr, "	Amount permanence is incremented when column becomes active\n");
			fprintf(stderr, "  -dec N (float, range 0.0 to 1.0, default: %.4f)\n", pPermanenceDec);
			fprintf(stderr, "	Amount permanence is decremented when column becomes active\n");
			fprintf(stderr, "  -scale N (float, range > 0.0, default: %.4f)\n", pPermanenceScaleFactor);
			fprintf(stderr, "	Amount permanence is scaled up when column is too inactive\n");
			fprintf(stderr, "  -boost N (float, range > 0.0, default: %.4f)\n", pBoostInc);
			fprintf(stderr, "	Amount overlap is scaled up when column becomes too inactive\n");
			fprintf(stderr, "  -maxboost N (float, range >= 2.0, default: %.4f)\n", pMaxBoost);
			fprintf(stderr, "	Maximum boost value for any column\n");
			fprintf(stderr, "  -minoverlap N (float, range > 0.0, default: %.4f)\n", pMinOverlap);
			fprintf(stderr, "	Minimum overlap value for a column to be counted on overlap duty\n");
			fprintf(stderr, "  -duty N (float, range 0.0 to 1.0, default: %.4f)\n", pMinDutyThreshold);
			fprintf(stderr, "	Minimum activity of a column as proportion of most active neighbour\n");
			fprintf(stderr, "  -connect N (integer, range 1 to 100, default: %d)\n", pConnect);
			fprintf(stderr, "	Probability that a synapse is potentially connected\n");
			fprintf(stderr, "  -living N (integer, range 1 to 100, default: %d)\n", pLiving);
			fprintf(stderr, "	Proportion of columns that are alive out of a total of %d\n", COLUMNS);
			fprintf(stderr, "  -window N (integer, range 1 to %d, default: %d)\n", INSTANCES, pCycleWindow);
			fprintf(stderr, "	Size of sliding window used to calculate average column acitivity\n");
			fprintf(stderr, "  -range N (integer, range >= 2, default: %d)\n", pIntensityRange);
			fprintf(stderr, "	Range within which input data point values are rescaled\n");
			fprintf(stderr, "  -decay N (integer, range >= 0, default: %d)\n", pActivityDecay);
			fprintf(stderr, "	How long a column ceases learning after being active\n");
			fprintf(stderr, "  -file <filename> (default: %s)\n", pInFileName);
			fprintf(stderr, "	Name of input text file containing lines of %d numeric values\n", PIXELS);
			
			fprintf(stderr, "\nControl Switches:\n");
			fprintf(stderr, "  -norand (default: ON)\n");
			fprintf(stderr, "	Stops the randomising of the input order\n");
			fprintf(stderr, "  -draw (default: OFF)\n");
			fprintf(stderr, "	Draws the synapse connections for each column\n");
			fprintf(stderr, "  -print (default: OFF)\n");
			fprintf(stderr, "	Prints overall statistics on program termination\n");
			fprintf(stderr, "  -report (default: OFF)\n");
			fprintf(stderr, "	Reports progress statistics after each program iteration\n");
			fprintf(stderr, "  -codes (default: OFF)\n");
			fprintf(stderr, "	Prints active columns for each input on program termination\n");
			fprintf(stderr, "  -kurt (default: OFF)\n");
			fprintf(stderr, "	Prints averaged kurtosis measures on program termination\n");
			fprintf(stderr, "  -numenta (default: OFF)\n");
			fprintf(stderr, "	Turns on the Numenta algorithm\n");
			fprintf(stderr, "  -entropy (default: OFF)\n");
			fprintf(stderr, "	Calculates the entropy of the image set\n");
			fprintf(stderr, "  -reconstruct (default: OFF)\n");
			fprintf(stderr, "	Reconstructs the image patches to form a unified image\n");
			
			fprintf(stderr, "\nUsage:\n");
			fprintf(stderr, "  spatial -parameters\n\n");
			exit(0);
		}
	}
	
	sReconstruct = 1;
	sReportProgress = 1;
	pConnect = 15;
	sReadCoords = 0;
	sInvert = 0;
	sDisplayColumns = 0;
	//pCycles = 1;
	
	if(sNumenta)	{
		if(!incSet) pPermanenceInc = NUMENTA_PERMANENCE_INC;
		if(!decSet) pPermanenceDec = NUMENTA_PERMANENCE_DEC;
		if(!rangeSet) pIntensityRange = NUMENTA_INTENSITY_RANGE;
		if(!olapSet) pMinOverlap = NUMENTA_MIN_OVERLAP;
	}
	
	printf("Parameter seed: %d\n", pSeed);
	printf("Parameter tests: %d\n", pTests);
	printf("Parameter cycles: %d\n", pCycles);
	printf("Parameter activity: %.4f\n", pDesiredLocalActivity);
	printf("Parameter perm: %.4f\n", pConnectedPerm);
	printf("Parameter inc: %.4f\n", pPermanenceInc);
	printf("Parameter dec: %.4f\n", pPermanenceDec);
	printf("Parameter scale: %.4f\n", pPermanenceScaleFactor);
	printf("Parameter boost: %.4f\n", pBoostInc);
	printf("Parameter maxboost: %.2f\n", pMaxBoost);
	printf("Parameter minoverlap: %.2f\n", pMinOverlap);
	printf("Parameter duty: %.4f\n", pMinDutyThreshold);
	printf("Parameter connect: %d\n", pConnect);
	printf("Parameter living: %d\n", pLiving);
	printf("Parameter window: %d\n", pCycleWindow);
	printf("Parameter range: %d\n", pIntensityRange);
	printf("Parameter decay: %d\n", pActivityDecay);
	printf("Parameter file: %s\n", pInFileName);
	printf("Switch norand: %d\n", sRandomise);
	printf("Switch numenta: %d\n", sNumenta);
	printf("Switch entropy: %d\n", sEntropy);
	printf("Switch reconstruct: %d\n", sReconstruct);
	
	srandom(pSeed);
}


void scanInt(int argc, char *argv[], int i, int *varptr)
{
	if (i >= argc || sscanf(argv[i], "%i", varptr) != 1)	{
		fprintf(stderr, "Bad integer argument %s\n", i<argc ? argv[i] : argv[argc-1]);
		exit(-1);
	}
}


void scanFloat(int argc, char *argv[], int i, float *varptr)
{
	if (i >= argc || sscanf(argv[i], "%f", varptr) != 1)	{
		fprintf(stderr, "Bad floating point argument %s\n", i<argc ? argv[i] : argv[argc-1]);
		exit(-1);
	}
}


void scanString(int argc, char *argv[], int i, char *varptr)
{
	if (i >= argc || sscanf(argv[i], "%s", varptr) != 1)	{
		fprintf(stderr, "Bad string argument %s\n", i<argc ? argv[i] : argv[argc-1]);
		exit(-1);
	}
}

//modified this to read inputs from binary inputData array instead of text file
void initialiseInput(unsigned char* inputData)
{
//	FILE *inFile;
	unsigned char* currentPixelPointer = inputData;
	int i, p, pixel, max, min, endOfInput = 0;
	
	meanTotalInput = maxTotalInput = numInstance = 0;
	minTotalInput = MAX_INTENSITY;
//	if((inFile = fopen(pInFileName, "r")) != NULL)
	//{
		for(i = 0; i < INSTANCES; ++i)
		{
			min = MAX_INTENSITY; max = 0;
			inputIndex[i] = i;
			/*if(sReadCoords)	{
				fscanf(inFile, "%d", &imageCoordinates[numInstance][X]);
				fscanf(inFile, "%d", &imageCoordinates[numInstance][Y]);
			}*/
			for(pixel = 0; pixel < PIXELS; ++pixel)
			{
				p=*currentPixelPointer;
				++currentPixelPointer;
				//fprintf(stdout,"%i ",p);
				if(p > max) max = p;
				if(p < min) min = p;
				input[numInstance][pixel] = p;
				meanInput[numInstance] += p;
			}
			if(endOfInput) break;
			else
			{ //if(max > min)	{
				meanInput[numInstance] /= PIXELS;
				meanTotalInput += meanInput[numInstance];
				maxInput[numInstance] = max;
				minInput[numInstance++] = min;
				if(max > maxTotalInput) maxTotalInput = max;
				if(min < minTotalInput) minTotalInput = min;
			}
		}
		if(pCycleWindow > numInstance) pCycleWindow = numInstance;
		meanTotalInput /= (double) numInstance;
		//fclose(inFile);
	//}
	/*else	{
		fprintf(stderr, "Error, could not open file %s for reading\n", pInFileName);
		exit(1);
	}*/
	printf("Number of instances %d max input %d min input %d\n", numInstance, maxTotalInput, minTotalInput);
}


void invertInput()
{
	int i, p, max, min, inverseMax;
	
	inverseMax = maxTotalInput;
	maxTotalInput = 0; minTotalInput = MAX_INTENSITY;
	for(i = 0; i < numInstance; ++i)	{
		meanInput[i] = 0;
		min = MAX_INTENSITY; max = 0; 
		for(p = 0; p < PIXELS; ++p)	{
			input[i][p] = inverseMax - input[i][p];
			if(input[i][p] > max) max = input[i][p];
			if(input[i][p] < min) min = input[i][p];
			meanInput[i] += input[i][p];
		}
		meanInput[i] /= PIXELS;
		meanTotalInput += meanInput[i];
		maxInput[i] = max;
		minInput[i] = min;
		if(max > maxTotalInput) maxTotalInput = max;
		if(min < minTotalInput) minTotalInput = min;
	}	
}


void initialiseDendrites()
{
	int x = 0, y = 0, sumSq = 0;
	
	for(x = 0; x < SIDE; ++x)	{
		for(y = 0; y <= x; ++y)	{
			sortedDendrites[sumSq].x = x;
			sortedDendrites[sumSq].y = y;
			sortedDendrites[sumSq++].length = (x * x) + (y * y);
		}
	}	
	qsort(sortedDendrites, sumSq, sizeof(dendrite), (void *)compDendrites); 
}


int compDendrites(const dendrite *d1, const dendrite *d2)	{
	if(d1->length <= d2->length) return -1;
	if(d1->length > d2->length) return 1;
	return 0;
}


void initialiseNeighbours()
{
	int c, distance, nextDendrite;
	int x, y, xOffset, yOffset, maxOffset, minOffset;
	
	for(c = 0; c < COLUMNS; ++c)	{
		x = X_VAL(c);
		y = Y_VAL(c);
		columns[c].neighbour[0] = c;
		distance = nextDendrite = 1;
		while(distance < SYNAPSES)	{
			
			xOffset = sortedDendrites[nextDendrite].x;
			yOffset = sortedDendrites[nextDendrite++].y;
			if(xOffset > yOffset) { maxOffset = xOffset; minOffset = yOffset; }
			else { maxOffset = yOffset; minOffset = xOffset; }
			
			if(maxOffset == minOffset)	{
				//Case One: there are four possible neighbours on each of the y diagonals
				if(x - maxOffset >= 0)	{
					if(y - maxOffset >= 0)	
						columns[c].neighbour[distance++] = ((x - maxOffset) * SIDE) + y - maxOffset;
					if(y + maxOffset < SIDE)	
						columns[c].neighbour[distance++] = ((x - maxOffset) * SIDE) + y + maxOffset;
				}
				if(x + maxOffset < SIDE)	{
					if(y - maxOffset >= 0)	
						columns[c].neighbour[distance++] = ((x + maxOffset) * SIDE) + y - maxOffset;
					if(y + maxOffset < SIDE)	
						columns[c].neighbour[distance++] = ((x + maxOffset) * SIDE) + y + maxOffset;
				}
			}
			else if(minOffset == 0)	{
				//Case Two: there are four possible neighbours, two on the x value and two on the y value 
				if(x - maxOffset >= 0)	
					columns[c].neighbour[distance++] = ((x - maxOffset) * SIDE) + y;
				if(x + maxOffset < SIDE)	
					columns[c].neighbour[distance++] = ((x + maxOffset) * SIDE) + y;
				if(y - maxOffset >= 0)	
					columns[c].neighbour[distance++] = (x * SIDE) + y - maxOffset;
				if(y + maxOffset < SIDE)	
					columns[c].neighbour[distance++] = (x * SIDE) + y + maxOffset;
			}
			else	{
				//Case Three: there are eight possible neighbours situated in pairs
				//between each diagonal and each x and each y value
				if(x - maxOffset >= 0)	{
					if(y - minOffset >= 0)	
						columns[c].neighbour[distance++] = ((x - maxOffset) * SIDE) + y - minOffset;
					if(y + minOffset < SIDE)	
						columns[c].neighbour[distance++] = ((x - maxOffset) * SIDE) + y + minOffset;
				}
				if(x + maxOffset < SIDE)	{
					if(y - minOffset >= 0)	
						columns[c].neighbour[distance++] = ((x + maxOffset) * SIDE) + y - minOffset;
					if(y + minOffset < SIDE)	
						columns[c].neighbour[distance++] = ((x + maxOffset) * SIDE) + y + minOffset;
				}
				if(y - maxOffset >= 0)	{
					if(x - minOffset >= 0)	
						columns[c].neighbour[distance++] = ((x - minOffset) * SIDE) + y - maxOffset;
					if(x + minOffset < SIDE)	
						columns[c].neighbour[distance++] = ((x + minOffset) * SIDE) + y - maxOffset;
				}
				if(y + maxOffset < SIDE)	{
					if(x - minOffset >= 0)
						columns[c].neighbour[distance++] = ((x - minOffset) * SIDE) + y + maxOffset;
					if(x + minOffset < SIDE)	
						columns[c].neighbour[distance++] = ((x + minOffset) * SIDE) + y + maxOffset;
				}
			}
		}
		//drawNeighbours(c);
	}
}


void initialiseColumns()
{
	int i, j, c, nextColumn;
	
	for(c = 0; c < COLUMNS; ++c)	{
		if(pLiving >= 100) livingColumn[c] = c;
		else livingColumn[c] = 0;
		columns[c].boost = 1.0;
		columns[c].overlap = 0.0;
		columns[c].activityDecay = columns[c].synapsesOn = columns[c].synapsesOff = 0;
		columns[c].activeDutyCycle = columns[c].overlapDutyCycle = 0;
	}
	
	if(pLiving >= 100) numLiving = COLUMNS;
	else {
		numLiving = (COLUMNS * pLiving)/100;
		for(i = 0; i < numLiving; ++i)	{
			j = random()%COLUMNS;
			while(livingColumn[j] && j < COLUMNS) j++;
			if(j == COLUMNS)	{
				j = 0;
				while(livingColumn[j]) j++;
			}
			livingColumn[j] = 1;
		}
		nextColumn = 0;
		for(i = 0; i < numLiving; ++i)	{
			for(j = nextColumn; j < COLUMNS; ++j)	{
				if(livingColumn[j] == 1) {
					livingColumn[i] = j;
					nextColumn = ++j;
					break;
				}
			}
		}
	}
}


void initialiseSynapses()
{
	int i, c, s, l, distance, numC, numP; 
	int x, y, xy, xMax, xMin, yMax, yMin, xyArea;
	float connectionBias, permanence;
	
	totalReceptiveFieldSize = totalConnections = changedCodes = 0;
	stillLearning = stillActive = 1;
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];
		numP = numC = numBoost = 0;
		
		for(i = 0; i < numInstance; ++i)	{
			columns[c].activityCycle[i] = columns[c].overlapCycle[i] = 0;
			activeColumns[i][c] = 0;
		}
		// The synapse under the column is always part of the column's receptive field
		xMax = xMin = X_VAL(c);
		yMax = yMin = Y_VAL(c);
		
		for(distance = 0; distance < SYNAPSES; ++distance)	{
			
			if(random()%100 < pConnect)	{
				// These are the potential synapses
				s = columns[c].neighbour[distance];
				// The closer the synapse to the column the greater the permanence probability
				connectionBias = random()%1000 - ((distance * 1000)/SYNAPSES);
				permanence = pConnectedPerm + connectionBias/10000.00;
				columns[c].potentialSynapses[numP] = s;
				lastPermanence[c][numP] = 0.0;
				//if(random()%2) columns[c].synapseInverted[numP] = 0;
				//else columns[c].synapseInverted[numP] = 1;
				columns[c].permanence[numP++] = permanence;
				
				if(permanence >= pConnectedPerm)	{
					// These are the connected synapses
					++numC;
					x = X_VAL(s);	y = Y_VAL(s);
					// Find the extreme edges of the receptive field of the column
					if(x > xMax) xMax = x; if(y > yMax) yMax = y;
					if(x < xMin) xMin = x; if(y < yMin) yMin = y;
				}
			}
		}
		// Calculate the area of the circle that fits within the receptive field edges
		xy = xMax - xMin + yMax - yMin + 2;
		if((xyArea = round((PI * xy * xy)/16.0)) > COLUMNS) xyArea = COLUMNS;
		
		columns[c].receptiveFieldSize = xyArea;
		totalReceptiveFieldSize += xyArea;
		columns[c].numConnected = numC;
		columns[c].numPotential = numP;
		totalConnections += numC;
	}
	averageReceptiveFieldSize = totalReceptiveFieldSize/numLiving;
	desiredLocalActivity = round(averageReceptiveFieldSize * pDesiredLocalActivity);
	meanConnections = totalConnections/numLiving;
}


void senseInput()
{
	//printf("begin senseInput: iIndex:%i inputIndex[iIndex]:%i\n",iIndex,inputIndex[iIndex]);
	int p, intensity, intensityRange, minIntensity, maxIntensity, pOn = 0;
	long int totalIntensity = 0;
	float meanOnIntensity;
	
	instance = inputIndex[iIndex];
	//fprintf(stderr,"%i %i %i \n",instance,minInput[instance], maxInput[instance]);
	minIntensity = minInput[instance]; 
	maxIntensity = maxInput[instance];
	intensityRange = maxIntensity - minIntensity + 1;
	//printf("\tbefore loop\n");
	for(p = 0; p < PIXELS; ++p)
	{
		//if(instance == 183)
		//{
		//	fprintf(stderr,"\t\t%i %i \n",instance,p);
		//	printf("\t\t%i asdf\n",input[instance+1][p]);
		//}
		intensity = input[instance][p];//SEG FAULT IS OCCURING ON THIS LINE!!!!!!
		
		
		//if(instance == 183)
		//	fprintf(stderr,"\t\there\n");
		image[p] = ((intensity - minIntensity) * pIntensityRange)/intensityRange;
		//if(instance == 183)
		//	fprintf(stderr,"\t\there\n");
		//image[p] = random()%pIntensityRange;
		image[p] = input[instance][p];
		//if(instance == 183)
		//fprintf(stderr,"\t\there\n");
		totalIntensity += image[p];
		//if(instance == 183)
		//	fprintf(stderr,"\t\there\n");
		if(image[p]) ++pOn;
		//if(instance == 183)
		//	fprintf(stderr,"\t\t%i after \n",instance);
	}
	//printf("\tafter loop\n");
	meanIntensity = (float)totalIntensity / (float)PIXELS; //CHANGE 9
	meanOnIntensity = (float)totalIntensity / (float)pOn;
	if(pMinOverlap < 0.0) minOverlap = meanIntensity * (float) meanConnections;
	else minOverlap = meanOnIntensity * pMinOverlap;
	
	if(sEntropy && test == 0 && cycle == 0)	{
		for(p = 0; p < PIXELS; ++p)	entropySum[p] += (float)image[p]/(float)(pIntensityRange - 1);
	}
	//printf("end senseInput\n");
}


void calculateOverlap()
{
	int c, s, l, numP;
	float overlap;
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];
		overlap = 0.0;
		numP = columns[c].numPotential;
		for(s = 0; s < numP; ++s)	{
			if(columns[c].permanence[s] >= pConnectedPerm)	{
				// CHANGE HERE
				if(columns[c].synapseInverted[s])
					overlap += (float) (pIntensityRange - 1) - image[columns[c].potentialSynapses[s]];
				else overlap += (float) image[columns[c].potentialSynapses[s]];
			}
		}
		//printf("Overlap for %d = %.2f\n", c, overlap);
		if(overlap < minOverlap)	{ // CHANGE 1 <= to <
			columns[c].overlap = 0.0;
			if(columns[c].overlapCycle[overlapCounter] == 1)	{
				columns[c].overlapCycle[overlapCounter] = 0;
				--(columns[c].overlapDutyCycle);
			}
		}
		else	{
			columns[c].overlap = overlap * columns[c].boost;			
			if(columns[c].overlapCycle[overlapCounter] == 0)	{
				columns[c].overlapCycle[overlapCounter] = 1;
				++(columns[c].overlapDutyCycle);
			}
		}
		activityStore[instance][c] = overlapStore[instance][c] = round(columns[c].overlap);
	}
	//exit(0);
	if(++overlapCounter == pCycleWindow) overlapCounter = 0; 
}


void inhibitColumns()
{
	int c, l, distance, activityCount, differenceCounter;
	float overlap;
	
	differenceCounter = numActive = 0;
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];
		if((overlap = columns[c].overlap) > 0.0001)	{
			distance = activityCount = 0;
			while(++distance <= averageReceptiveFieldSize && activityCount < desiredLocalActivity)	{
				if(columns[columns[c].neighbour[distance]].overlap > overlap) ++activityCount;
			}
			if(activityCount < desiredLocalActivity)	{
				if(activeColumns[instance][numActive] != c)	{
					activeColumns[instance][numActive] = c;
					++differenceCounter;
				}
				++numActive;
				if(columns[c].activityDecay < pActivityDecay)
					columns[c].activityDecay = pActivityDecay;				
				if(columns[c].activityCycle[activityCounter] == 0)	{
					columns[c].activityCycle[activityCounter] = 1;
					++(columns[c].activeDutyCycle);
				}
			}
			else	{
				activityStore[instance][c] = 0;
				if(columns[c].activityCycle[activityCounter] == 1)	{
					columns[c].activityCycle[activityCounter] = 0;
					--(columns[c].activeDutyCycle);
				}
			}
		}
		else if(columns[c].activityCycle[activityCounter] == 1)	{
			columns[c].activityCycle[activityCounter] = 0;
			--(columns[c].activeDutyCycle);
		}
	}	
	if(differenceCounter > 0 || numActive != activeCount[instance]) ++changedCodes;
	if(++activityCounter == pCycleWindow) activityCounter = 0;
	activeCount[instance] = numActive;
}


void performLearning()
{
	int a, c, s, l, distance, activity, numP, maxDutyCycle, minDutyCycle, maxS, *nPtr;
	float intensity, newP, oldP, midP = pConnectedPerm - pPermanenceInc, maxP;
	
	if(stillLearning)	{
		for(a = 0; a < numActive; ++a)	{
			c = activeColumns[instance][a];
			numP = columns[c].numPotential;
			for(s = 0; s < numP; ++s) {
				oldP = columns[c].permanence[s];
				//CHANGE HERE
				if(columns[c].synapseInverted[s])
					intensity = ((pIntensityRange - 1) - image[columns[c].potentialSynapses[s]]) - 
							((pIntensityRange - 1) - meanIntensity);
				else intensity = image[columns[c].potentialSynapses[s]] - meanIntensity;

				// NUMENTA START	
				if(sNumenta)	{
					if(intensity > 0)	{
						if((newP = oldP + pPermanenceInc) > 1.0) newP = 1.0;
					}
					else if((newP = oldP - pPermanenceDec) < 0.0) newP = 0.0;
				}
				// NUMENTA END
				// NOT NUMENTA START
				else {
					// Increase connected synapses with +ve intensity to limit of 1.0
					if(intensity > 0 && oldP >= pConnectedPerm)	{
						if((newP = oldP + pPermanenceInc) > 1.0) newP = 1.0;
					}
					// Increase disconnected synapses with +ve intensity to limit of midP
					else if(intensity > 0 && oldP < pConnectedPerm)	{
						if((newP = oldP + pPermanenceInc) >= midP) newP = midP;
					}
					// Decrease disconnected synapses with -ve intensity to limit of 0.0
					else if((newP = oldP - pPermanenceDec) < 0.0) newP = 0.0;
				}
				// NOT NUMENTA END
				columns[c].permanence[s] = newP;
				// Keep track of changes to avoid recalculating receptive fields of unchanged columns
				if(newP < pConnectedPerm && oldP >= pConnectedPerm) ++(columns[c].synapsesOff);
				// NUMENTA START
				else if(sNumenta && newP >= pConnectedPerm && oldP < pConnectedPerm)	{
					++(columns[c].synapsesOn);
					// THIS IS MY CHANGE TO GET NUMENTA TO WORK:
					// if(columns[c].boost > 1.0) columns[c].boost = 1.0;
					columns[c].activityDecay = pActivityDecay;
				}
				// NOT NUMENTA END
			}
		}
	}
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];
		if(columns[c].activityDecay == 0)	{
			// Only test for boosting after the decay period
			maxDutyCycle = distance = 0;
			nPtr = columns[c].neighbour;
			while(++distance < averageReceptiveFieldSize)	{
				if((activity = columns[*(nPtr++)].activeDutyCycle) > maxDutyCycle) maxDutyCycle = activity;
			}
			if((minDutyCycle = round(pMinDutyThreshold * maxDutyCycle)) < 1 && cycle) minDutyCycle = 1; 
			// minDutyCycle = round(pMinDutyThreshold * maxDutyCycle);
			// minDutyCycle = pMinDutyThreshold * maxDutyCycle; // CHANGE 5 round and 7 < 1
			if(columns[c].activeDutyCycle < minDutyCycle) {
				columns[c].boost += pBoostInc;
				++numBoost;
				stillLearning = 1;
				// NOT NUMENTA START
				if(!sNumenta && columns[c].boost > pMaxBoost)	{
					// Turn on closest disconnected synapse with highest permanence
					numP = columns[c].numPotential;
					maxP = 0.0; maxS = 0;
					for(s = 0; s < numP; ++s)
						if((oldP = columns[c].permanence[s]) > maxP && oldP < pConnectedPerm) 
						{ maxP = oldP; maxS = s; }
					//if(maxP > 0.01)	{ // CHANGE 8
					columns[c].activityDecay = pActivityDecay;
					columns[c].permanence[maxS] = pConnectedPerm + pPermanenceInc; 
					++(columns[c].synapsesOn);
					columns[c].boost = 1.0; 
					//}
				}
				// NOT NUMENTA END
			}
			else	{
				columns[c].activityDecay = pActivityDecay * (columns[c].activeDutyCycle - minDutyCycle);
				// START NUMENTA (THIS WAS THE WAY NUMENTA HAS IT)	
				if(sNumenta && columns[c].boost > 1.0) columns[c].boost = 1.0;
				//END NUMENTA
			}
			// NUMENTA START
			if(sNumenta && columns[c].overlapDutyCycle < minDutyCycle)	{
				numP = columns[c].numPotential;
				++numInc;
				for(s = 0; s < numP; ++s)	{
					oldP = columns[c].permanence[s];
					if(oldP < pPermanenceMinimum) newP = pPermanenceMinimum;
					else if((newP = oldP * pPermanenceMultiplier) > 1.0) newP = 1.0;
					columns[c].permanence[s] = newP; 
					if(newP >= pConnectedPerm && oldP < pConnectedPerm)		{
						columns[c].activityDecay = pActivityDecay;
						++(columns[c].synapsesOn);
					}
				}
			}
			// NUMENTA END
		} else if(columns[c].activityDecay > 0) --(columns[c].activityDecay);
	}
}


void updateAverageReceptiveFieldSize()
{
	int c, s, l, sPos, numC, numP;
	int x, y, xy, xMax, xMin, yMax, yMin, xyArea;
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];
		if(columns[c].synapsesOn || columns[c].synapsesOff)	{
			// Only update columns where synapses have been turned on or off
			totalReceptiveFieldSize -= columns[c].receptiveFieldSize;
			totalConnections -= columns[c].numConnected;
			numC = columns[c].synapsesOn = columns[c].synapsesOff = 0;
			numP = columns[c].numPotential;	
			// Always include the synapse under the column in the receptive field
			xMax = xMin = X_VAL(c);
			yMax = yMin = Y_VAL(c);
			
			for(s = 0; s < numP; ++s)	{
				if(columns[c].permanence[s] >= pConnectedPerm)	{
					++numC;
					sPos = columns[c].potentialSynapses[s];
					x = X_VAL(sPos);	y = Y_VAL(sPos);
					// Find the extreme edges of the receptive field
					if(x > xMax) xMax = x; if(y > yMax) yMax = y;
					if(x < xMin) xMin = x; if(y < yMin) yMin = y;
				}
			}
			// Find the area of the circle that fits within the receptive field's edges
			xy = xMax - xMin + yMax - yMin + 2;
			if((xyArea = round((PI * xy * xy)/16.0)) > COLUMNS) xyArea = COLUMNS;
			
			columns[c].receptiveFieldSize = xyArea;
			totalReceptiveFieldSize += xyArea;
			columns[c].numConnected = numC;
			totalConnections += numC;
		}
	}
	averageReceptiveFieldSize = totalReceptiveFieldSize/numLiving;
	desiredLocalActivity = round(averageReceptiveFieldSize * pDesiredLocalActivity); //CHANGE
	meanConnections = totalConnections/numLiving; //CHANGE 4
}


void testForConvergence()
{
	int c, s, l, numP, changedSynapses = 0;
	float lastP, thisP;
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];		
		numP = columns[c].numPotential;
		for(s = 0; s < numP; ++s)	{
			if(fabs((lastP = lastPermanence[c][s]) - (thisP = columns[c].permanence[s])) > 0.0001)	{
				++changedSynapses;
			}
			lastPermanence[c][s] = columns[c].permanence[s];
		}
		columns[c].activityDecay = 0; 
	}
	if(sReportProgress)	{
		printf("Cycle %3d Codes %6d Synapses %6d Boosts %6d Incs %4d field %3d\n", 
			   cycle, changedCodes, changedSynapses, numBoost, numInc, averageReceptiveFieldSize);
	}
	
	if(numBoost == 0 && numInc == 0) stillLearning = 0;
	else stillLearning = 1;
	
	if(!changedCodes && !changedSynapses && !stillLearning) stillActive = 0;
	changedCodes = numBoost = numInc = 0;
}


void randomiseIndex()
{
	int i, temp, randIndex;
	for(i = 0; i < numInstance; ++i)
	{
		randIndex = random()%numInstance;
		temp = inputIndex[randIndex];
		inputIndex[randIndex] = inputIndex[i];
		inputIndex[i] = temp;	
	}
//OLD RANDOMISATION - John's
/*
	int i, indexOne, indexTwo, indexOneIndex;
	
	for(i = 0; i < numInstance/2; ++i)	{
		indexOne = random()%numInstance;
		indexTwo = random()%numInstance;
		indexOneIndex = inputIndex[indexOne];
		inputIndex[indexOne] = inputIndex[indexTwo];
		inputIndex[indexTwo] = indexOneIndex;
	}*/
}


void printStats()
{
	int c, l, activity, distance, maxDutyCycle, minDutyCycle, *nPtr;
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];
		maxDutyCycle = distance = 0;
		nPtr = columns[c].neighbour;
		while(++distance < averageReceptiveFieldSize)
			if((activity = columns[*(nPtr++)].activeDutyCycle) > maxDutyCycle) maxDutyCycle = activity;
		minDutyCycle = pMinDutyThreshold * maxDutyCycle;
		
		printf("Column %4d olapCycle %6d activeCycle %5d minCycle %4d conn %3d field %3d boost %5.1f\n", 
			   c, columns[c].overlapDutyCycle, columns[c].activeDutyCycle, minDutyCycle,
			   columns[c].numConnected, columns[c].receptiveFieldSize, columns[c].boost);
	}
}


void standardiseColumnOverlaps(int **olapStore)
{
	int c, l, i;
	long int totalOverlap;
	long double x2Sum, mean, diff, stdDev;
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];		
		totalOverlap = 0;
		for(i = 0; i < numInstance; ++i) totalOverlap += olapStore[i][c];
		if(totalOverlap > 0)	{
			mean = (long double) totalOverlap / numInstance;
			x2Sum = 0.0;
			for(i = 0; i < numInstance; ++i)	{
				diff = olapStore[i][c] - mean;
				x2Sum += (diff * diff);
			}
			stdDev = sqrt(x2Sum / (numInstance - 1));
			for(i = 0; i < numInstance; ++i)
				normalisedOverlap[i][c] = (olapStore[i][c] - mean) / stdDev;
		}
	}
}


long double calculateLifetimeKurtosis()
{
	int c, l, i;
	long double x2Sum, x4Sum, diff, lifetimeKurtosisSum = 0.0;
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];		
		x2Sum = x4Sum = 0.0;
		for(i = 0; i < numInstance; ++i)	{
			diff = normalisedOverlap[i][c];
			x2Sum += (diff * diff);
			x4Sum += (diff * diff * diff * diff);
		}
		lifetimeKurtosisSum += numInstance * (x4Sum / (x2Sum * x2Sum)) - 3.0;
	}
	return (lifetimeKurtosisSum / numLiving);
}


long double calculatePopulationKurtosis()
{
	int i, l;
	long double x2Sum, x4Sum, diff, populationKurtosisSum = 0.0;
	
	for(i = 0; i < numInstance; ++i)	{
		x2Sum = x4Sum = 0.0;
		for(l = 0; l < numLiving; ++l)	{
			diff = normalisedOverlap[i][livingColumn[l]];
			x2Sum += (diff * diff);
			x4Sum += (diff * diff * diff * diff);
		}
		populationKurtosisSum += numLiving * (x4Sum / (x2Sum * x2Sum)) - 3.0;
	}
	return (populationKurtosisSum / numInstance);
}


void calculateKurtosis()
{	
	long double lifetimeKurtosis, populationKurtosis;
	
	//standardiseColumnOverlaps(overlapStore);
	//printf("Overlap kurtosis:  Lifetime %6.2Lf ", calculateLifetimeKurtosis());
	//printf("Population %6.2Lf\n", calculatePopulationKurtosis());	
	standardiseColumnOverlaps(activityStore);
	lifetimeKurtosis = calculateLifetimeKurtosis();
	populationKurtosis = calculatePopulationKurtosis();
	totalLifetimeKurtosis += lifetimeKurtosis;
	totalPopulationKurtosis += populationKurtosis;
	printf("Activity kurtosis: Lifetime %6.2Lf ", lifetimeKurtosis);
	printf("Population %6.2Lf\n", populationKurtosis);
}


void calculateEntropy()
{
	int p;
	long double posP, negP, pEntropy, meanPosP = 0.0, entropy = 0.0;
	
	for(p = 0; p < PIXELS; ++p)	{
		posP = entropySum[p]/(double)numInstance;
		if(posP <= 0.0000000000) posP = 0.000000001;
		else if(posP >= 1.000000000) posP = 0.9999999999;
		negP = 1.0 - posP;
		meanPosP += posP;
		pEntropy = -(posP * log2(posP)) - (negP * log2(negP));
		entropy += pEntropy;
	}
	meanPosP /= (double)PIXELS;
	entropy /= (double)PIXELS;
	printf("Entropy Range %d: Mean Pixel Value %.4Lf Entropy %.4Lf\n", pIntensityRange, meanPosP, entropy);
	
}


void performReconstruction()
{
	int x, y, err;
	long double sumSquaredErr = 0.0;
	long double RMSE;
	
	reconstructOriginalImage();
	reconstructFilterImage();
	
	for(x = 0; x < PIXELS; ++x)	{
		for(y = 0; y < PIXELS; ++y)	{
			err = reconstructedImage[x][y] - reconstructedFilterImage[x][y];
			sumSquaredErr += (err * err);
		}
	}
	RMSE = sqrt(sumSquaredErr/(long double)(PIXELS * PIXELS));
	
	printf("RMSE %.2Lf\n", RMSE);
	
	if(pImage >= 0)	{
		drawInstance(pImage);
		//drawReconstructedInstance(pImage);
		drawReconstructedFilter(pImage);
	}
}

/*
void reconstructFilterImage()
{
	int i, p, x, y, a, c, s, numA, numP, xStart, xEnd, yStart, yEnd, pSum; 
	int inputRange, filterRange, filterIncrement;
	double filterRescale;
	int newMax, newMin, meanDiff;
		
	for(i = 0; i < numInstance; ++i)	{
		numA = activeCount[i];
		pSum = filterMean[i] = 0;
		for(a = 0; a < numA; ++a)	{
			c = activeColumns[i][a];
			numP = columns[c].numPotential;
			for(s = 0; s < numP; ++s)	{
				if(columns[c].permanence[s] >= pConnectedPerm)	{
					// CHANGE HERE
					if(columns[c].synapseInverted[s])	{
						filter[i][columns[c].potentialSynapses[s]] -= (FILTER_SCALE * columns[c].boost);
						filterMean[i] -= (FILTER_SCALE * columns[c].boost);
					}
					else	{
						filter[i][columns[c].potentialSynapses[s]] += (FILTER_SCALE * columns[c].boost);
						filterMean[i] += (FILTER_SCALE * columns[c].boost);
					}
				}
			}
			pSum += numP;
		}
		if(pSum) filterMean[i] /= pSum;		
	}
	
	for(i = 0; i < numInstance; ++i)	{
		minFilter[i] = MAX_INTENSITY;
		maxFilter[i] = p = 0;
		xStart = imageCoordinates[i][X] - 1;
		yStart = imageCoordinates[i][Y] - 1;
		xEnd = xStart + SIDE;
		yEnd = yStart + SIDE;
		for(x = xStart; x < xEnd; ++x)	{
			for(y = yStart; y < yEnd; ++y)	{
				if(filter[i][p] > maxFilter[i]) maxFilter[i] = filter[i][p];
				if(filter[i][p] < minFilter[i]) minFilter[i] = filter[i][p];
				++p;
			}
		}
		p = 0;
		filterRange = maxFilter[i] - minFilter[i];
		inputRange = maxInput[i] - minInput[i];
		meanDiff = meanInput[i] - filterMean[i];
		if(filterRange) filterRescale = (double)inputRange/(double)filterRange;
		else filterRescale = 0.0;
		filterIncrement = minInput[i] - round((double)filterRescale * (double)minFilter[i]);
		for(x = xStart; x < xEnd; ++x)	{
			for(y = yStart; y < yEnd; ++y)	{
				filter[i][p] = round((double)filter[i][p] * filterRescale);
				filter[i][p] += filterIncrement;// + meanDiff;
				//filter[i][p] *= round((float)meanInput[i]/(1.0 * (float)filterMean[i]));
				//if(filter[i][p] > maxInput[i]) filter[i][p] = maxInput[i];
				//else if(filter[i][p] < minInput[i]) filter[i][p] = minInput[i];
				reconstructedFilterImage[x][y] += filter[i][p++];
				++filterCount[x][y];
			}
		}
	}
	newMax = 0;
	newMin = MAX_INTENSITY;
	for(x = 0; x < PIXELS; ++x)	{
		for(y = 0; y < PIXELS; ++y)	{
			reconstructedFilterImage[x][y] /= filterCount[x][y];
			if(reconstructedFilterImage[x][y] > newMax) newMax = reconstructedFilterImage[x][y];
			if(reconstructedFilterImage[x][y] < newMin) newMin = reconstructedFilterImage[x][y];
		}
	}
	filterRange = newMax - newMin;
	inputRange = maxTotalInput - minTotalInput;
	if(filterRange) filterRescale = (double)inputRange/(double)filterRange;
	else filterRescale = 0.0;
	filterIncrement = minTotalInput - round((double)filterRescale * (double)newMin);
	for(x = 0; x < PIXELS; ++x)	{
		for(y = 0; y < PIXELS; ++y)	{
			reconstructedFilterImage[x][y] = round((double)reconstructedFilterImage[x][y] * filterRescale);
			reconstructedFilterImage[x][y] += filterIncrement;
		}
	}
}
//*/
//*
void reconstructFilterImage()
{
	int i, p, x, y, a, c, s, numA, numP, xStart, xEnd, yStart, yEnd; 
	int onSum, upperSum;
	
	for(i = 0; i < numInstance; ++i)	{
		numA = activeCount[i];
		onSum = upperSum = 0;
		for(a = 0; a < numA; ++a)	{
			c = activeColumns[i][a];
			numP = columns[c].numPotential;
			for(s = 0; s < numP; ++s)	{
				if(columns[c].permanence[s] >= pConnectedPerm)	{
					filter[i][columns[c].potentialSynapses[s]] += (FILTER_SCALE * columns[c].boost);
					//filterMean[i] += (FILTER_SCALE * columns[c].boost);
					filterMean[i] += FILTER_SCALE;
					++onSum;
				}
			}
		}
		for(p = 0; p < PIXELS; ++p)		{
			if(input[i][p] > meanInput[i])	{
				upperMean[i] += input[i][p];
				++upperSum;
			}
		}
		lowerMean[i] = ((meanInput[i] * PIXELS) - upperMean[i])/(PIXELS - upperSum);
		if(upperSum) upperMean[i] /= upperSum;
		if(onSum) filterMean[i] /= onSum;
		//printf("Filter %d onSum %d\n", i, onSum);
	}
	for(i = 0; i < numInstance; ++i)	{
		p = 0;
		xStart = imageCoordinates[i][X] - 1;
		yStart = imageCoordinates[i][Y] - 1;
		xEnd = xStart + SIDE;
		yEnd = yStart + SIDE;
		for(x = xStart; x < xEnd; ++x)	{
			for(y = yStart; y < yEnd; ++y)	{
				if(filter[i][p] == 0) filter[i][p] = lowerMean[i]; 
				else filter[i][p] *= round((float)upperMean[i]/(float)filterMean[i]);
				//else filter[i][p] *= round((float)meanInput[i]/(float)filterMean[i]);
				if(filter[i][p] > maxInput[i]) filter[i][p] = maxInput[i];
				//if(filter[i][p] < minInput[i]) filter[i][p] = minInput[i];
				reconstructedFilterImage[x][y] += filter[i][p++];
				++filterCount[x][y];
			}
		}
	}
	for(x = 0; x < PIXELS; ++x)	{
		for(y = 0; y < PIXELS; ++y)		{
			if(filterCount[x][y]) reconstructedFilterImage[x][y] /= filterCount[x][y];
			else reconstructedFilterImage[x][y] = maxTotalInput;
		}
	}
}
//*/


void drawReconstructedFilter(int instanceID)
{
	int x, y;
	int sideLength = sqrt(numInstance);
	int xStart = (instanceID/sideLength) * 2;
	int yStart = (instanceID%sideLength) * 2;
	int xEnd = xStart + SIDE;
	int yEnd = yStart + SIDE;
	
	printf("Reconstructed filter %d:\n", instanceID);
	for(x = xStart; x < xEnd; ++x)	{
		for(y = yStart; y < yEnd; ++y)	{
			printf("%4d ", reconstructedFilterImage[x][y]);
			if(y == yStart + SIDE - 1) printf("\n");
		}
	}
}


void reconstructOriginalImage()
{
	int i, p, x , y, xStart, xEnd, yStart, yEnd;
	
	for(i = 0; i < numInstance; ++i)	{
		p = 0;
		xStart = imageCoordinates[i][X] - 1;
		yStart = imageCoordinates[i][Y] - 1;
		xEnd = xStart + SIDE;
		yEnd = yStart + SIDE;
		for(x = xStart; x < xEnd; ++x)	{
			for(y = yStart; y < yEnd; ++y)	{
				reconstructedImage[x][y] += input[i][p++];
				++reconstructionCount[x][y];
			}
		}
	}
	for(x = 0; x < PIXELS; ++x)	{
		for(y = 0; y < PIXELS; ++y)		{
			if(reconstructionCount[x][y]) reconstructedImage[x][y] /= reconstructionCount[x][y];
			else reconstructedImage[x][y] = maxTotalInput;
		}
	}
}


void drawReconstructedInstance(int instanceID)
{
	int x, y;
	int sideLength = sqrt(numInstance);
	int xStart = (instanceID/sideLength) * 2;
	int yStart = (instanceID%sideLength) * 2;
	int xEnd = xStart + SIDE;
	int yEnd = yStart + SIDE;
	
	printf("Reconstructed instance %d:\n", instanceID);
	for(x = xStart; x < xEnd; ++x)	{
		for(y = yStart; y < yEnd; ++y)	{
			printf("%4d ", reconstructedImage[x][y]);
			if(y == yStart + SIDE - 1) printf("\n");
		}
	}
}


void outputCodes()
{
	int i, c, f, numC, maxC = 0, minC = COLUMNS;
	long int sumC = 0, sumC2 = 0;
	int maxF = 0, minF = INSTANCES, sumF = 0, sumF2 = 0, numF = 0;
	float meanC, stdDevC, meanF, stdDevF;
	int codeLengthFrequency[COLUMNS];
	
	for(c = 0; c < COLUMNS; ++c) codeLengthFrequency[c] = 0;
	for(i = 0; i < numInstance; ++i)	{
		numC = activeCount[i];
		printf("Instance %d: ", i);
		for(c = 0; c < numC; ++c) printf("%d ", activeColumns[i][c]);
		printf("\n");
		if(numC > maxC) maxC = numC;
		if(numC < minC) minC = numC;
		sumC += numC;
		sumC2 += numC * numC;
		++(codeLengthFrequency[numC]);
	}
	meanC = (float)sumC/(float)numInstance;
	stdDevC = sqrt((float)sumC2/(float)numInstance - (meanC * meanC));
	for(c = 0; c <= maxC; ++c)	{
		f = codeLengthFrequency[c];
		if(f)	{
			if(f > maxF) maxF = f;
			if(f < minF) minF = f;
			sumF += f;
			sumF2 += f * f;
			++numF;
		}
		printf("CodeLength %3d Frequency %4d\n", c, f);
	}
	meanF = (float)sumF/(float)numF;
	stdDevF = sqrt((float)sumF2/(float)numF - (meanF * meanF));
	
	printf("Code lengths: Min %d Max %d Mean %.2f StdDev %.2f\n", minC, maxC, meanC, stdDevC);
	printf("Code length frequencies: Min %d Max %d Mean %.2f StdDev %.2f\n", minF, maxF, meanF, stdDevF); 
}


void drawImage(int instanceID)
{
	int p, intensity, minIntensity, maxIntensity, intensityRange;
	
	minIntensity = minInput[instanceID]; 
	maxIntensity = maxInput[instanceID];
	intensityRange = maxIntensity - minIntensity + 1;
	for(p = 0; p < PIXELS; ++p)		{
		intensity = input[instanceID][p];
		image[p] = ((intensity - minIntensity) * pIntensityRange)/intensityRange;
	}
	printf("Image %d:\n", instanceID);
	for(p = 0; p < PIXELS; ++p)	{
		if(p && p%SIDE == 0) printf("\n");
		printf("%4d ", image[p]);
	}
	printf("\n");
}


void drawInstance(int instanceID)
{
	int p;
	
	printf("Instance %d, mean %d max %d min %d:\n", 
		   instanceID, meanInput[instanceID], maxInput[instanceID], minInput[instanceID]);
	for(p = 0; p < PIXELS; ++p)	{
		if(p && p%SIDE == 0) printf("\n");
		printf("%4d ", input[instanceID][p]);
	}
	printf("\n");
}


void drawFilter(int instanceID)
{
	int a, c, s, numA, numP;
	
	numA = activeCount[instanceID];
	printf("\nInstance %d codes:\n", instanceID);
	for(a = 0; a < numA; ++a)	{
		c = activeColumns[instanceID][a];
		printf("Col %3d boost %.3f: ", c, columns[c].boost);
		numP = columns[c].numPotential;
		for(s = 0; s < numP; ++s)	{
			if(columns[c].permanence[s] >= pConnectedPerm)	{
				printf("%d ", columns[c].potentialSynapses[s]);
				filter[instanceID][columns[c].potentialSynapses[s]] += (FILTER_SCALE * columns[c].boost);
			}	
		}
		printf("\n");
	}
	
	printf("\nFilter %d:\n", instanceID);
	for(s = 0; s < SYNAPSES; ++s)	{
		if(s && s%SIDE == 0) printf("\n");
		if(filter[instanceID][s] == 0) printf("  -  ");
		else printf("%4d ", filter[instanceID][s]);
		
	}
	printf("\n");
	
	drawImage(instanceID);
}


void testReceptiveFieldSize()
{
	int s, c, l, numP, numC, sPos; 
	int x, y, xy, xyArea, xMax, xMin, yMax, yMin;
	long int xyTotal = 0, cTotal = 0;
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];
		numC = 0;
		numP = columns[c].numPotential;	
		xMax = xMin = X_VAL(c);
		yMax = yMin = Y_VAL(c);
		for(s = 0; s < numP; ++s)	{
			if(columns[c].permanence[s] >= pConnectedPerm)	{
				++numC;
				sPos = columns[c].potentialSynapses[s];
				x = X_VAL(sPos);	y = Y_VAL(sPos);
				if(x > xMax) xMax = x; if(y > yMax) yMax = y;
				if(x < xMin) xMin = x; if(y < yMin) yMin = y;
			}
		}
		xy = xMax - xMin + yMax - yMin + 2;
		if((xyArea = round((PI * xy * xy)/16.0)) > COLUMNS) xyArea = COLUMNS;
		xyTotal += xyArea;
		cTotal += numC;
		
		if(columns[c].receptiveFieldSize != xyArea)	{
			fprintf(stderr, "ERROR! Receptive field size is %d not %d for column %d\n",
					xyArea, columns[c].receptiveFieldSize, s);
			exit(1);
		}
		if(columns[c].numConnected != numC)	{
			fprintf(stderr, "ERROR! numConnected is %d not %d for column %d\n",
					numC, columns[c].numConnected, s);
			exit(1);
		}
	}
	
	if(totalReceptiveFieldSize != xyTotal)	{
		fprintf(stderr, "ERROR! Total receptive field size is %ld not %d\n",
				xyTotal, totalReceptiveFieldSize);
		exit(1);
		
	}
	if(totalConnections != cTotal)	{
		fprintf(stderr, "ERROR! Total connections are %ld not %d\n",
				cTotal, totalConnections);
		exit(1);
		
	}
}


void drawColumns()
{
	int s, c, l, numP; 
	char cellImage[SYNAPSES];
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];
		
		for(s = 0; s < SYNAPSES; ++s) cellImage[s] = ' ';
		numP = columns[c].numPotential;
		for(s = 0; s < numP; ++s)	{
			if(columns[c].permanence[s] >= pConnectedPerm)
				cellImage[columns[c].potentialSynapses[s]] = 'X';
			else cellImage[columns[c].potentialSynapses[s]] = '-';
		}
		if(cellImage[c] == 'X') cellImage[c] = 'O';
		else cellImage[c] = '#';
		printf("Column %d Connected %d Field Size %d\n", 
			   c, columns[c].numConnected, columns[c].receptiveFieldSize);
		for(s = 0; s < SYNAPSES; ++s)	{
			if(s && s%SIDE == 0) printf("\n");
			printf("%c ", cellImage[s]);
		}
		printf("\n");
	}
}


void drawNeighbours(int c)
{
	int distance, s, x, y;
	
	for(distance = 0; distance < SYNAPSES; ++distance)	{
		s = columns[c].neighbour[distance];
		x = X_VAL(s); y = Y_VAL(s);
		synapticImage[x][y] = distance;
	}
	printf("Column %d (row %d column %d) Neighbour Distances:\n", c, X_VAL(c), y = Y_VAL(c));
	for(x = 0; x < SIDE; ++x)	{
		for(y = 0; y < SIDE; ++y) printf("%3d ", synapticImage[x][y]);
		printf("\n");
	}
}


double elapsedSeconds(void)
{
	double answer;
	struct tms t;
	times(&t);
	answer = t.tms_utime + t.tms_cutime;
	return answer / 100;
}


#ifdef GRAPHICS


void displayArray()
{
	gfxData data;
	int i, x, y, p = 0; 
	int width, height, pixelScale, xStart;
	int filterP, invertedP;
	FILE *filterFile, *invertedFilterFile;
	double rescaler;
	
	rescaler = (double)(pIntensityRange - 1) / (double) maxTotalInput;
	if(sReconstruct)	{
		if(sInvert == 0) filterFile = fopen("filter.txt", "w");
		else if(sInvert == 1) invertedFilterFile = fopen("invertedFilter.txt", "w");
		else if(sInvert == 2)	{
			filterFile = fopen("filter.txt", "r");
			invertedFilterFile = fopen("invertedFilter.txt", "r");
		}
		numImages = 2;
		pixelScale = 4;
		width = height = PIXELS;
		for(y = 0; y < PIXELS; ++y)	{
			for(x = 0; x < PIXELS; ++x)	{
				displayImage[0][p] = round(rescaler * (double) reconstructedImage[PIXELS - y - 1][x]);
				displayImage[1][p] = round(rescaler * (double) reconstructedFilterImage[PIXELS - y - 1][x]);
				if(sInvert == 0)	{
					fprintf(filterFile, "%d ", displayImage[1][p]);
				}
				else if(sInvert == 1)	{
					displayImage[0][p] = (pIntensityRange - 1) - displayImage[0][p];
					displayImage[1][p] = (pIntensityRange - 1) - displayImage[1][p];
					fprintf(invertedFilterFile, "%d ", displayImage[1][p]);
				}
				else {
					fscanf(filterFile, "%d ", &filterP);
					fscanf(invertedFilterFile, "%d ", &invertedP);
					displayImage[1][p] = (filterP + invertedP)/2;
				}
				++p;
			}
		}
		//calculateError();
		drawBox();
		if(sInvert == 0) fclose(filterFile);
		else if(sInvert == 1) fclose(invertedFilterFile);
		else if(sInvert == 2)	{
			fclose(filterFile);
			fclose(invertedFilterFile);
		}
	}
	else if(sDisplayColumns)	{
		numImages = 1;
		pixelScale = 1;
		width = height = SCREEN;
		createColumnDisplay();
	}
	else	{
		numImages = numInstance;
		reconstructFilterImage();
		pixelScale = 25;
		width = height = SIDE;
		for(i = 0; i < numInstance; ++i)	{
			p = 0;
			while(p < PIXELS)	{
				xStart = SIDE * (SIDE - p/SIDE - 1);
				for(x = 0; x < SIDE; ++x)	{
					//displayPatch[i][p + x] = round(rescaler * (double) input[i][xStart + x]);
					displayPatch[i][p + x] = round(rescaler * (double) filter[i][xStart + x]);
				}
				p += SIDE;
			}
		}
	}
	
	//if(sInvert == 2)	{
		init(&data,width*pixelScale,height*pixelScale,width,height,pixelScale,"/Users/John/Research/HTM/Code/Spatial/Data/");
		while(data.running)		{
			updateArrayInput(&data, 50);
			renderArray(&data);
			handleEvents(&data);
			SDL_Delay(1);
		}
		destroyData(&data);
	//}
}

void drawBox()
{
	int x, y, p = 0;
	
	for (x = 0; x < PIXELS; ++x) {
		for (y = 0; y < PIXELS; ++y) {
			if((x == SIDE * 8 || x == SIDE * 12) && y < 16) 
				displayImage[1][p] = 0; //INTENSITY_RANGE - 1; 
			else if(y && y < PIXELS - 1 && y%SIDE == 0 && y/SIDE == 1 && x > (SIDE * 8) - 1 && x < (SIDE * 12) + 1) 
				displayImage[1][p] = 0; //INTENSITY_RANGE - 1;
			++p;
		}
	}
}


void createColumnDisplay()
{
	int c, l, x, y, p = 0, border = (SIDE * P_SCALE) + 1;
	int numP, s, ps, sVal, xStart, yStart, sx, sy, sxStart, syStart, sxEnd, syEnd;
	float maxB = 0.0;
	
	
	for (x = 0; x < SCREEN; ++x) {
		for (y = 0; y < SCREEN; ++y) {
			displayColumns[0][p] = 0;
			if(x < SCREEN - 1 && (x + 1)%(border) == 0) displayColumns[0][p] = INTENSITY_RANGE - 1; 
			else if(y < SCREEN - 1 && (y + 1)%(border) == 0) displayColumns[0][p] = INTENSITY_RANGE - 1;
			++p;
		}
	}
	for(l = 0; l < numLiving; ++l) if(columns[livingColumn[l]].boost > maxB) maxB = columns[livingColumn[l]].boost;
	
	for(l = 0; l < numLiving; ++l)	{
		c = livingColumn[l];
		numP = columns[c].numPotential;
		for(ps = 0; ps < numP; ++ps)	{
			sVal = round((columns[c].boost/maxB) * (double)(INTENSITY_RANGE - 1));
			if(columns[c].permanence[ps] >= pConnectedPerm)	{
				s = columns[c].potentialSynapses[ps];
				x = X_VAL(c); y = Y_VAL(c); sx = X_VAL(s); sy = Y_VAL(s);
				xStart = x * border; yStart = y * border;
				sxStart = xStart + (sx * P_SCALE); syStart = yStart + (sy * P_SCALE);
				sxEnd = sxStart + P_SCALE; syEnd = syStart + P_SCALE;
				for (x = sxStart; x < sxEnd; ++x) {
					for(y = syStart; y < syEnd; ++y) displayColumns[0][(x * SCREEN) + y] = sVal; 
				}
			}
		}
	}
}


void updateArrayInput(gfxData* data, int arraySize)
{	
	if(data->instanceInc == 0) return;
	data->instanceIndex += data->instanceInc;
	if(data->instanceIndex < 0) data->instanceIndex = 0;
	else if(data->instanceIndex >= numImages) data->instanceIndex = numImages - 1;
	printf("instance %i\n", data->instanceIndex);
	data->instanceInc = 0;
	if(sReconstruct) data->patch = &displayImage[data->instanceIndex][0];
	else if(sDisplayColumns) data->patch = &displayColumns[data->instanceIndex][0]; 
	else data->patch = &displayPatch[data->instanceIndex][0];
}


void calculateError()
{
	int p, err, improving = 1;
	long double sumSquaredErr = 0.0;
	long double RMSE, lastRMSE = MAX_INTENSITY;
	
	while (improving) {
		sumSquaredErr = 0.0;
		for(p = 0; p < PIXELS * PIXELS; ++p)	{
			if(displayImage[1][p] > pIntensityRange - 1) displayImage[1][p] = pIntensityRange - 1;
			err = displayImage[0][p] - displayImage[1][p];
			sumSquaredErr += (err * err);
		}
		RMSE = sqrt(sumSquaredErr/(long double)(PIXELS * PIXELS));
		if(RMSE > lastRMSE)		{
			for(p = 0; p < PIXELS * PIXELS; ++p)	{
				if(displayImage[1][p] < pIntensityRange - 1) --(displayImage[1][p]);
				RMSE = lastRMSE;
				improving = 0;
			}
		}
		else	{
			lastRMSE = RMSE;
			for(p = 0; p < PIXELS * PIXELS; ++p)	{
				if(++(displayImage[1][p]) > pIntensityRange - 1) 
					displayImage[1][p] = pIntensityRange - 1;
			}
		}
	}

	printf("RMSE %.2Lf\n", RMSE);
}

#endif