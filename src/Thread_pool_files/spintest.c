#include "spinlock.h"
#include <stdlib.h>
#include <pthread.h>
#include <stdio.h>

#define NUM_THREADS 20
#define NUM_ITERATIONS 100

static int count = 0;

typedef struct thread_arg_st {
    spinlock_t lockh;
} thread_arg_t;

static void* test_thread(void* arg) {
    thread_arg_t* the_arg = (thread_arg_t*) arg;
    rc_t* rc = malloc(sizeof(rc_t));

    *rc = Success;

    if (rc == NULL)
        return NULL;

    int k;
    for (k=0; k<NUM_ITERATIONS; k++) {
        *rc = spinlock_acquire(&the_arg->lockh);
        if (*rc != Success)
            return rc;
        count += 1;
        *rc = spinlock_release(&the_arg->lockh);
        if (*rc != Success)
            return rc;
    }

    return rc;
}

int main(int argc, char* argv[]) {

    thread_arg_t the_arg;
    rc_t rc;
    pthread_t threads[NUM_THREADS];

    rc = spinlock_create(&the_arg.lockh, NULL);
    if (rc != Success) {
        printf("lock create failed with %d\n", rc);
        return rc;
    }

    for (int k=0; k<NUM_THREADS; k++) {
        int prc = pthread_create(&threads[k], NULL, &test_thread, &the_arg);
        if (prc != 0) {
            printf("There was a problem during pthread creation with error=%d\n", prc);
            return -1;
        }
    }

    for (int k=0; k<NUM_THREADS; k++) {
        int* trc;
        int prc = pthread_join(threads[k], (void**)&trc);

        if (prc != 0) {
            printf("There was a problem during pthread join with error=%d\n", prc);
            return -1;  
        }

        if (trc == NULL) {
            printf("Got NULL back for thread return code.\n");
            return -1;
        }

        if (*trc != Success) {
            printf("There was a problem with the thread return code=%d\n", *trc);
            return -1; 
        }

        free(trc);
    }

    int expected_value = NUM_THREADS * NUM_ITERATIONS;

    if (count != expected_value) {
        printf("The test failed. Expected value of count was %d and actual value was %d\n", expected_value, count);
    } else {
        printf("The test passed\n");
    }

    return 0;
}