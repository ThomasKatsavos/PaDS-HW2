#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mat_to_csr.h"
#include "bfs.h"


static double now_seconds(void) {
	struct timespec t;
	clock_gettime(CLOCK_MONOTONIC, &t);
	return (double)t.tv_sec + t.tv_nsec * 1e-9;
}

int main(int argc, char *argv[]) {
    
    const char *fname = "testmat2.mat";
    if (argc >= 2) fname = argv[1];
    
    CSR_info csr = mat_to_csr((char *)fname);

	
    uint32_t rows = csr.nrows;
    uint32_t nz = csr.nzcount;
    printf("Loaded matrix:\n number of rows:%u,\n number of nonzeros:%u\n", rows, nz);
	
    //Access row-pointer vector
    
    uint32_t *labels = malloc((size_t )nz * sizeof(uint32_t));
    if (!labels) { fprintf(stderr, "OOM allocating labels\n");  clean_csr_vectors(csr); return 3; }
    

    double t0 = now_seconds();
    int iters = coloring_cc(rows, csr.row_ptr, csr.col_idx, labels, 0);
    double t1 = now_seconds();

    int comps = count_components(rows, labels);
    printf("Coloring CC: iterations=%d, time=%.6f sec, Connected components=%d\n", iters, t1 - t0, comps);
    
    //Print some labels
    if(comps>0){
        printf("First 20 labels (or fewer):\n");
        print_sample_labels(rows, labels, 20);
    }

    free(labels);
    clean_csr_vectors(csr);
	
    return 0;
}

