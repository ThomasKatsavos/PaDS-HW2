#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "mat_to_csr.h"
#include "bfs.h"


int coloring_cc(int n, const int *row_ptr, const int *col_idx, int *labels, int max_iters) {
	if (!row_ptr || !col_idx || !labels) return -1;
	if (n <= 0) return 0;

	/* initialize each vertex label to its own id (1-based) per assignment
	 * requirement: labels range from 1..n instead of 0..n-1 */
	for (int v = 0; v < n; ++v) labels[v] = v + 1;

	int iterations = 0;
	int changed = 1;

	while (changed) {
		++iterations;
		changed = 0;

		/* For each vertex, try to adopt a smaller label seen among neighbors */
		for (int v = 0; v < n; v++) {
			int best = labels[v];


			for (int e = row_ptr[v]; e < row_ptr[v+1]; ++e) {
				int u = col_idx[e];
				/* skip invalid neighbor indices (defensive, in case .mat wasn't strictly adjacency) */
				if (u < 0 || u >= n) continue;
				int lu = labels[u];
				if (lu < best){best = lu;}
			}
			if (best < labels[v]) {
				labels[v] = best;
				changed = 1;
			}
		}

		if (max_iters > 0 && iterations >= max_iters) break;
	}

	return iterations;
}


int count_components(int n, const int *labels) {
	if (n <= 0 || !labels) return 0;

	/* labels are expected in the range 1..n (inclusive) */
	unsigned char *seen = calloc((size_t)(n + 1), 1); /* index 0 unused */
	if (!seen) return -1;

	int count = 0;
	for (int i = 0; i < n; ++i) {
		int lbl = labels[i];
		if (lbl < 1 || lbl > n) continue; /* skip invalid labels */
		if (!seen[lbl]) { seen[lbl] = 1; ++count; }
	}

	free(seen);
	return count;
}

/* Print up to `max_print` label pairs (vertex:label) for quick inspection */
void print_sample_labels(int n, const int *labels, int max_print) {
	int limit = (n < max_print) ? n : max_print;
	/* Print vertex indices 1-based to match label numbering */
	for (int i = 0; i < limit; ++i) printf("%d:%d ", i + 1, labels[i]);
	if (limit > 0) printf("\n");
}

