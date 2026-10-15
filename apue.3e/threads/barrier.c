#include <stdio.h>   /* for printf */
#include <stdlib.h>  /* for exit, qsort, srandom, random */
#include <pthread.h> /* for pthread_create, pthread_barrier_init, pthread_barrier_wait, pthread_t, pthread_barrier_t */
#include <limits.h>  /* for LONG_MAX */
#include <sys/time.h> /* for gettimeofday, struct timeval */

#include "apue.h"

#define NTHR 8               /* number of threads */
#define NUMNUM 8000000L      /* number of numbers to sort */
#define TNUM (NUMNUM / NTHR) /* number to sort per thread */

long nums[NUMNUM];
long snums[NUMNUM];

pthread_barrier_t b;

#define heapsort qsort

/*
 * Compare two long integers (helper function for heapsort)
 */
int complong(const void *arg1, const void *arg2) {
    const long l1 = *(const long *)arg1;
    const long l2 = *(const long *)arg2;

    if (l1 == l2) {
        return 0;
    } else if (l1 < l2) {
        return -1;
    } else {
        return 1;
    }
}

/*
 * Worker thread to sort a portion of the set of numbers.
 */
void *thr_fn(void *arg) {
    const long idx = (long)arg;

    heapsort(&nums[idx], TNUM, sizeof(long), complong);
    pthread_barrier_wait(&b);

    /*
     * Go off and perform more work ...
     */
    return ((void *)0);
}

/*
 * Merge the results of the individual sorted ranges.
 */
void merge() {
    long idx[NTHR];
    for (long i = 0; i < NTHR; i++) {
        idx[i] = i * TNUM;
    }
    for (long sidx = 0; sidx < NUMNUM; sidx++) {
        long num = LONG_MAX;
        long minidx = 0;
        for (long i = 0; i < NTHR; i++) {
            if ((idx[i] < (i + 1) * TNUM) && (nums[idx[i]] < num)) {
                num = nums[idx[i]];
                minidx = i;
            }
        }
        snums[sidx] = nums[idx[minidx]];
        idx[minidx]++;
    }
}

int main(void) {
    struct timeval start;
    struct timeval end;

    /*
     * Create the initial set of numbers to sort.
     */
    srandom(1);
    for (unsigned long i = 0; i < NUMNUM; i++) {
        nums[i] = random();
    }

    /*
     * Create 8 threads to sort the numbers.
     */
    gettimeofday(&start, nullptr);
    pthread_barrier_init(&b, nullptr, NTHR + 1);
    for (unsigned long i = 0; i < NTHR; i++) {
        pthread_t tid;
        int err = pthread_create(&tid, nullptr, thr_fn, (void *)(i * TNUM));
        if (err != 0) {
            err_exit(err, "can't create thread");
        }
    }
    pthread_barrier_wait(&b);
    merge();
    gettimeofday(&end, nullptr);

    /*
     * Print the sorted list.
     */
    const long long startusec = start.tv_sec * 1000000 + start.tv_usec;
    const long long endusec = end.tv_sec * 1000000 + end.tv_usec;
    const double elapsed = (double)(endusec - startusec) / 1000000.0;
    printf("sort took %.4f seconds\n", elapsed);
    for (unsigned long i = 0; i < NUMNUM; i++) {
        printf("%ld\n", snums[i]);
    }
    exit(0);
}
