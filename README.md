PTHREAD Projects
==================

Project Overview
-------------------
This project contains multiple exercises to learn, understand, and practice using the `pthread` library.
It is a preliminary project for [**SpiceC - Programming Multicores for Parallelism**](http://www.cs.ucr.edu/~gupta/research/Projects/multi1.html) at [UC Riverside](http://ucr.edu).

Installation Guide / How to Use
-------------------------------
All exercises may be found in their respective folders under `/src`. These exercises should also have their respective Makefiles, which you can type `make` to compile and runs.

If a Makefile is not present, you may use `g++ -pthread -std=c++11 [name of definition(s) and declaration(s)]` as well as any preferred flags.

To run the file, enter the `/bin` directory, and type `make check ARGS="<number>"`, where `<number>` is passed into `num_threads`.

Projects
-----------------------------

### Heat Distribution

A heat distribution simulation on a thin metal plate with constant (isothermal) temperatures on each side that is modeled using a 2D array. 

##### Basic algorithm:

1. Read from the input file the initial temps for top, right, bottom, and left sides of plate.

2. Read from the input file the tolerance for equilibrium.

3. Initialize the edges of the 2D grid with initial temps you got from the input file, and initialize the

inner cells of the grid to 0.0.

4. Distribute the array by columns to each thread (`num_threads` passed in as a command line argument)

5. Continue updating temperature values within inner cells until equilibrium is reached.

6. Output to the output file the values of the inner cells of the grid after equilibrium obtained.

##### Bugs / Known Issues / Limitations:
- Child threads exit before doing work because max_difference will always be smaller than the tolerance at the start.

### Riemann Sum with `r_barrier_wait ()`

A riemann sum calculator that takes in the 3 parameters through an input file (`left bound`, `right bound`, `number of parts`).

##### Basic algorithm:

1. Read from the input file the left and right bound of the equation to compute, as well as the number of parts (accuracy).

2. Read from the input file the number of parts (accuracy).

3. Distribute the array by colums to each thread (`num_threads` passed in as a command line argument)

4. Compute the local sum (the work done in each thread).

5. If a thread completes its work, use `r_barrier_wait()` to check if work sharing is available.

6. Steal work if condition is met, otherwise wait at the `pthread_barrier_wait()`.

7. After all work is completed, compute the global sum, and print the solution to the console.

##### Bugs / Known Issues / Limitations

- None.

