/*
	Artificial Bee Colony Algorithm 
	(algorithm defined by Dervis Karaboga, 2005)
	
	Dynamic Link Library (DLL) v1
	(program written by Abhishek Ghosh, abhishek.c.ghosh@gmail.com)
	
	Limitations of this Library (v1)
	> Maximum Dimensions supported is limited by the macro MAX_DIM_COUNT
	> Maximum Colony Size supported is limited by the macro MAX_COLONY_SIZE	
*/

// DECLDIR will perform an export for us
#define DLL_EXPORT

// required headers
#include<stdio.h>
#include<conio.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include<windows.h>
#include<string.h>

// all variables, structures, constant declared here
#define MAX_DIM_COUNT 10 						// The number of parameters of the problem to be optimized
#define MAX_COLONY_SIZE 100 					// The number of colony size (employed bees+onlooker bees)
#define MAX_FOOD_NUMBER MAX_COLONY_SIZE / 2 	// The number of food sources equals the half of the colony size

#define RANDOMIZE r = ( (double)rand() / ((double)(RAND_MAX) + (double)(1)) )		//randomize r value 

// create a food item structure and variables
struct foodSource {
	double dim[MAX_DIM_COUNT];				// dim[0] = length of link L1, dim[1] = length of link L2, dim[2] = angle THETA
	double objFxValue; 						// Sum of Square Errors (Beta_actual - Beta_theoritical)
	double fitness;
	double trials;
	double prob;
};
foodSource foods[MAX_FOOD_NUMBER], sol, globalBest;
double r;
int runCount, pCount, probMaxMin;
FILE * fp = NULL;
char fName[150], fNameClean[100], fCount[5];
		
//input parameters for the run
int vDIM_COUNT, vFOOD_NUMBER, vMAX_ITERATIONS, vTRIALS_LIMIT;
// lower and upper bounds (generic feature capability, for dimensions with different bound limits)
double lBound[MAX_DIM_COUNT], uBound[MAX_DIM_COUNT];

// functions that are library implementation dependent (objective function, constraints check)
typedef double (*objectiveFunction)(foodSource);
objectiveFunction objFxDLL;
typedef int (*constraintFunction)(foodSource);
constraintFunction consFxDLL;

// functions not exposed
// randomize using timer as see
void randomizeSeedTimer() {
	time_t seconds;
	time(&seconds);
	srand((unsigned int) seconds);
}

// fitness function
double calcFitness(double value) {
	double fitnessValue = 0;	
	if (probMaxMin == 0) {
		// maximization problem
		if (value >= 0) {
			fitnessValue = 1 + (value);		
		} else {
			fitnessValue = 1 / (1 + fabs(value));
		}	
	} else {
		// minimization problem
		if (value >= 0) {
			fitnessValue = 1 / (value + 1);		
		} else {
			fitnessValue = 1 + fabs(value);
		}	
	}	
	return fitnessValue;
}

// memorize the best food source
void memorizeBestFoodSource() {
	int count;
	for (count = 0; count < vFOOD_NUMBER; count++) {
		if (foods[count].objFxValue < globalBest.objFxValue) {
			globalBest = foods[count];
		}
	}
}

// initialize a single food source
void initFood(int index) {
	int dimIndex, constraintsPassed = 0; 
	//double lhsConstraint3, rhsConstraint3;	
	while(!constraintsPassed) {
		// generate values of l1, theta, and calc l2 = f(l1, theta)
		for (dimIndex = 0; dimIndex < vDIM_COUNT; dimIndex++) {
			RANDOMIZE;
			foods[index].dim[dimIndex] = lBound[dimIndex] + r * (uBound[dimIndex] - lBound[dimIndex]);					
		}
		//constraints check
		constraintsPassed = consFxDLL(foods[index]);
			
	}	
	foods[index].objFxValue = objFxDLL(foods[index]);
	foods[index].fitness = calcFitness(foods[index].objFxValue);	
	foods[index].trials = 0;
	foods[index].prob = 0;
}

// initialize all food sources 
void initFoodSources() {
	int foodIndex;
	for (foodIndex = 0; foodIndex < vFOOD_NUMBER; foodIndex++) {
		initFood(foodIndex);
	}
	globalBest = foods[0];
}

//employed bees phase
void sendEmployees() {
	int foodIndex, paramToChange, neighbour, constraintsPassed;
	for (foodIndex = 0; foodIndex < vFOOD_NUMBER; foodIndex++) {
		// random determine parameter to be changed
		RANDOMIZE;
		paramToChange = (int)(r * (vDIM_COUNT));		
		// random determine solution to produce a mutant solution of the solution [foodIndex]
		// also randomly selected neighbour must not the [foodIndex] solution itself		
		RANDOMIZE;
		neighbour = (int)(r * vFOOD_NUMBER);
		while (neighbour == foodIndex) {
			RANDOMIZE;
			neighbour = (int)(r * vFOOD_NUMBER);
		}
		// mutate and check generated parameter's boundaries
		sol = foods[foodIndex]; constraintsPassed = 0;
		while(!constraintsPassed) {
			RANDOMIZE;
			sol.dim[paramToChange] = foods[foodIndex].dim[paramToChange] + (foods[foodIndex].dim[paramToChange] - foods[neighbour].dim[paramToChange]) * (r - 0.5) * 2;
			if (sol.dim[paramToChange] < lBound[paramToChange]) {
				sol.dim[paramToChange] = lBound[paramToChange];			
			} else if (sol.dim[paramToChange] > uBound[paramToChange]) {
				sol.dim[paramToChange] = uBound[paramToChange];			
			}
			constraintsPassed = consFxDLL(sol);
		}
		// calc objective function value and fitness for the mutation
		sol.objFxValue = objFxDLL(sol);
		sol.fitness = calcFitness(sol.objFxValue);
		// greedily check if mutation is better than original
		if (sol.fitness > foods[foodIndex].fitness) {
			// if mutation is better,
			foods[foodIndex] = sol;			// replace original with mutation
			foods[foodIndex].trials = 0;	// reset trials count			
		} else {
			//failure, so increment trials count
			foods[foodIndex].trials++;
		}			
	}		
}

// calculate probability for onlooker selection
void calcProbabilities() {
	int foodIndex;
	double sumFitness = 0;
	//find sum fitness
	for (foodIndex = 0; foodIndex < vFOOD_NUMBER; foodIndex++) {
		sumFitness += foods[foodIndex].fitness;
	}
	//calc probalities using: prob_i = fit_i / sum (fit_i)
	for (foodIndex = 0; foodIndex < vFOOD_NUMBER; foodIndex++) {
		foods[foodIndex].prob = (0.9 * (foods[foodIndex].fitness / sumFitness)) + 0.1;		
		
	}	
}

//onlookers phase
void sendOnlookers() {
	int foodIndex, onlookerIndex, dimIndex, paramToChange, neighbour, constraintsPassed;
	foodIndex = 0;
	onlookerIndex = 0;
	while (onlookerIndex < vFOOD_NUMBER) {
		RANDOMIZE;
		if (r < foods[foodIndex].prob) {		// choose a food source dependig upon probability
			//chosen
			onlookerIndex++;
			// random determine parameter to be changed
			RANDOMIZE;
			paramToChange = (int)(r * (vDIM_COUNT));
			// random determine solution to produce a mutant solution of the solution [foodIndex]
			// also randomly selected neighbour must not the [foodIndex] solution itself		
			RANDOMIZE;
			neighbour = (int)(r * vFOOD_NUMBER);
			while (neighbour == foodIndex) {
				RANDOMIZE;
				neighbour = (int)(r * vFOOD_NUMBER);
			}
			// mutate and check generated parameter's boundaries
			sol = foods[foodIndex]; constraintsPassed = 0;
			while(!constraintsPassed) {
				RANDOMIZE;
				sol.dim[paramToChange] = foods[foodIndex].dim[paramToChange] + (foods[foodIndex].dim[paramToChange] - foods[neighbour].dim[paramToChange]) * (r - 0.5) * 2;
				if (sol.dim[paramToChange] < lBound[paramToChange]) {
					sol.dim[paramToChange] = lBound[paramToChange];			
				} else if (sol.dim[paramToChange] > uBound[paramToChange]) {
					sol.dim[paramToChange] = uBound[paramToChange];			
				}
				constraintsPassed = consFxDLL(sol);
			}
			// calc objective function value and fitness for the mutation
			sol.objFxValue = objFxDLL(sol);
			sol.fitness = calcFitness(sol.objFxValue);
			// greedily check if mutation is better than original
			if (sol.fitness > foods[foodIndex].fitness) {
				// if mutation is better,
				foods[foodIndex] = sol;			// replace original with mutation
				foods[foodIndex].trials = 0;	// reset trials count			
			} else {
				//failure, so increment trials count
				foods[foodIndex].trials++;
			}
		}
		foodIndex++;							// this foodIndex's work is done
		if (foodIndex >= vFOOD_NUMBER - 1) {		// keep resetting foodIndex when overflows till all onlookers are filled
			foodIndex = 0;
		}
	}
}

// scouts phase
// determine foods whose trials count exceeds trials limit, abandon it (one at a time per iteration) and randomize a new food source
void sendScouts() {
	int maxTrialsIndex, foodIndex;
	maxTrialsIndex = 0;
	for (foodIndex = 0; foodIndex < vFOOD_NUMBER; foodIndex++) {
		if (foods[foodIndex].trials > foods[maxTrialsIndex].trials) {
			maxTrialsIndex = foodIndex;
		}		
	}
	if (foods[maxTrialsIndex].trials > vTRIALS_LIMIT) {
		initFood(maxTrialsIndex);
	}
}

// Include our header, must come after #define DLL_EXPORT
#include "abcalgo.h"

// Get rid of name mangeling
extern "C"
{	// Define our exported functions	
	// initiate ABC required parameters
	DECLDIR void initABC(int dimCount, int colonySize, int maxIterations, int trialsLimit, double lB[], double uB[], objectiveFunction objFxPtr, constraintFunction consFxPtr, int maxMin, char dumpFileName[]) {
		// set parameter values
		vDIM_COUNT = dimCount;
		vFOOD_NUMBER = (int)(colonySize / 2);
		vMAX_ITERATIONS = maxIterations;
		vTRIALS_LIMIT = trialsLimit;	
		runCount = 0;
		probMaxMin = maxMin;
		strcpy(fNameClean, dumpFileName);
		// randomize r with timer
		randomizeSeedTimer();
		// set bounds
		int count;
		for (count = 0 ; count < dimCount; count++) {
			lBound[count] = lB[count];
			uBound[count] = uB[count];
		}
		// set objective fx and constraint fx pointers
		objFxDLL = objFxPtr;
		consFxDLL = consFxPtr;			
	}
	// main ABC algo run function (with data file dump support)
	DECLDIR foodSource runABC(int dumpData) {
		if (dumpData) {			
			strcpy(fName, fNameClean);
			sprintf(fCount, "%d", (runCount + 1));
			strcat(fName, fCount);
			strcat(fName, ".txt");
			fp = fopen(fName, "w+");
		}
		initFoodSources();
		memorizeBestFoodSource();
		int iteration;
		for (iteration = 0; iteration < vMAX_ITERATIONS; iteration++) {
			sendEmployees();												// employed bees phase
			calcProbabilities();											// calculate food source selection probailities for onlookers
			sendOnlookers(); 												// onlookers phase
			memorizeBestFoodSource();										// set best food source obtained this iteration
			sendScouts();													// scouts phase	
			// if file write true
			if (dumpData) {
				for(pCount = 0; pCount < vDIM_COUNT; pCount++) {
					fprintf(fp, "%e\t", globalBest.dim[pCount]);
				}
				fprintf(fp, "%e\n", globalBest.objFxValue);
			}
		}		
		if (dumpData) {
			fclose(fp);
		}
		runCount++;
		return globalBest;
	}	
}
