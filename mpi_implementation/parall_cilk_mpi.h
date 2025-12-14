#ifndef PARALL_CILK_MPI_H
#define PARALL_CILK_MPI_H

#include <stdint.h>
#include <mpi.h>
typedef struct{
	uint32_t global_rows;
	uint32_t nz_count;
	uint32_t local_rows;
	uint32_t rows_start_idx;
	uint32_t local_nz_count;
	uint32_t* row_ptr;
	uint32_t* col_idx;
	uint32_t* values;
}CSR_info;

void clean_csr_vectors(CSR_info vecs);

CSR_info mat_to_csr_mpi(char* filename, int rank, int size);

int coloring_cc(uint32_t global_rows, CSR_info csr, uint32_t* global_labels, int max_iters, int rank, int size);

int count_components(uint32_t n, const uint32_t* labels);
void print_sample_labels(uint32_t n, const uint32_t* labels, int max_print);

#endif
