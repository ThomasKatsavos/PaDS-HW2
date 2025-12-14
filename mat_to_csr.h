#ifndef MAT_TO_CSR_H
#define MAT_TO_CSR_H
#include <stdint.h>

typedef struct{
   uint32_t nzcount;
   uint32_t nrows;

   uint32_t *row_ptr;
   uint32_t *col_idx;
}CSR_info;

void clean_csr_vectors(CSR_info vecs);
CSR_info mat_to_csr(char* file_name);

#endif
