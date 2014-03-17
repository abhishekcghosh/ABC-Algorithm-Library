/*	
	Benchmark test and example implementation of the ABC algorithm DLL using
	a 15-dimentional Ratrigins's function.	
	
	An n-Dimensional Rastrigin's function is defined as:
						  n
		f(x) = 10*n + Sigma { x_i^2 - 10*cos(2*PI*x_i) }
						i=1
	where  -5.12 < x_i < 5.12.
	Thus the global minima of the function being f(x) = 0 at all x_i = 0.
	
	Artificial Bee Colony (ABC) algorithm is one of the most recently defined algorithms 
	by Dervis Karaboga in 2005, motivated by the intelligent behavior of honey bees.
	
	Program written by Abhishek Ghosh (abhishek.c.ghosh@gmail.com)
*/



#include<stdio.h>								// required headers
#include<conio.h>
#include<math.h>
#include<time.h>
#include<stdlib.h>
#include<windows.h>
#include<string.h>

// ============================================================================================================================
// ************************************* LIBRARY INTERACTION PREREQUISITES ****************************************************
// *** EVERYTHING INSIDE THIS SECTION MUST BE IN YOUR CODE TOO. JUST COPY AND PASTE IT IF YOU DONOT NEED TO CHANGE ANYTHING ***
// ============================================================================================================================

#define MAX_DIM_COUNT 50 						// The max number of parameters of the problem to be optimized (structure definition)
												// This value MUST MATCH with what has been written in the ABC Library source code.
												// Default value is 50. Don't change if you are not sure.

#define MAXIMIZATION_PROBLEM 0					// Other macros for easy interaction with library
#define MINIMIZATION_PROBLEM 1
#define DONT_GENERATE_DATA_FILE 0
#define GENERATE_DATA_FILE 1
#define CONSTRAINTS_FAILED 0
#define CONSTRAINTS_PASSED 1

// create a food item structure and variables
struct foodSource {	
	double dim[MAX_DIM_COUNT];				
	double objFxValue; 						
	double fitness;
	double trials;
	double prob;	
};
foodSource bestSolution;

//input parameters for the run
int dimCount, colonySize, maxIterations, trialsLimit; 
double lB[MAX_DIM_COUNT], uB[MAX_DIM_COUNT];
char dumpFileName[100];

// dll import function pointers
typedef double (*objectiveFunction)(foodSource);
typedef int (*constraintFunction)(foodSource);
typedef void (*initABCFunc)(int, int, int, int, double[], double[], objectiveFunction, constraintFunction, int, char[]);
typedef foodSource (*runABCFunc)(int);

// ============================================================================================================================
// ************************************* END OF LIBRARY INTERACTION PREREQUISITES *********************************************
// ============================================================================================================================


// variables required for average value calculation
foodSource avgSolution; 
int runTimes;


// init abc algo parameters
void initABCParameters() {	
	dimCount = 15;				// attempting to solve a 10-dimensional Rastrigin
	colonySize = 20;			// with colony size 20
	maxIterations = 2000;		// maximum cycle number 2000
	trialsLimit = 100;			// and trials limit 100
	runTimes = 10;				// running the ABC algorithm 10 times to calculate some averages
	strcpy(dumpFileName, "abcRastriginDatasheets");
	avgSolution.dim[0] = 0;
	avgSolution.dim[1] = 0;
	avgSolution.objFxValue = 0;	
}


// calculate bounds
void calcBounds() {	
	int dimIndex;
	for (dimIndex = 0; dimIndex < dimCount; dimIndex++) {
		// set upper limit anad lower limit values to -5.12 to +5.12 
		// in each dimension in the function as per function definition
		lB[dimIndex] = -5.12;
		uB[dimIndex] = 5.12;
	}
}

// init variables and headers print
void initProg() {	
	printf("==============================================================================");
	printf("\n15-dim Rastrigin's function optimization benchmark test using Artificial Bee \nColony algorithm (defined by Dervis Karaboga, 2005).");
	printf("\n==============================================================================");
	printf("\n\n");
	// run initialization routines
	initABCParameters();
	calcBounds();
	// display init info	
	printf("\nStarting Optimisation routine...\n\nBee Colony Size: "); 
	printf("%d", colonySize);
	printf("\nMaximum Iterations: ");
	printf("%d", maxIterations);
	printf("\nTrials Limit: ");
	printf("%d", trialsLimit);
	printf("\nNumber of Algorithm Runs: ");
	printf("%d", runTimes);
	printf("\n\n------------------------------------------------------------------------------\n");		
}


// *** your objective function ***
// 20-DIM RASTRIGIN'S TEST
double rastriginsBenchmark(foodSource f) {
	double rastValue = 0;
	int dimIndex;
	for (dimIndex = 0; dimIndex < dimCount; dimIndex++) {
		rastValue += 10 + ( f.dim[dimIndex] * f.dim[dimIndex] - 10 * cos ( 2 * M_PI * f.dim[dimIndex] ) );
	}
	return rastValue;
}

// *** your constraints function ***
// check constraints validity for a particular solution
// returns 0 if not validated, non-zero if validated
int checkConstraints(foodSource f) {
	int validity = CONSTRAINTS_PASSED;		// since no additional constraint checks to be performed here
	return validity;	
}

// per run outputs on screen
void perRunOutput (int r, double result) {	
	if (r == -1) {
		printf("\nMEAN VALUE: "); 
	} else {
		printf("\nRUN: "); 
		printf("%d", r + 1); 
	}		
	printf("\n\tCONVERGENCE = "); 
	printf("%e", result); 
}

//main function
int main() {	
	// execution time calculations
	clock_t start, end;
	double cpuTimeUsed;	
	// init program parameters required to initiate abc
	initProg();	
	// typedef functions to hold what's in the dll
	initABCFunc _initABC;
	runABCFunc _runABC;
	HINSTANCE hInstLibrary = LoadLibrary("abcalgo32.dll");	
	if (hInstLibrary) {		
		// DLL load successful		
		// set up function pointers
		_initABC = (initABCFunc)GetProcAddress(hInstLibrary, "initABC");
		_runABC = (runABCFunc)GetProcAddress(hInstLibrary, "runABC");		
		// if _initABC loaded, run. else don't.
		if (_initABC) {			
			// initialize ABC
			_initABC(dimCount, colonySize, maxIterations, trialsLimit, lB, uB, &rastriginsBenchmark, &checkConstraints, MINIMIZATION_PROBLEM, dumpFileName);			
			// if _runABC loaded, run. else don't.
			// the actual optimization program working lies here
			if(_runABC) {				
				start = clock();
				int runs;
				for(runs = 0; runs < runTimes; runs++) {
					// obtain best solution
					bestSolution = _runABC(DONT_GENERATE_DATA_FILE);
					// update average solution
					avgSolution.dim[0] += bestSolution.dim[0];
					avgSolution.dim[1] += bestSolution.dim[1];
					avgSolution.objFxValue += bestSolution.objFxValue;
					// output this run's best solution
					perRunOutput(runs, bestSolution.objFxValue);
				}					
				// avg calc and output
				avgSolution.dim[0] /= runTimes;
				avgSolution.dim[1] /= runTimes;
				avgSolution.objFxValue /= runTimes;
				perRunOutput(-1, avgSolution.objFxValue);				
				end = clock();
			 	cpuTimeUsed = ((double) (end - start)) / CLOCKS_PER_SEC;	 	
				printf("\n\nTOTAL RUNTIME (secs): ");
				printf("%lf", cpuTimeUsed);
				printf("\nMEAN RUNTIME (secs): ");
				printf("%lf", cpuTimeUsed / runTimes);
				// exit protocol
				printf("\n\n==============================================================================");
				printf("\nEnd of program.");
			} else {				
				// runABC not loaded properly
				printf("\nDLL load error on function: runABC()");				
			}		
		} else {			
			// runABC not loaded properly
			printf("\nDLL load error on function: initABC()");			
		}		
		// after we are done with the library, free it
		 FreeLibrary(hInstLibrary);	
	} else {		
		// DLL load unsuccessful		
		printf("\nDLL Failed to load!");		
	}	
	printf("\n\nPress any key to continue...");	
	getch();	
	return 0;	
}





