# Parallel and Distributed Systems - HW2

## Introduction
This git repository contains the source code and representative testing results from a hybrid 
parallelization implementation of a BFS-based label propagation algorithm using OpenMPI 5 and 
OpenCilk 2. The project's program is written in C language and is intended to be run in Linux OS computers or
WSL 2 as well as Aristotle HPC. It supports reading and loading a graph's adjacency matrix from Matlab Sparse form in a .mat file, converting 
it to CSR format, storing it in a data structure and executing the LP algorithm, both in a sequential and parallel 
way.

## Program Structure and Description
This repository has 4 parts:
* Testing Results from Aristotle HPC in separate directories
    * "Sequential_Results": Contains sequential execution output and running time of the algorithm for each matrix
    * "Parallel_Results": Contains parallel execution output and running time of the algorithm for each matrix with varying parameters
* Hybrid Parallelization directory "mpi_implementation". It contains source code, header files, a Makefile and an example slurm script for submitting jobs to Aristotle's queues, for building and testing the program
* Sequential part(the main repository/not a directory). Include C files and headers for the sequential algorithm program, a Makefile, a .gitignore file, a slurm job script and this README.md

### How the sequential approach works
The sequential program demonstrates a typical label propagation iterative algorithm, executed by the coloring_cc() function 
in the 'labelprop_seq.c' source file. The graph itself, stored in sparse Matlab format(CSC format extended with Matlab information and data)
is loaded, converted to CSR format, for faster traversal and saving memory space and 'kept' in a  C struct object created especially for this project('mat_to_csr.h') with the mat_to_csr.c file's code. The main function in 'main.c' file executes and coordinates the whole program, 
prints messages related to the progress  or possible errors and calls the functions mentioned above, as well as secondary ones, for counting the SCCs of the graph 
based on labels or printing each label's value.

### How the parallel approach works
 The parallel approach generally follows the core of the sequential one, for practical reasons and the shake of simplicity regarding the .mat loading proccess. The 
first difference we notice is that the respective file for that part, 'mat_to_csr_ompi.c', uses only rank 0 for loading and conversion and then distributes equal(as much as possible) chunks to all of them, keeping one for itself too. Then, with the piece of code inside 'parall_cilk_mpi.c', we iterate at the same time
through all chunks using MPI and Cilk with a synchronous technique and blocking functions, a simple and safe approach. The algorithm in this implementation though is altered and enriched with Union-Find characteristics, but kept its initial iterative LP form, in order to increase speed and efficiency.

## Installation and execution in Aristotle-HPC
This project and especially its MPI implementation were developed and tested around usage in HPC clusters like Aristotelis. The following instructions show how to install
and test it in this environment:
1. Login to Aristotle's 'login node' from bash via SSH using EduVPN, or simply via the WebPortal and 'Open in Terminal' choice.
2. Install the project's repository in a directory of your choice manually, or, as it is advised, using git command: <br>
``` git clone https://github.com/ThomasKatsavos/PaDS-HW2.git ``` <br>
3. Download the matrices you want to test the program on using the appropriate link from [SuiteSparse website](https://suitesparse-collection-website.herokuapp.com/) for MATLAB (.mat) format with the command ``` wget <matrix link> ```.<br> Otherwise upload from your PC manually(10GB limit!).
!!! The matrix has to be in the same subdirectory with the executable AND the slurm script for its execution !!!

4. Configure you node - load modules needed for the correct execution of the programs. The command for this is: <br>
```module load gcc/13.2.0-iqpfkya  openmpi/5.0.3-rhzbeym  matio/1.5.26-xzs7hxf```<br>
and loads the MATIO library module, the OpenMPI module and a dependency -  gcc compiler version. The OpenCilk2 is likely already
pre-loaded and the path for its compiler is specified in the Makefile alrady. If this is not the case run:
```module load OpenCilk/v2.0```   or/and change the OMPI_CC variable in the Makefile with the path indicated after you execute the command:<br>
```echo $OPENCILK```
5. The project contains 2 Makefiles, one inside the "PaDS-HW2", outside of other subdirectories, responsible for building the sequential program and one inside the
"mpi_implementation" subdirectory, for building the hybrid parallel program. You need to execute ``` make ``` once when yout current working directory is the project's folder and one after you move to "mpi_implementation" with ``` cd mpi_implementation ``` and then ``` make ``` again.
6. Use the Slurm job script "hpc_test_script_example_seq.sh" or the one in the "mpi_implementation" subdir("hpc_test_script_example.sh") to create a sequential or parallelized (respectively) program job and test the program by submitting it
with the command <br> ``` sbatch  hpc_test_script_example.sh ```. Before that, you can configure it with an acceptable combination of nodes and CPUs count, MPI tasks count, maximum duraton, requested memory or cilk workers count (variable CILK_NWORKERS). For the MPI program it is strongly recommended to use the 'mpiexec' command for execution, with the format it already has in the script, and not 'srun'. The script loads some modules, which most of the time are not necessary but can help the user in case OpenMPI is unloaded between submitting and executing a job. In the last line of each script, in the execution command, the pre-selected matrix is 'com-Friendster'. You can change that and place a matrix of your choice. !! Reminder: the slurm script, .mat file and executable must be in the same subdirectory when submitting and running a job !!
7. View the results that are returned in the directory of each script with the form of output files, after the job is executed.

## Installation and execution in your linux PC or WSL2
 For the project's testing and ecaluation in a commercial Linux computer or WSL2 environment please follow the steps bellow:
 1. Install the project's repository in a directory of your choice manually, or, as it is advised, using git command: <br>
``` git clone https://github.com/ThomasKatsavos/PaDS-HW2.git ``` <br>
3. Download the matrices you want to test the program on using the appropriate link from [SuiteSparse website](https://suitesparse-collection-website.herokuapp.com/) for MATLAB (.mat) format with the command ``` wget <matrix link> ``` <br> . Otherwise move them from another folder  manually.
!!! The matrix has to be in the same subdirectory with the executable !!!
4. Make sure you have MATIO library and OpenCilk 2 downloaded and configured in your system. For the sequential program's compilation a gcc compiler is needed. For the parallel approach we use Cilk's clang compiler, so the user must change the path saved at variable OMPI_CC with the one pointing to where it is located in the current system (e.g. /home/...opencilk.../bin/clang).
5. Compile the program separately, with one make command for the sequential and another one inside "mpi_implementation" subdir for parallel method.
6. Run the sequential executable with this command and any .mat file:<br>
``` ./seq <filename_of_your_choice(.mat)> ```
Run the sequential executable with this command and any .mat file and MPI rank count:<br>
``` mpiexec -n <num_of_mpi_ranks> ./cilk_mpi <filename_of_your_choice(.mat)> ```<br>
The user can control the number of Cilk workers before each run with the command:<br>
``` export CILK_NWORKERS=<preferred number> ```<br>
e.g. ``` export CILK_NWORKERS=8 ```
7. View the results printed in the command line.

## WARNING
The program is supposed to run big matrices like com-Friendster, as it was specially mentioned, in the Aristotelis HPC cluster but in commercial computers too. In order to keep this interoperability trade-off, it was decided to use uint32_t data type for storing the largest integer values of the project, all of them non negative of course. This limits the size of matrices we can process to that similar with com-Friendster, but keep the memory requirements low(comparing to using 64-bit integers for even bigger matrices).  


#   --- THE END ---
