#include <stdio.h>
#include <stdlib.h>

#define CACHE_SIZE    16384
#define LINE_SIZE     64
#define TOTAL_LINES   (CACHE_SIZE / LINE_SIZE)   /* 256 */
#define INT_PER_LINE  (LINE_SIZE / sizeof(int))

#define CONFLICT_STRIDE  (CACHE_SIZE / (int)sizeof(int))
#define CONFLICT_LINES   5
#define LRU_LINES        (TOTAL_LINES + 1)       /* 257 for full stress */

volatile int sink = 0;

int main(void)
{
    int i, r, total = 0;

    int *conflict_array = malloc(CONFLICT_LINES * CONFLICT_STRIDE * sizeof(int));
    int *lru_array      = malloc(LRU_LINES * INT_PER_LINE * sizeof(int));
    int *scan_array     = malloc((CACHE_SIZE * 2 / sizeof(int)) * sizeof(int));

    if (!conflict_array || !lru_array || !scan_array) {
        fprintf(stderr, "malloc failed\n");
        return 1;
    }

    /* ================== STRONGER STRESS LOOPS ================== */

    /* Phase 1: Conflict misses */
    for (r = 0; r < 1000; r++) {                    // increased
        for (i = 0; i < CONFLICT_LINES; i++)
            total += conflict_array[i * CONFLICT_STRIDE];
        conflict_array[0] = total;
    }

    /* Phase 2: Replacement policy sensitivity */
    for (r = 0; r < 2000; r++) {                    // increased
        for (i = 0; i < CONFLICT_LINES; i++)
            total += conflict_array[i * CONFLICT_STRIDE];
        total += conflict_array[1 * CONFLICT_STRIDE];
        total += conflict_array[2 * CONFLICT_STRIDE];
    }

    /* Phase 5: MAIN FULLY-ASSOCIATIVE STRESS (257 lines) */
    for (r = 0; r < 800; r++) {                     // <<< This is the most important loop
        for (i = 0; i < TOTAL_LINES; i++)
            total += lru_array[i * INT_PER_LINE];

        total += lru_array[TOTAL_LINES * INT_PER_LINE];     /* force eviction */

        for (i = TOTAL_LINES - 1; i >= 0; i--)
            total += lru_array[i * INT_PER_LINE];

        total += lru_array[0];
        total += lru_array[TOTAL_LINES * INT_PER_LINE];
    }

    /* Phase 6: Working set sweep */
    for (int ws = 16; ws <= LRU_LINES; ws += 8) {   // finer steps
        for (r = 0; r < 20; r++) {                  // increased
            for (i = 0; i < ws; i++)
                total += lru_array[i * INT_PER_LINE];
        }
    }

    sink = total;

    free(conflict_array);
    free(lru_array);
    free(scan_array);
    return 0;
}
