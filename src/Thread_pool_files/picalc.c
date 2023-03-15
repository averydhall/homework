#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "pool.h"
#include "rc.h"

#define NUM_POINTS 10000000
#define NUM_THREADS 10

//f(x) = sqrt(1 – x^2)

bool isUnderCurve(double x, double y) {
    double curveY = sqrt(1 - pow(x, 2));
    return (y < curveY);
}

rc_t generatePointAndCheck(void* arg, void** result) {
    double xRand;
    double yRand;
    bool* under = malloc(sizeof(bool));

    if (under == NULL) {
        return OutOfMemory;
    }

    xRand = ((double)rand() / RAND_MAX);
    yRand = ((double)rand() / RAND_MAX);
    *under = isUnderCurve(xRand, yRand);
    *result = under;

    return Success;
}

int main(int argc, char* argv[]) {

    int totalUnderCurve = 0;
    double piEstimate;
    rc_t rc;
    pool_t pool;
    void* args[NUM_POINTS];
    void* results[NUM_POINTS];

    rc = pool_create(&pool, NUM_THREADS);
    if (rc != Success) {
        fprintf(stderr, "The pool was not created successfully.\n");
        return 0;
    }
    printf("%s", "Pool was created successfully.");

    rc = pool_map(&pool, generatePointAndCheck, NUM_POINTS, args, results);

    rc = pool_destroy(&pool);
    if (rc != Success) {
        fprintf(stderr, "The pool was not destroyed successfully.\n");
        return 0;
    }
    //printf("%s%d\n", "Total under curve: ", totalUnderCurve);

    piEstimate = (double) (4 * ((double)totalUnderCurve / NUM_POINTS));
    printf("%s%f\n", "Pi estimate: ", piEstimate);
    
}
