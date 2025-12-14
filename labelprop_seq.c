#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdint.h>
#include <stddef.h>
#include "mat_to_csr.h"
#include "bfs.h"


int coloring_cc(uint32_t n, const uint32_t *row_ptr, const uint32_t *col_idx, uint32_t *labels, int max_iters) {
	if (!row_ptr || !col_idx || !labels) return -1;
	if (n <= 0) return 0;

	/* initialize each vertex label to its own id (1-based) per assignment
	 * requirement: labels range from 1..n instead of 0..n-1 */
	for (uint32_t v = 0; v < n; ++v) labels[v] = v + 1;

	int iterations = 0;
	int changed = 1;

	while (changed) {
		++iterations;
		changed = 0;

		/* For each vertex, try to adopt a smaller label seen among neighbors */
		for (uint32_t v = 0; v < n; v++) {
			uint32_t best = labels[v];


			for (uint32_t e = row_ptr[v]; e < row_ptr[v+1]; ++e) {
				uint32_t u = col_idx[e];
				/* skip invalid neighbor indices (defensive, in case .mat wasn't strictly adjacency) */
				if (u < 0 || u >= n) continue;
				uint32_t lu = labels[u];
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


int count_components(uint32_t n, const uint32_t *labels) {
	if (n <= 0 || !labels) return 0;

	/* labels are expected in the range 1..n (inclusive) */
	unsigned char *seen = calloc((size_t)(n + 1), 1); /* index 0 unused */
	if (!seen) return -1;

	int count = 0;
	for (uint32_t i = 0; i < n; ++i) {
		uint32_t lbl = labels[i];
		if (lbl < 1 || lbl > n) continue; /* skip invalid labels */
		if (!seen[lbl]) { seen[lbl] = 1; ++count; }
	}

	free(seen);
	return count;
}

/* Print up to `max_print` label pairs (vertex:label) for quick inspection */
void print_sample_labels(uint32_t n, const uint32_t *labels, uint32_t max_print) {
	uint32_t limit = (n < max_print) ? n : max_print;
	/* Print vertex indices 1-based to match label numbering */
	for (uint32_t i = 0; i < limit; ++i) printf("%u:%u ", i + 1, labels[i]);
	if (limit > 0) printf("\n");
}

