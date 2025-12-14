#ifndef BFS_H
#define BFS_H
#include <stdint.h>


int coloring_cc(uint32_t n, const uint32_t *row_ptr, const uint32_t *col_idx, uint32_t *labels, int max_iters);
int count_components(uint32_t n, const uint32_t *labels);
void print_sample_labels(uint32_t n, const uint32_t *labels, uint32_t  max_print);

#endif 
