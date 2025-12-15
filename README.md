# Parallel and Distributed Systems - HW2

## Introduction
This git repository contains the source code and representative testing results from a hybrid 
parallelization implementation of a BFS-based label propagation algorithm using OpenMPI 5 and 
OpenCilk 2. The project's program is written in C language and is intended to be run in Linux OS computers or
WSL 2. It supports reading and loading a graph's afjacency matrix from Matlab Sparse form in a .mat file, converting 
it to CSR format, storing it in a data structure and executing the LP algorithm, both in a sequential and parallel 
way.

## Program Structure and Description
This repository has 4 parts:
* Testing Results from Aristotle HPC in separate directories
    * "Sequential_Results": Contains sequential execution output and running time of the algorithm for each matrix
    * "Parallel_Results": Contains parallel execution output and running time of the algorithm for each matrix with varying parameters
* Hybrid Parallelization directory "mpi_implementation". It contains source code, header files, a Makefile and an example slurm script for submitting jobs to Aristotle's queues, for building and testing the program
* Sequential part(the main repository/not a directory). Include C files and headers for the sequential algorithm program, a Makefile, a .gitignore file and this README.md

### How the sequential approach works
 .............

### How the parallel approach works
 .............

## Installation and execution in Aristotle HPC
  ................
matio/1.5.26-xzs7hxf gcc/13.2.0-iqpfkya  openmpi/5.0.3-rhzbeym

## Installation and execution in your PC
 ...........................

#   --- THE END ---
