#ifndef PGXICOR_H
#define PGXICOR_H

#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "utils/guc.h"
#include "funcapi.h"

typedef struct xicor_problem
{
    int n;
    double* x;
    double* y;
} xicor_problem;

typedef struct xicor_parameter
{
    int seed;
    bool ties; /* bool */
} xicor_parameter;

typedef struct xicor_score
{
    double score;
} xicor_score;

void quicksort(double *a, int *idx, int l, int u);
int *argsort(double *a, int n);

xicor_score *xicor_compute_score(xicor_problem *prob, xicor_parameter *param);

#endif
