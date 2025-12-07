#include <stdio.h>
#include <stdlib.h>
#include <matio.h>
#include <stdint.h>
#include <inttypes.h>
#include "mat_to_csr.h"


void clean_csr_vectors(CSR_info vecs){
    free(vecs.row_ptr);
    free(vecs.col_idx);
}


CSR_info mat_to_csr(char* file_name) {

    // Open .mat file
    mat_t *matfp = Mat_Open(file_name, MAT_ACC_RDONLY);
    if (!matfp) {
        fprintf(stderr, "MATIO->ERROR: cannot open %s file \n", file_name);
        return (CSR_info){1,1,NULL,NULL};
    }
    
    
    // Read the sparse matrix  with variable name "A" inside struct "Problem" 
    matvar_t *projectvar = Mat_VarRead(matfp, "Problem");
    if (!projectvar) {
        fprintf(stderr, "MATIO->ERROR:'Problem' struct missing in %s\n", file_name);
        Mat_Close(matfp);
        return (CSR_info){1,1, NULL,NULL};
    }

    matvar_t *matvar = Mat_VarGetStructFieldByName(projectvar, "A", 0);
     if (!matvar) {
        fprintf(stderr, "MATIO->ERROR: 'A' matrix missing in struct 'Problem' inside %s\n", file_name);
	      Mat_VarFree(projectvar);
        Mat_Close(matfp);
        return (CSR_info){1,1, NULL,NULL};
    }

    if (matvar->class_type != MAT_C_SPARSE) {
        fprintf(stderr, "MATIO->ERROR: matrix read in %s is not sparse\n", file_name);
	      Mat_VarFree(projectvar);
        Mat_Close(matfp);
        return (CSR_info){1,1,NULL,NULL};
    }

    puts("Converting matrix to CSR format...");
    mat_sparse_t *sparse = (mat_sparse_t* )matvar->data;
    int numrows = matvar->dims[0];   
    int ncols = matvar->dims[1];   
    int *row_ind =(int* ) sparse->ir;  
    int *col_ptr =(int* ) sparse->jc;  

    //Save row pointer and column index data in CSR_info object
    CSR_info csr;

    csr.row_ptr = (int*)calloc(numrows+1, sizeof(int));
    
    // Count nonzeros per row
    for (int c = 0; c<ncols; c++) {
        for (int i = col_ptr[c]; i < col_ptr[c+1]; i++) {
            int r = row_ind[i];
            csr.row_ptr[r+1]++; 
        }
    }

    // Define row_ptr
    for (int i = 0; i<numrows; i++) {
        csr.row_ptr[i+1] += csr.row_ptr[i];
    }

    //Define non-zeros and rows count
    csr.nzcount = csr.row_ptr[numrows];
    csr.nrows = numrows;

    printf("Number of nonzeros: %d\n", csr.nzcount);
  
    //Define col_idx
    csr.col_idx = (int* )malloc(csr.nzcount * sizeof(int));
    
    int *cnt = (int*)malloc(numrows * sizeof(int));
    for (int i = 0; i < numrows; i++){cnt[i] = csr.row_ptr[i];}

    for(int i=0; i<ncols; i++){
        for(int j=col_ptr[i]; j<col_ptr[i+1]; j++){
            int r = row_ind[j];
	          int dest = cnt[r]++;   
            csr.col_idx[dest] = i; 
	}
    }


    // Cleanup dynamically allocated used vectors
    Mat_VarFree(projectvar);
    Mat_Close(matfp);

    
    printf("Your .mat to CSR format in .bin files conversion complete.\n");
    

    return csr;
}

