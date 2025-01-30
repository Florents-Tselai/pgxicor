#include "pgxicor.h"
#include "postgres.h"
#include "fmgr.h"
#include "utils/array.h"
#include "utils/guc.h"
#include "funcapi.h"

PG_MODULE_MAGIC;

static int guc_seed;
static bool guc_ties;

/* Startup */
void _PG_init(void)
{
    DefineCustomIntVariable("xicor.seed",
                            "Random seed for xicor computations",
                            NULL,
                            &guc_seed,
                            42, /* Default */
                            0, /* Min */
                            INT_MAX, /* Max */
                            PGC_SUSET,
                            0,
                            NULL,
                            NULL,
                            NULL);

    DefineCustomBoolVariable("xicor.ties",
                             "Enable tie handling in xicor computations",
                             NULL,
                             &guc_ties,
                             false, /* Default value */
                             PGC_SUSET,
                             0,
                             NULL,
                             NULL,
                             NULL);
}

void quicksort(double* a, int* idx, int l, int u)
{
    int i, m, idx_temp;
    double a_temp;

    if (l >= u)
        return;

    m = l;
    for (i = l + 1; i <= u; i++)
    {
        if (a[i] < a[l])
        {
            ++m;

            idx_temp = idx[m];
            idx[m] = idx[i];
            idx[i] = idx_temp;

            a_temp = a[m];
            a[m] = a[i];
            a[i] = a_temp;
        }
    }

    idx_temp = idx[l];
    idx[l] = idx[m];
    idx[m] = idx_temp;

    a_temp = a[l];
    a[l] = a[m];
    a[m] = a_temp;

    quicksort(a, idx, l, m - 1);
    quicksort(a, idx, m + 1, u);
}

int* argsort(double* a, int n)
{
    double* a_cpy;
    int i, *idx;

    a_cpy = (double*)malloc(n * sizeof(double));
    if (a_cpy == NULL)
        return NULL;

    idx = (int*)malloc(n * sizeof(int));
    if (idx == NULL)
    {
        free(a_cpy);
        return NULL;
    }

    /* fill a_cpy */
    memcpy(a_cpy, a, n * sizeof(double));

    /* fill idx */
    for (i = 0; i < n; i++)
        idx[i] = i;

    quicksort(a_cpy, idx, 0, n - 1);
    free(a_cpy);

    return idx;
}


static void build_xicor_problem(ArrayType* arg0,
                                bool null_arg0,
                                ArrayType* arg1,
                                bool null_arg1,
                                xicor_problem* ret_prob)
{
    ArrayIterator iter0, iter1;
    int i;
    Datum x_value, y_value;

    ret_prob->n = ArrayGetNItems(ARR_NDIM(arg0), ARR_DIMS(arg0));
    //TODO: check that both arrays are of the same size
    ret_prob->x = (double*)palloc(ret_prob->n * sizeof(double));
    ret_prob->y = (double*)palloc(ret_prob->n * sizeof(double));

    /* populate prob.x and prob.y arrays */
    iter0 = array_create_iterator(arg0, 0, NULL);
    i = 0;
    while (array_iterate(iter0, &x_value, &null_arg0))
    {
        ret_prob->x[i] = DatumGetFloat8(x_value);
        i++;
    }
    array_free_iterator(iter0);

    iter1 = array_create_iterator(arg1, 0, NULL);
    i = 0;
    while (array_iterate(iter1, &y_value, &null_arg1))
    {
        ret_prob->y[i] = DatumGetFloat8(y_value);
        i++;
    }
    array_free_iterator(iter1);
}


xicor_score* xicor_compute_score(xicor_problem* prob, xicor_parameter* param)
{
    if (!prob || !prob->x || !prob->y || prob->n <= 0)
    {
        fprintf(stderr, "Invalid input to xicor_compute_score\n");
        return NULL;
    }

    int n = prob->n;
    double* x = prob->x;
    double* y = prob->y;
    int ties = param->ties;

    /* Initialize the random seed */
    srand(param->seed);

    /* Get the sorted order of x using argsort */
    int* order = argsort(x, n);
    if (!order)
    {
        fprintf(stderr, "Memory allocation failed for order array\n");
        return NULL;
    }

    /* Compute the rank arrays l and r */
    int* l = (int*)malloc(n * sizeof(int));
    int* r = (int*)malloc(n * sizeof(int));
    if (!l || !r)
    {
        fprintf(stderr, "Memory allocation failed for rank arrays\n");
        free(order);
        return NULL;
    }

    for (int i = 0; i < n; i++)
    {
        l[i] = 0;
        for (int j = 0; j < n; j++)
        {
            if (y[order[j]] >= y[order[i]])
            {
                l[i]++;
            }
        }
        r[i] = l[i];
    }

    /* Handle ties if required */
    if (ties)
    {
        for (int i = 0; i < n; i++)
        {
            int tie_count = 0;
            for (int j = 0; j < n; j++)
            {
                if (r[j] == r[i]) tie_count++;
            }
            if (tie_count > 1)
            {
                int* tie_indices = (int*)malloc(tie_count * sizeof(int));
                if (!tie_indices)
                {
                    fprintf(stderr, "Memory allocation failed for tie indices\n");
                    free(order);
                    free(l);
                    free(r);
                    return NULL;
                }

                int k = 0;
                for (int j = 0; j < n; j++)
                {
                    if (r[j] == r[i])
                    {
                        tie_indices[k++] = j;
                    }
                }

                for (int k = 0; k < tie_count; k++)
                {
                    r[tie_indices[k]] = r[tie_indices[k]] - k;
                }

                free(tie_indices);
            }
        }
    }

    /* Compute the Xi correlation score */
    double sum_l = 0.0, sum_r_diff = 0.0;
    for (int i = 0; i < n; i++)
    {
        sum_l += l[i] * (n - l[i]);
    }
    for (int i = 1; i < n; i++)
    {
        sum_r_diff += abs(r[i] - r[i - 1]);
    }

    double score;
    if (ties)
    {
        score = 1.0 - (n * sum_r_diff) / (2.0 * sum_l);
    }
    else
    {
        score = 1.0 - (3.0 * sum_r_diff) / ((double)(n * n - 1));
    }

    /* Create the result structure */
    xicor_score* result = (xicor_score*)malloc(sizeof(xicor_score));
    if (!result)
    {
        fprintf(stderr, "Memory allocation failed for result\n");
        free(order);
        free(l);
        free(r);
        return NULL;
    }
    result->score = score;

    /* Free allocated memory */
    free(order);
    free(l);
    free(r);

    return result;
}


PG_FUNCTION_INFO_V1(xicor_final);

Datum
xicor_final(PG_FUNCTION_ARGS)
{
    HeapTupleHeader problem = PG_GETARG_HEAPTUPLEHEADER(0);
    bool n_isnull, x_isnull, y_isnull;

    Datum n = GetAttributeByName(problem, "n", &n_isnull);
    Datum x = GetAttributeByName(problem, "x", &x_isnull);
    Datum y = GetAttributeByName(problem, "y", &y_isnull);

    xicor_problem prob;
    xicor_parameter param = {.seed = guc_seed, .ties = guc_ties};
    build_xicor_problem(DatumGetArrayTypeP(x), x_isnull, DatumGetArrayTypeP(y), y_isnull, &prob);

    xicor_score* score = xicor_compute_score(&prob, &param);
    float8 result = score->score;
    free(score);

    PG_RETURN_FLOAT8(result);
}
