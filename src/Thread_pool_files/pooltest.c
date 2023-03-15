#include "pool.h"
#include "rc.h"
#include <stdio.h>
#include <stdlib.h>

#define NUM_THREADS 10

/* These are the coefficients of the cross product
   using Sarrus's rule. See wikipedia's page on
   cross-product calculation. From wikipedia:
   The space E together with the cross product
   is an algebra over the real numbers, which is
   neither commutative nor associative, but is a
   Lie algebra with the cross product being the
   Lie bracket.*/

static int cp_terms[3][2] = {{1, 2}, {2, 0}, {0, 1}};

rc_t myfun(void* arg, void** result) {
   *result = NULL;
   printf("Hello World\n");
   return Success;
}

typedef struct vectors_st {
   float* vector_a;
   float* vector_b;
   int* cp_term;
} vectors_t;

typedef struct vec_result_st {
   float cp_coeff;
   float dp_term;
} vec_result_t;

rc_t vector_math(vectors_t* arg, vec_result_t** result) {
   // cross product coefficient
   *result = malloc(sizeof(vec_result_t));
   if (*result == NULL)
      return OutOfMemory;

   (*result)->cp_coeff = arg->vector_a[arg->cp_term[0]] * arg->vector_b[arg->cp_term[1]]
                        - arg->vector_a[arg->cp_term[1]] * arg->vector_b[arg->cp_term[0]];

   int dotp_idx = arg->cp_term[0];

   (*result)->dp_term = arg->vector_a[dotp_idx] * arg->vector_b[dotp_idx];

   return Success;
}

int main(int argc, char* argv[]) {
   rc_t rc;
   thread_pool_t pool;

   rc = pool_create(&pool, NUM_THREADS);
   if (rc != Success) {
      fprintf(stderr, "The pool was not created successfully.\n");
      return rc;
   }

   void* args[NUM_THREADS*2];
   void* results[NUM_THREADS*2];

   rc = pool_map(&pool, myfun, NUM_THREADS*2, args, results);
   if (rc != Success) {
      fprintf(stderr, "The pool map was not successful. Error code=%d\n", rc);
      return rc;
   }

   rc = pool_destroy(&pool);
   if (rc != Success) {
      fprintf(stderr, "The pool was not destroyed successfully.\n");
      return rc;
   }

   printf("Test 1 completed.\n");
   fflush(stdout);

   void* args2[100];
   void* results2[100];

   rc = pool_create(&pool, 1);
   if (rc != Success) {
      fprintf(stderr, "The pool was not created successfully.\n");
      return rc;
   }

   rc = pool_map(&pool, myfun, 100, args2, results2);
   if (rc != Success) {
      fprintf(stderr, "The pool map was not successful. Error code=%d\n", rc);
      return rc;
   }

   printf("Test 2 completed.\n");

   float x[] = {1.0, 2.0, 3.0};
   float y[] = {2.5, 4.5, 6.0};

   vectors_t* vargs[3];
   for (int k=0;k<3;k++) {
      vectors_t* varg = malloc(sizeof(vectors_t));
      varg->vector_a = x;
      varg->vector_b = y;
      varg->cp_term = cp_terms[k];
      vargs[k] = varg;
   }

   vec_result_t* vres[3];

   rc = pool_map(&pool, (pool_fun_t*)vector_math, 3, (void*)vargs, (void**)vres);

   if (rc != Success) {
      fprintf(stderr, "The pool map on vectors was not successful. Error code=%d\n", rc);
      return rc;
   }

   printf("The axb coefficients of the cross product are:\n");
   printf("%fi + %fj + %fk\n", vres[0]->cp_coeff, vres[1]->cp_coeff, vres[2]->cp_coeff);

   float dotp = 0;
   for (int k=0; k<3; k++)
      dotp+=vres[k]->dp_term;

   printf("The dot product of the two vectors is %f\n", dotp);

   for (int k=0; k<3; k++) {
      free(vargs[k]);
      free(vres[k]);
   }

   rc = pool_destroy(&pool);
   if (rc != Success) {
      fprintf(stderr, "The pool was not destroyed successfully.\n");
      return rc;
   }

   printf("Test 3 completed.\n");

   return Success;
}