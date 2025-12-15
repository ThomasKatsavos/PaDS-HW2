#!/bin/bash

#SBATCH --job-name=CILK_MPI_Scaling

#SBATCH --ntasks-per-node=1

#SBATCH --cpus-per-task=4

#SBATCH --nodes=2

#SBATCH --partition=rome

#SBATCH --time=00:20:00

#SBATCH --mem=64GB

module load  gcc/13.2.0-iqpfkya  openmpi/5.0.3-rhzbeym  matio/1.5.26-xzs7hxf

export CILK_NWORKERS=$SLURM_CPUS_PER_TASK

echo "--- Starting Job ---"

echo "MPI Tasks: $SLURM_NTASKS"

echo "Cilk Workers: $CILK_NWORKERS"

echo "--------------------"

mpiexec -n 2  ./cilk_mpi  com-Friendster.mat
