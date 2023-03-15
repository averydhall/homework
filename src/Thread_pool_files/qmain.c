
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "cqueue.h"


#define NUM_THREADS 40
#define NUM_ITERATIONS 10000

typedef struct thread_arg_st {
    cqueue_t* queue;
} thread_arg_t;

static int count = 0;

static void* enqueue_thread(void* arg) {
    thread_arg_t* the_arg = (thread_arg_t*) arg;
    rc_t* rc = malloc(sizeof(rc_t));

    if (rc == NULL)
        return NULL;
    
    *rc = Success;

    int k;
    for (k=0; k<NUM_ITERATIONS; k++) {
        *rc = cqueue_enqueue(the_arg->queue, &k, sizeof(int), NULL);
        if (*rc != Success) {
            fprintf(stderr, "There was an error enqueueing, error value was %d\n", *rc);
            return rc;
        }
    }

    return rc;
}

static void* dequeue_thread(void* arg) {
    thread_arg_t* the_arg = (thread_arg_t*) arg;
    rc_t* rc = malloc(sizeof(rc_t));

    if (rc == NULL)
        return NULL;
    
    *rc = Success;

    int* value;
    uint32_t size;

    int k;
    for (k=0; k<NUM_ITERATIONS; k++) {
        *rc = cqueue_dequeue(the_arg->queue, sizeof(int), (void**)&value, &size, NULL);
        if (*rc != Success) {
            fprintf(stderr, "There was an error dequeueing, error value was %d\n", *rc);
            return rc;
        }

        free(value);   
    }

    return rc;
}

int main(int argc, char* argv[]) {
    thread_arg_t the_arg;
    pthread_t threads[NUM_THREADS*2];
    rc_t rc;
    cqueue_t queue;
    cqueue_attr_t attrs;
    uint32_t size;
    int x = 7;

    rc = cqueue_attr_init(&attrs);
    if (rc != Success) {
        fprintf(stderr, "Error calling attr init.\n");
        return -1;
    }

    attrs.block_size = 8;
    attrs.num_blocks = 16;

    rc = cqueue_create(&queue, &attrs);
    if (rc != Success) {
        fprintf(stderr, "Error calling cqueue create.\n");
    }

    the_arg.queue = &queue;

    for (int k=0; k<NUM_THREADS; k++) {
        int prc = pthread_create(&threads[k], NULL, &enqueue_thread, &the_arg);
        if (prc != 0) {
            fprintf(stderr, "There was a problem during pthread creation for enqueue with error=%d\n", prc);
            return -1;
        }
    }

    for (int k=0; k<NUM_THREADS; k++) {
        int prc = pthread_create(&threads[NUM_THREADS+k], NULL, &dequeue_thread, &the_arg);
        if (prc != 0) {
            fprintf(stderr, "There was a problem during pthread creation for dequeue with error=%d\n", prc);
            return -1;
        }
    }

    for (int k=0; k<NUM_THREADS*2; k++) {
        int* trc;
        int prc = pthread_join(threads[k], (void**)&trc);

        if (prc != 0) {
            printf("There was a problem during pthread join with error=%d\n", prc);
            return -1;  
        }

        if (trc == NULL) {
            fprintf(stderr, "Got NULL back for thread return code.\n");
            return -1;
        }

        if (*trc != Success) {
            fprintf(stderr, "There was a problem with the thread return code=%d\n", *trc);
            return -1; 
        }

        free(trc);
    }

    rc = cqueue_size(&queue, &size);
    if (rc != Success) {
        fprintf(stderr, "Error calling cqueue size.\n");
        return -1;
    } 

    if (size != 0) {
        fprintf(stderr, "Error on getting size. Should be 0. Got %u\n", size);
        return -1;
    }   
    
    rc = cqueue_destroy(&queue);
    if (rc != Success) {
        fprintf(stderr, "Error calling cqueue destoy.\n");
        return -1;
    } 

    printf("Tests Completed Successfully\n");
    return 0;
}

