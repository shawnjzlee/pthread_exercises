PTHREAD Exercises
==================

Project Overview
-------------------
This project contains multiple exercises to learn, understand, and practice using the `pthread` library.
It is a preliminary project for [**SpiceC - Programming Multicores for Parallelism**](http://www.cs.ucr.edu/~gupta/research/Projects/multi1.html) at [UC Riverside](http://ucr.edu).

Installation Guide / How to Use
-------------------------------
All exercises may be found in their respective folders under `/src`. These exercises should also have their respective Makefiles, which you can type `make` to compile and runs.

If a Makefile is not present, you may use `g++ -pthread -std=c++11 [name of definition(s) and declaration(s)]` as well as any preferred flags.

Bugs/Known Issues/Limitations
-----------------------------

#### Heat Distribution

- Child threads exit before doing work because max_difference will always be smaller than the tolerance at the start.

#### `r_barrier_wait ()`

- None so far!

Contributors
------------

- [Shawn Lee](https://github.com/shawnjzlee)
- [Stavan Thaker](https://github.com/sthak004)