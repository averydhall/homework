#include "rc.h"
#include "cqueue.h"
#include <pthread.h>

typedef rc_t pool_fun_t(void* arg, void** result);

typedef struct pool_st {
   cqueue_t work_queue;
   cqueue_t result_queue;
   int size;
   pthread_t* threads;
} pool_t;


typedef struct pool_work_st {
   int id;
   void *arg;
   pool_fun_t *fun;
} pool_work_t;

typedef struct pool_result_st {
   int id;
   int rc;
   void* result;
} pool_result_t;


rc_t pool_create(pool_t* pool, int pool_size);
rc_t pool_destroy(pool_t* pool);
rc_t pool_map(pool_t* pool, pool_fun_t fun, int arg_count, void* args[], void* results[]);

rc_t pool_work_create(pool_work_t* pool_work, int id, pool_t* pool, pool_fun_t* fun, void* arg);
rc_t pool_result_create(pool_result_t* pool_result, int id, rc_t rc, void* result);
