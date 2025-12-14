#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <matio.h>
#include <mpi.h>
#include <math.h>
#include "parall_cilk_mpi.h"
#include <string.h>


void clean_csr_vectors(CSR_info vecs){
    free(vecs.row_ptr);
    free(vecs.col_idx);
}

//Generated the equivalent of MPI_Scatterv() that accepts uint32_t type integers as 'sendcounts' arguments
void scatterv_uint32(const void *sendbuf, const uint32_t *sendcounts, const uint32_t *displs, MPI_Datatype sendtype,void *recvbuf, uint32_t recvcount, int root, MPI_Comm comm){
    int rank, size;
    MPI_Comm_rank(comm, &rank);
    MPI_Comm_size(comm, &size);
    int typesize;
    MPI_Type_size(sendtype, &typesize);

    if (rank == root) {
        for (int p = 0; p < size; p++) {
            const char *src = (const char *)sendbuf + (size_t)displs[p] * typesize;
            if (p == root) {
                if (recvbuf != src) {
                    memcpy(recvbuf, src, (size_t)recvcount * typesize);
                }
            } else {
                MPI_Send(src, sendcounts[p], sendtype, p, 0,comm);
            }
        }
    } else {
        MPI_Recv(recvbuf,recvcount, sendtype, root, 0, comm, MPI_STATUS_IGNORE);
    }
}

//This function is an expnasion of the 'mat_to_csr()' funstion that uses one MPI rank to load and convert the matrix from
// a .mat file and then, if there are more than 1 ranks, it distributes it across each one of them.
CSR_info mat_to_csr_mpi(char* filename, int rank, int size) {
    CSR_info local_csr = {0};
    uint32_t  n_global = 0;
    uint32_t  *global_row_ptr = NULL;
    uint32_t  *global_col_idx = NULL;

    // Here rank 0 loads the matrix
    if (rank == 0) {
        puts("Rank 0: Loading matrix structure...");
        
        mat_t *matfp = Mat_Open(filename, MAT_ACC_RDONLY);
        if (!matfp){
        fprintf(stderr, "MATIO->ERROR: cannot open %s file! \n", filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        matvar_t *projectvar = Mat_VarRead(matfp, "Problem");
        matvar_t *matvar = Mat_VarGetStructFieldByName(projectvar, "A", 0);

        if (!matvar || matvar->class_type != MAT_C_SPARSE){
        fprintf(stderr, "MATIO->ERROR: 'A' matrix missing in struct 'Problem' inside %s or matrix is not sparse!\n", filename);
        MPI_Abort(MPI_COMM_WORLD, 1);
        }
        
        mat_sparse_t *sparse = (mat_sparse_t*)matvar->data;
        n_global = matvar->dims[0]; 
        uint32_t  ncols = matvar->dims[1]; 
        uint32_t* row_ind = (uint32_t* )sparse->ir; 
        uint32_t* col_ptr = (uint32_t* )sparse->jc;

        //Start ptrocedure to change format from Matlab's CSC to CSR
        // Calculate CSR row_ptr
        global_row_ptr = (uint32_t*)calloc(n_global + 1, sizeof(uint32_t));
        for (uint32_t c = 0; c < ncols; c++) {
            for (uint32_t i = col_ptr[c]; i < col_ptr[c+1]; i++) {
                global_row_ptr[row_ind[i] + 1]++; 
            }
        }
        for (uint32_t i = 0; i < n_global; i++) {
            global_row_ptr[i+1] += global_row_ptr[i];
        }

        uint32_t total_nnz = global_row_ptr[n_global];
        global_col_idx = (uint32_t*)malloc(total_nnz * sizeof(uint32_t));

        //Calculate CSR col_idx
        uint32_t *cnt = (uint32_t*)malloc(n_global * sizeof(uint32_t));
        for (uint32_t i = 0; i < n_global; i++){ cnt[i] = global_row_ptr[i]; }

        for(uint32_t i = 0; i < ncols; i++){
            for(uint32_t j = col_ptr[i]; j < col_ptr[i+1]; j++){
                uint32_t r = row_ind[j];
                uint32_t dest = cnt[r]++;
                global_col_idx[dest] = i; 
            }
        }
        free(cnt);
        
        //Message to confirm that loading went OK, data printed help us verify the correctness early on
        printf("Rank 0: Global matrix loaded. N=%u, NNZ=%u. Starting to distribute...\n", n_global, total_nnz);
        Mat_VarFree(projectvar);
        Mat_Close(matfp);
    } 
    
    //Distribution procedure
    //Share global rows count with all ranks
    MPI_Bcast(&n_global, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);
    local_csr.global_rows = n_global;
    
    //Distribute rows local partitions
    
    //Standard MPI procedure, computing counts and displacements for buffers/sharing
    uint32_t *row_counts = (uint32_t*)malloc(size * sizeof(uint32_t));
    uint32_t *row_displs = (uint32_t*)malloc(size * sizeof(uint32_t));
    
    uint32_t remainder = n_global % size;
    uint32_t base_rows = n_global / size;
    row_displs[0] = 0;

    for (int i = 0; i < size; i++) {
        row_counts[i] = base_rows + (i < remainder ? 1 : 0);
        if (i < size - 1) row_displs[i+1] = row_displs[i] + row_counts[i];
    }
    
    local_csr.local_rows = row_counts[rank];
    local_csr.rows_start_idx = row_displs[rank];
    
    uint32_t *nnz_counts = NULL;
    uint32_t *nnz_displs = NULL;
    
    if (rank == 0) {
        nnz_counts = (uint32_t*)malloc(size * sizeof(uint32_t));
        nnz_displs = (uint32_t*)malloc(size * sizeof(uint32_t));
        
        for (int i = 0; i < size; i++) {
            uint32_t start_row = row_displs[i];
            uint32_t end_row = row_displs[i] + row_counts[i];
            nnz_counts[i] = global_row_ptr[end_row] - global_row_ptr[start_row];
            nnz_displs[i] = global_row_ptr[start_row];
        }
        local_csr.nz_count = global_row_ptr[n_global];
    }
    
    MPI_Scatter(nnz_counts, 1, MPI_UINT32_T, &local_csr.local_nz_count, 1, MPI_UINT32_T, 0, MPI_COMM_WORLD);
    
    local_csr.row_ptr = (uint32_t*)malloc((local_csr.local_rows + 1) * sizeof(uint32_t));
    local_csr.col_idx = (uint32_t*)malloc(local_csr.local_nz_count * sizeof(uint32_t));
    //'values' vector is not needed, we have it as a typical CSR component but keep it empty
    local_csr.values  = NULL; 
    //"Scatter" data to local vactors of each rank after chunking them
    scatterv_uint32(global_col_idx, nnz_counts, nnz_displs, MPI_UINT32_T, 
                 local_csr.col_idx, local_csr.local_nz_count, 0, MPI_COMM_WORLD);
    
    uint32_t* rowptr_scatter_counts = (uint32_t*)malloc(size * sizeof(uint32_t));
    uint32_t* rowptr_scatter_displs = (uint32_t*)malloc(size * sizeof(uint32_t));
    
    for (int i = 0; i < size; i++) {
        rowptr_scatter_counts[i] = row_counts[i] + 1;
        rowptr_scatter_displs[i] = row_displs[i];
    }
    
    scatterv_uint32(global_row_ptr, rowptr_scatter_counts, rowptr_scatter_displs, MPI_UINT32_T,
                 local_csr.row_ptr, local_csr.local_rows + 1, 0, MPI_COMM_WORLD);
    
    //Here we need to re-normalize row pointers
    uint32_t start_offset = local_csr.row_ptr[0]; 
    for (uint32_t i = 0; i <= local_csr.local_rows; i++) {
        local_csr.row_ptr[i] -= start_offset;
    }
    
    //Print critical data to check that distribution went as expected for each rank
    printf("Rank %d: Local rows: %u, NNZ: %u. Ready.\n", rank, local_csr.local_rows, local_csr.local_nz_count);

    // "Cleaning" operations
    if (rank == 0) {
        free(global_row_ptr);
        free(global_col_idx);
        free(nnz_counts);
        free(nnz_displs);
    }
    
    
    free(rowptr_scatter_counts);
    free(rowptr_scatter_displs);
    free(row_counts);
    free(row_displs);


    return local_csr;
}
