#ifndef BFS_H
#define BFS_H



int coloring_cc(int n, const int *row_ptr, const int *col_idx, int *labels, int max_iters);
int count_components(int n, const int *labels);
void print_sample_labels(int n, const int *labels, int max_print);

#endif 
