#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <mpi.h>
#include <cilk/cilk.h>
#include <cilk/cilkscale.h>
#include <cilk/cilk_api.h>
#include "parall_cilk_mpi.h"


static double now_seconds(void){
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);

	return (double)t.tv_sec + t.tv_nsec * 1e-9;
}


//------ IMPLEMENT REDUCER FOR VARIABLE: 'CHANGED' ------
void bool_or_identity(void* view){
	*(int* )view=0;
}

void bool_or_reduce(void* left, void* right){
	int a = *(int* )left;
        int b = *(int* )right;
	*(int* )left = a | b;
}
//-------------------------------------------------------


int coloring_cc(uint32_t global_rows, CSR_info csr, uint32_t* global_labels, int max_iters, int rank, int size){
	
	uint32_t local_rows = csr.local_rows;
	uint32_t rows_start_idx = csr.rows_start_idx;
	const uint32_t* row_ptr = csr.row_ptr;
	const uint32_t*  col_idx = csr.col_idx;

	if(!row_ptr||!col_idx||!global_labels) return -1;
	if(global_rows <=0 || local_rows<=0) return -1;
	
	//Calculate MPI counts and displscements(for scattering chunks of data)
	uint32_t* receive_counts = (uint32_t*) malloc(sizeof(uint32_t)*size);
	uint32_t* dspls = (uint32_t*) malloc(sizeof(uint32_t)*size);
	uint32_t rem = global_rows%size;
	uint32_t base_chunk = global_rows/size;
	
	dspls[0]=0;
	for(int i=0; i<size; i++){
		receive_counts[i] = base_chunk + (i<rem? 1 : 0);
		if(i<size -1){
			dspls[i+1] = dspls[i] + receive_counts[i];
		}
	}

	int iterations =0;
	int cilk_reducer(bool_or_identity, bool_or_reduce)changed = 1;
	int global_changed = 1;

	while(global_changed){
		++iterations;
		changed = 0;  //Reset local change variable

		//OpenCilk parallelization - 'cilk_for' loop
		cilk_for(uint32_t v=0; v<local_rows; v++){
			uint32_t global_vert = rows_start_idx + v;

			//Union find algorithm's path compression characteristic
			uint32_t best = global_labels[global_vert];
			while(best>global_labels[best-1]){
				best = global_labels[best-1];
			}

			//Case where label is 1, can't get any lower/better
			if(best==1){
				if(best<global_labels[global_vert]){
					uint32_t pr_label = __atomic_fetch_min(&global_labels[global_vert],best, __ATOMIC_SEQ_CST);
					if(pr_label>best){changed=1;}
				}
				continue;    //skip neighbouring vertices
			}
			//Look at the neighbouring vertices
			for(uint32_t e=row_ptr[v]; e<row_ptr[v+1]; e++){
			uint32_t global_u = col_idx[e];
			if(global_u<0||global_u>=global_rows) continue;

			uint32_t lu = global_labels[global_u];
			if(lu<best){
				best =lu;
				//Early exit shortcut
				if(best==1)break;
			}

			}
			//Update
                        if(best<global_labels[global_vert]){
                                        uint32_t pr_label = __atomic_fetch_min(&global_labels[global_vert],best, __ATOMIC_SEQ_CST);
                                        if(pr_label>best){changed=1;}
                        }
                        
		}


		//Convergence check using MPI blocking synchronous command
		MPI_Allreduce(&changed, &global_changed,1,MPI_INT, MPI_LOR, MPI_COMM_WORLD);

		if(!global_changed) break;
		if(max_iters>0 && iterations>= max_iters) break;

		//Communication between ranks
		uint32_t* send_buffer = &global_labels[rows_start_idx];

		//blocking and synchronous
		MPI_Allgatherv(send_buffer, local_rows, MPI_UINT32_T, global_labels, receive_counts, dspls, MPI_UINT32_T, MPI_COMM_WORLD);
	}
	
	cilk_for(uint32_t v=0; v< local_rows; v++){
		uint32_t global_vert = rows_start_idx + v;
		uint32_t root = global_labels[global_vert];
		//Path compression
		while(root>global_labels[root-1]){
			root = global_labels[root-1];
		}

		global_labels[global_vert] = root;
	}

	//Synchronize roots after UF traversal
	uint32_t* send_buffer = &global_labels[rows_start_idx];

	MPI_Allgatherv(send_buffer, local_rows, MPI_UINT32_T, global_labels, receive_counts, dspls, MPI_UINT32_T, MPI_COMM_WORLD);

	if(rank==0){puts("Converged!");}

	//Cleaning operations
	free(receive_counts);
	free(dspls);

	return iterations;
}


int count_components(uint32_t n, const uint32_t* labels) {
    if (n <= 0 || !labels) return 0;

    /* labels are expected in the range 1..n (inclusive) */
    unsigned char *seen = calloc((size_t)(n + 1), 1); /* index 0 unused */
    if (!seen) return -1;

    int count = 0;
    for (int i = 0; i < n; ++i) {
        uint32_t lbl = labels[i];
        if (lbl < 1 || lbl > n) continue; /* skip invalid labels */
        if (!seen[lbl]) { seen[lbl] = 1; ++count; }
    }

    free(seen);
    return count;
}


void print_sample_labels(uint32_t n, const uint32_t* labels, int max_print) {
    int limit = (n < max_print) ? n : max_print;
    /* Print vertex indices 1-based to match label numbering */
    for (int i = 0; i < limit; ++i) printf("%d:%u ", i + 1, labels[i]);
    if (limit > 0) printf("\n");
}
