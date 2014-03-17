# Artificial Bee Colony Algorithm 
## Dynamic Link Library (DLL) v1

[Artificial Bee Colony (ABC) algorithm](http://en.wikipedia.org/wiki/Artificial_bee_colony_algorithm) is an optimization algorithm based on the intelligent foraging behaviour of honey bee swarm, proposed by [Dervis Karaboga]() in 2005.

### A brief guide on implementing the library:

The library implementation consists of two primary functions:

**`initABC()`** – the initialization function
 
**`runABC()`** – performs a single run of the algorithm and returns the solution obtained



Acoording to the design of the ABC algorithm, **possible solutions are denoted as food sources** that the honeybees try to investigate and forage. The model of a food source is implemented using a **C** structure which looks like this:

	struct foodSource {
    	double dim[MAX_DIM_COUNT];
    	double objFxValue; 
    	double fitness;
    	double trials;
    	double prob;
    };

Here, **`MAX_DIM_COUNT`** is a constant that indicates the maximum number of dimensions of a solution that the program can handle. 

To use the algorithm, it must be first initialized by invoking the **`initABC() `**function to which a number of parameters need to be passed, the prototype of which looks like this:

	void initABC (
			  int dimCount, 
			  int colonySize, 
			  int maxIterations, 
			  int trialsLimit, 
			  double lB[], 
			  double uB[], 
			  objectiveFunction objFxPtr, 
			  constraintFunction consFxPtr, 
			  int maxMin, 
			  char dumpFileName[]
	); 

The parameters have been described in detail:

- **`dimCount`** (*integer*) 						– 	number of **dimensions** or variables in the optimization problem. It is limited to a maximum value of **50** as set in the library source by default (can be modified).
   
- **`colonySize`** (*integer*) 						– 	**size** of the bee colony, i.e. the hive size. It is limited to a maximum value of **100** as set in the library source  by default (can be modified).

- **`maxIterations`** (*integer*) 					– 	maximum number of iterations (also known as **MCN** or the **maximum cycle number**) that the algorithm will attempt to go through before it terminates	deciding that it has obtained its solution 

- **`trailsLimit`** (*integer*) 					– 	number of **consecutively failed attempts** the algorithm will tolerate to improve a bad solution before it is discarded 

- **`lb`** (*double precision floating point array*) – 	a set of values denoting the **lower limits** of the various dimensions of the problem. Array length is hence equal to **`dimCount`** 

- **`ub`** (*double precision floating point array*) – 	a set of values denoting the **upper limits** of the various dimensions of the problem. Array length is hence equal to **`dimCount`** 

- **`objFxPtr`** 									– 	a **function pointer** that will hold the address of the **objective function** of the problem. Its return type is **`double`** and it accepts **one input parameter** of the **`foodSource`** type. 

- **`consFxPtr `**								– 	another **function pointer** that will hold the address of any additional function that will check whether a randomly generated possible solution to the problem does satisfy any other **constraints imposed by the problem** at hand if required. Its return type is integer and it accepts **one input parameter** of the **`foodSource`** type. It should return **1** if all constraints are **passed**, and **0** if one or more constraint check **fails** for the generated possible solution. 

- **`maxMin`** (*integer*) 							– 	an argument that denotes whether the problem to be solved is a **maximization** problem 													(pass the value **0**) or **minimization** problem (pass the value **1**) 

- **`dumpFileName`** (*string of maximum length **100***) – 	a string that holds the name of a **data-sheet text file** that will be outputed by the 	program containing the best solution generated per iteration in the algorithm, thus giving an **indication of the convergence** of the solution.

The next step in the process is much simpler. 

Invoke the **`runABC()`** function to 
perform a **single run** of the ABC algorithm and obtain a generated solution for that run. The generated solution returned by the function is of the **`foodSource`** type. 

The **`runABC()`** function takes **one input parameter** of the **integer** type which dictates whether to **generate a data-sheet** text file for this run (pass non-zero value is yes, or zero if no). Hence the **`runABC()`** function looks simply like this:

	foodSource runABC(int dumpData); 

The custom **function pointer type definitions** and the **primary function prototypes** mentioned above are provided as follows:

    typedef double (*objectiveFunction)(foodSource);
    
    typedef int (*constraintFunction)(foodSource);
    
    typedef void (*initABCFunc)(int, int, int, int, double[], double[], objectiveFunction, constraintFunction, int, char[]);
    
    typedef foodSource (*runABCFunc)(int); 


Thus, you can see that by simply invoking these two functions from a DLL, the ABC algorithm can be functionally run and implemented on any multi-modal optimization problem.



	
### The project contains 3 folders:

* **abcalgo32** - the source for a 32-bit implementation of the library in DEV-C++ (v5.4.1 used)
* **abcalgo64** - the source for a 64-bit implementation of the library in DEV-C++ (v5.4.1 used)
* **rastriginTest** - example code for calling the library (both 32-bit and 64-bit) and using it to find solution to the [Rastrigin's Function](http://www-optima.amp.i.kyoto-u.ac.jp/member/student/hedar/Hedar_files/TestGO_files/Page2607.htm). It contains the following files:
	* a 64-bit compiled DLL of the library (**abcalgo64.dll**) [compiled using TDM-GCC 4.7.1 64-bit]
	* a 64-bit version of a sample program using the library (**testRast64.cpp**)	
	* a 32-bit compiled DLL of the library (**abcalgo32.dll**) [compiled using TDM-GCC 4.7.1 32-bit]
	* a 32-bit version of a sample program using the library (**testRast32.cpp**)

	