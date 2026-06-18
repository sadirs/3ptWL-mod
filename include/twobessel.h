// ============================================================================
//        1          2          3          4        ^ 5          6          7

#include <stddef.h>

typedef struct config {
	int sys_Flag;
	double l1, l2;
	double nu1, nu2;
	double c_window_width;
	long Nk_sample;
} config;

int two_Bessel_binave(double *k1, double *k2, double **fk1k2,
                      long N1, long N2, config *config,
                      double smooth_dlnr, int dimension,
                      double *r1, double *r2, double **result,
                      char *errmsg, size_t errmsg_size);
