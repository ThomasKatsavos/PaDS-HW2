#!/bin/bash
#SBATCH --job-name=seq_test 
#SBATCH --partition=ondemand
#SBATCH --nodes=1
#SBATCH --time=00:30:00
#SBATCH --mem=128GB


module load gcc/13.2.0-iqpfkya openmpi/5.0.3-rhzbeym

module load matio/1.5.26-xzs7hxf

echo "--- Starting Job ---"

./seq  com-Friendster.mat
