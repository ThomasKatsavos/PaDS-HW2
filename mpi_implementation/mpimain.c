#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>
#include "parall_cilk_mpi.h"
#include <stdint.h>

//Function for time measurements
static double now_seconds(void) {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (double)t.tv_sec + t.tv_nsec * 1e-9;
}

int main(int argc, char *argv[]) {
    
    //Initializing MPI framework
    MPI_Init(&argc, &argv);
    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
	

    if(rank==0){
	printf("MPI structure created, number of ranks: %d\n", size);
    }
    
    //reading from input the .mat file's name
    const char *filename = "testmat2.mat";
    if(argc>=2){filename=argv[1];}

    //Loading and receiving the local part of the csr matrix for each rank
    CSR_info local_csr = mat_to_csr_mpi((char*)filename, rank, size);

    uint32_t global_rows = local_csr.global_rows;
    if(global_rows<=0){puts("Empty matrix!?");return 0;}
    uint32_t* global_labels = (uint32_t* ) malloc((uint32_t)global_rows * sizeof(uint32_t));
    
    if(!global_labels){
    	fprintf(stderr, "OOM for global labels vector! Rank: %d \n", rank);
	    clean_csr_vectors(local_csr);
    	MPI_Abort(MPI_COMM_WORLD, 2);
    }
    
    //Populate global labels array
    for(uint32_t i=0; i<global_rows; i++){
    	global_labels[i] = i+1;
    }
    
    //Run coloring cc algorithm
    double t1=now_seconds();
    int iters = coloring_cc(global_rows, local_csr, global_labels, 0,rank, size);
    double t2=now_seconds();

    printf("Time needed for rank %d is %.7lf sec, with %d iterations.\n",rank, t2-t1, iters);

    //Calculate the final number of connected components
    if(rank==0){
	int components = count_components(global_rows, global_labels);
    	printf("Connected components: %d\n", components);
    }

    //Cleaning operations
    free(global_labels);
    clean_csr_vectors(local_csr);
    MPI_Finalize();
    return 0;
}

