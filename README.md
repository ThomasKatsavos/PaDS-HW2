# Parallel and Distributed Systems - HW2

## Introduction
This git repository contains the source code and representative testing results from a hybrid 
parallelization implementation of a BFS-based label propagation algorithm using OpenMPI 5 and 
OpenCilk 2. The project's program is written in C language and is intended to be run in Linux OS computers or
WSL 2. It supports reading and loading a graph's adjacency matrix from Matlab Sparse form in a .mat file, converting 
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
The sequential program demonstrates a typical label propagation iterative algorithm, executed by the coloring_cc() function 
in the 'labelprop_seq.c' source file. The graph itself, stored in sparse Matlab format(CSC format extended with Matlab information and data)
is loaded, converted to CSR format, for faster traversal and saving memory space and 'kept' in a  C struct object created especially for this project('mat_to_csr.h') with the mat_to_csr.c file's code. The main function in 'main.c' file executes and coordinates the whole program, 
prints messages related to the progress  or possible errors and calls the functions mentioned above, as well as secondary ones, for counting the SCCs of the graph 
based on labels or printing each label's value.

### How the parallel approach works
 The parallel approach generally follows the core of the sequential one, for practical reasons and the shake of simplicity regarding the .mat loading proccess. The 
 first difference we notice is that the respective file for that part, 'mat_to_csr_ompi.c', uses only rank 0 for loading and conversion and then distributes equal(as much as possible) chunks to all of them, keeping one for itself too.  .................

## Installation and execution in Aristotle HPC
  ................
matio/1.5.26-xzs7hxf gcc/13.2.0-iqpfkya  openmpi/5.0.3-rhzbeym

## Installation and execution in your PC
 ...........................

#   --- THE END ---
