#include <pthread.h>
#include "rc.h"
#include "cqueue.h"
#include "pool.h"
#include <stdio.h>

//return type of thread start function?
static void* thread_start(pool_t* pool) {

    rc_t rc;
    pool_work_t* dequeued_work;
    pool_result_t* pool_result;
    void* result;

    rc = cqueue_dequeue(&pool->work_queue, sizeof(pool_work_t), (void**)&dequeued_work, sizeof(pool_work_t), NULL);
    if (rc != Success) {
        fprintf(stderr, "thread_start: an error occurred during call to cqueue_dequeue");
        return 0;
    } 

    while (dequeued_work->fun != NULL) {

        //call function with arg
        rc = dequeued_work->fun(dequeued_work->arg, &result);

        //create work result
        rc = pool_result_create(pool_result, dequeued_work->id, rc, result);
        if (rc != Success) {
            fprintf(stderr, "thread_start: an error occurred during call to pool_result_create");
            return 0;
        } 
        
        //enqueue completed result to queue
        rc = cqueue_enqueue(&pool->result_queue, result, sizeof(pool_result_t), NULL);
        if (rc != Success) {
            fprintf(stderr, "thread_start: an error occurred during call to cqueue_enqueue");
            return 0;
        } 

        //dequeue next item from work queue
        rc = cqueue_dequeue(&pool->work_queue, NULL, (void**)&dequeued_work, sizeof(pool_work_t), NULL);
        if (rc != Success) {
            fprintf(stderr, "thread_start: an error occurred during call to cqueue_dequeue");
            return 0;
        } 

    }

    return 0;
}

rc_t pool_work_create(pool_work_t* pool_work, int id, pool_t* pool, pool_fun_t* fun, void* arg) {
    pool_work->id = id;
    pool_work->fun = fun;
    pool_work->arg = arg;
    return Success;
}

rc_t pool_result_create(pool_result_t* pool_result, int id, rc_t rc, void* result) {
    pool_result->id = id;
    pool_result->rc = rc;
    pool_result->result = result;
    return Success;
}


rc_t pool_create(pool_t* pool, int pool_size){

    pthread_t currThread;
    pthread_t* threads[pool_size];
    cqueue_t work_queue;
    cqueue_t result_queue;
    pool->result_queue = result_queue;
    pool->work_queue = work_queue;
    pool->threads = threads;
    rc_t rc;

    rc = cqueue_create(&pool->work_queue, NULL);
    if (rc != Success) {
        fprintf(stderr, "pool_create: an error occurred during call to cqueue_create");
        return rc;
    } 
    
    rc = cqueue_create(&pool->result_queue, NULL);
    if (rc != Success) {
        fprintf(stderr, "pool_create: an error occurred during call to cqueue_create");
        return rc;
    } 

    for (int i=0; i<pool_size; i++) {
        //Create thread
        rc = pthread_create(&currThread, NULL, thread_start, pool);
        if (rc != Success) {
            fprintf(stderr, "pool_create: an error occurred during call to pthread_create");
            return rc;
        } 
        //Add to threads
        pool->threads[i] = currThread;

    }

    return Success;

}

rc_t pool_destroy(pool_t* pool) {

    rc_t rc;

    for (int i=0; i<pool->size; i++) {
        //Should retval be NULL?
        rc = pthread_join(pool->threads[i], NULL);
        if (rc != Success) {
            fprintf(stderr, "pool_destroy: an error occurred during call to pthread_join");
            return rc;
        } 
    }

    rc = cqueue_destroy(&pool->work_queue);
    if (rc != Success) {
        fprintf(stderr, "pool_destroy: an error occurred during call to cqueue_destroy");
        return rc;
    } 

    rc = cqueue_destroy(&pool->result_queue);
    if (rc != Success) {
        fprintf(stderr, "pool_destroy: an error occurred during call to cqueue_destroy");
        return rc;
    } 
    
    return Success;
}

rc_t pool_map(pool_t* pool, pool_fun_t fun, int arg_count, void* args[], void* results[]) {

    rc_t rc;
    pool_work_t work_item;
    pool_result_t work_result;
    void* result;

    for (int i=0; i<arg_count; i++) {
        //find correct way of arg indexing (does args + i work?)
        rc = pool_work_create(&work_item, i, &pool, fun, args[i]);
        if (rc != Success) {
            fprintf(stderr, "pool_map: an error occurred during call to pool_create");
            return rc;
        }
        
        rc = cqueue_enqueue(&pool->work_queue, &work_item, sizeof(pool_work_t), NULL);
        if (rc != Success) {
            fprintf(stderr, "pool_map: an error occurred during call to cqueue_enqueue");
            return rc;
        }
    }

    for (int i=0; i<arg_count; i++) {

        rc = cqueue_dequeue(&pool->result_queue, sizeof(pool_result_t), &work_result, sizeof(pool_result_t), NULL);
        if (rc != Success) {
            fprintf(stderr, "pool_map: an error occurred during call to cqueue_dequeue");
            return rc;
        }

        //Check return code
        if (work_result.rc != Success) {
            fprintf(stderr, "pool_map: pool result's return code was invalid");
            return rc;
        }

        //Add to result array
        results[work_result.id] = work_result.result;
    } 

    //signal threads to exit by enqueueing a dummy work item with NULL fun ptr for each thread
    for (int i=0; i<pool->size; i++) {
        //find correct way of arg indexing (does args + i work?)
        rc = pool_work_create(&work_item, i, &pool, NULL, args);
        if (rc != Success) {
            fprintf(stderr, "pool_map: an error occurred during call to pool_create");
            return rc;
        }
        
        rc = cqueue_enqueue(&pool->work_queue, &work_item, sizeof(pool_work_t), NULL);
        if (rc != Success) {
            fprintf(stderr, "pool_map: an error occurred during call to cqueue_enqueue");
            return rc;
        }
    }

    return Success;
}