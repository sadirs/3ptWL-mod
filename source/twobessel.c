// ============================================================================
//        1          2          3          4        ^ 5          6          7

#include "globaldefs.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <fftw3.h>

#include "utils.h"
#include "twobessel.h"

//B keep these two unused routines only for reference
//      they were originally in the header file twobessel.h
static int two_sph_bessel(double *k1, double *k2, double **fk1k2, long N1,
                   long N2, config *config, double *r1, double *r2,
                   double **result);
static int two_sph_bessel_binave(double *k1, double *k2, double **fk1k2,
                          long N1, long N2, config *config,
                          double smooth_dlnr, int dimension, double *r1,
                          double *r2, double **result);
//E

static int two_sph_bessel(double *k1, double *k2, double **fk1k2, long N1,
                    long N2, config *config, double *r1, double *r2,
                   double **result) {
    if (N1 < 2 || N2 < 2 || N1 % 2 || N2 % 2)
        return FAILURE;
    
    long halfN1 = N1/2, halfN2 = N2/2;
    
    double k10, k20, r10,r20;
    k10 = k1[0];
    k20 = k2[0];
    
    double dlnk1, dlnk2;
    dlnk1 = log(k1[1]/k10);
    dlnk2 = log(k2[1]/k20);
    
    // Only calculate the m>=0 part
    double eta_m[halfN1+1], eta_n[halfN2+1];
    long i,j;
    for(i=0; i<=halfN1; i++) {eta_m[i] = 2*M_PI / dlnk1 / N1 * i;}
    for(j=0; j<=halfN2; j++) {eta_n[j] = 2*M_PI / dlnk2 / N2 * j;}
    
    double complex g1[halfN1+1], g2[halfN2+1];
    g_l(config->l1, config->nu1, eta_m, g1, halfN1+1);
    g_l(config->l2, config->nu2, eta_n, g2, halfN2+1);
    
    // calculate r1,r2 arrays
    for(i=0; i<N1; i++) {r1[i] = 1. / k1[N1-1-i];}
    for(i=0; i<N2; i++) {r2[i] = 1. / k2[N2-1-i];}
    r10 = r1[0];
    r20 = r2[0];
    
    // biased input func
    double *Pb;
    Pb = malloc(N1 * N2* sizeof(double));
    if (Pb == NULL)
        return FAILURE;
    
    for(i=0; i<N1; i++) {
        for(j=0; j<N2; j++) {
            Pb[i*N2+j] = fk1k2[i][j] / pow(k1[i], config->nu1)
            / pow(k2[j], config->nu2) ;
        }
    }
    
    fftw_complex *out;
    fftw_plan plan_forward, plan_backward;
    out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N1*(halfN2+1) );
    if (out == NULL) {
        free(Pb);
        return FAILURE;
    }
    
    plan_forward = fftw_plan_dft_r2c_2d(N1, N2, Pb, out, FFTW_ESTIMATE);
    if (plan_forward == NULL) {
        fftw_free(out);
        free(Pb);
        return FAILURE;
    }
    
    fftw_execute(plan_forward);
    fftw_destroy_plan(plan_forward);
    free(Pb);
    
    c_window_2d(out, config->c_window_width, halfN1, halfN2);
    
    long ij;
    for(i=0; i<=halfN1; i++) {
        for(j=0; j<=halfN2; j++) {
            ij = i*(halfN2+1) + j;
            out[ij] *= cpow(k20*r20, -I*eta_n[j]) * g2[j] * cpow(k10*r10, -I*eta_m[i]) * g1[i] ;
            out[ij] = conj(out[ij]);
        }
    }
    for(i=halfN1+1; i<N1; i++) {
        for(j=0; j<=halfN2; j++) {
            ij = i*(halfN2+1) + j;
            out[ij] *= cpow(k20*r20, -I*eta_n[j]) * g2[j] * cpow(k10*r10, I*eta_m[N1-i]) * conj(g1[N1-i]) ;
            out[ij] = conj(out[ij]);
        }
    }
    double *out_ifft;
    out_ifft = malloc(sizeof(double) * N1 * N2);
    if (out_ifft == NULL) {
        fftw_free(out);
        return FAILURE;
    }

    plan_backward = fftw_plan_dft_c2r_2d(N1, N2, out, out_ifft, FFTW_ESTIMATE);
    if (plan_backward == NULL) {
        free(out_ifft);
        fftw_free(out);
        return FAILURE;
    }
    
    fftw_execute(plan_backward);
    fftw_destroy_plan(plan_backward);
    fftw_free(out);
    
    for(i=0; i<N1; i++) {
        for(j=0; j<N2; j++) {
            result[i][j] = out_ifft[i*N2 + j] * M_PI / (16.*N1*N2 * pow(r2[j], config->nu2) * pow(r1[i], config->nu1));
        }
    }
    
    free(out_ifft);
    return SUCCESS;
}

static int two_sph_bessel_binave(double *k1, double *k2, double **fk1k2,
                           long N1, long N2, config *config,
                           double smooth_dlnr, int dimension, double *r1,
                           double *r2, double **result) {
    if (N1 < 2 || N2 < 2 || N1 % 2 || N2 % 2)
        return FAILURE;

	long halfN1 = N1/2, halfN2 = N2/2;

	double k10, k20, r10,r20;
	k10 = k1[0];
	k20 = k2[0];

	double dlnk1, dlnk2;
	dlnk1 = log(k1[1]/k10);
	dlnk2 = log(k2[1]/k20);

	// Only calculate the m>=0 part
	double eta_m[halfN1+1], eta_n[halfN2+1];
	long i,j;
	for(i=0; i<=halfN1; i++) {eta_m[i] = 2*M_PI / dlnk1 / N1 * i;}
	for(j=0; j<=halfN2; j++) {eta_n[j] = 2*M_PI / dlnk2 / N2 * j;}

	double complex g1[halfN1+1], g2[halfN2+1];
	g_l_smooth(config->l1, config->nu1, eta_m, g1, halfN1+1, smooth_dlnr, dimension);
	g_l_smooth(config->l2, config->nu2, eta_n, g2, halfN2+1, smooth_dlnr, dimension);

	double s_d_lambda = (exp(dimension*smooth_dlnr) - 1.) / dimension;

	// calculate r1,r2 arrays
	for(i=0; i<N1; i++) {r1[i] = 1. / k1[N1-1-i];}
	for(i=0; i<N2; i++) {r2[i] = 1. / k2[N2-1-i];}
	r10 = r1[0];
	r20 = r2[0];

	// biased input func
	double *Pb;
	Pb = malloc(N1 * N2* sizeof(double));
    if (Pb == NULL)
        return FAILURE;
    
	for(i=0; i<N1; i++) {
		for(j=0; j<N2; j++) {
			Pb[i*N2+j] = fk1k2[i][j] / pow(k1[i], config->nu1) / pow(k2[j], config->nu2) ;
		}
	}

	fftw_complex *out;
	fftw_plan plan_forward, plan_backward;
	out = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N1*(halfN2+1) );
    if (out == NULL) {
        free(Pb);
        return FAILURE;
    }
    
	plan_forward = fftw_plan_dft_r2c_2d(N1, N2, Pb, out, FFTW_ESTIMATE);
    if (plan_forward == NULL) {
        fftw_free(out);
        free(Pb);
        return FAILURE;
    }
    
	fftw_execute(plan_forward);
	fftw_destroy_plan(plan_forward);
	free(Pb);

	c_window_2d(out, config->c_window_width, halfN1, halfN2);

	long ij;
	for(i=0; i<=halfN1; i++) {
		for(j=0; j<=halfN2; j++) {
			ij = i*(halfN2+1) + j;
			out[ij] *= cpow(k20*r20, -I*eta_n[j]) * g2[j] * cpow(k10*r10, -I*eta_m[i]) * g1[i] ;
			out[ij] = conj(out[ij]);
		}
	}
	for(i=halfN1+1; i<N1; i++) {
		for(j=0; j<=halfN2; j++) {
			ij = i*(halfN2+1) + j;
			out[ij] *= cpow(k20*r20, -I*eta_n[j]) * g2[j] * cpow(k10*r10, I*eta_m[N1-i]) * conj(g1[N1-i]) ;
			out[ij] = conj(out[ij]);
		}
	}
	double *out_ifft;
    out_ifft = malloc(sizeof(double) * N1 * N2);
    if (out_ifft == NULL) {
        fftw_free(out);
        return FAILURE;
    }

    plan_backward = fftw_plan_dft_c2r_2d(N1, N2, out, out_ifft, FFTW_ESTIMATE);
    if (plan_backward == NULL) {
        free(out_ifft);
        fftw_free(out);
        return FAILURE;
    }

	fftw_execute(plan_backward);
	fftw_destroy_plan(plan_backward);
	fftw_free(out);

	for(i=0; i<N1; i++) {
		for(j=0; j<N2; j++) {
			result[i][j] = out_ifft[i*N2 + j] * M_PI / (16.*N1*N2 * pow(r2[j], config->nu2) * pow(r1[i], config->nu1)) / s_d_lambda/s_d_lambda;
		}
	}

    free(out_ifft);
    return SUCCESS;
}

int two_Bessel_binave(double *k1, double *k2, double **fk1k2,
                      long N1, long N2, config *config,
                      double smooth_dlnr, int dimension,
                      double *r1, double *r2, double **result,
                      char *errmsg, size_t errmsg_size)
{
    //B
    if (errmsg == NULL || errmsg_size == 0)
        return FAILURE;

    if (k1 == NULL || k2 == NULL || fk1k2 == NULL || config == NULL ||
        r1 == NULL || r2 == NULL || result == NULL) {
        snprintf(errmsg, errmsg_size, "two_Bessel_binave received NULL input");
        return FAILURE;
    }
    //E

    if (N1 < 2 || N2 < 2 || N1 % 2 || N2 % 2) {
        snprintf(errmsg, errmsg_size,
                 "invalid FFTLog grid sizes: N1=%ld N2=%ld, both must be even and >= 2",
                 N1, N2);
        return FAILURE;
    }
    
    long halfN1 = N1/2, halfN2 = N2/2;

    //B    
    //B
    for (long i = 0; i < N1; i++) {
        if (k1[i] <= 0.0 || !isfinite(k1[i]) ||
            fk1k2[i] == NULL || result[i] == NULL) {
            snprintf(errmsg, errmsg_size,
                     "invalid k1/fk/result row at i=%ld", i);
            return FAILURE;
        }

        if (i > 0 && k1[i] <= k1[i - 1]) {
            snprintf(errmsg, errmsg_size,
                     "FFTLog k1 grid must be strictly increasing at i=%ld: k1[i-1]=%g k1[i]=%g",
                     i, k1[i - 1], k1[i]);
            return FAILURE;
        }
    }

    for (long j = 0; j < N2; j++) {
        if (k2[j] <= 0.0 || !isfinite(k2[j])) {
            snprintf(errmsg, errmsg_size,
                     "invalid k2 value at j=%ld", j);
            return FAILURE;
        }

        if (j > 0 && k2[j] <= k2[j - 1]) {
            snprintf(errmsg, errmsg_size,
                     "FFTLog k2 grid must be strictly increasing at j=%ld: k2[j-1]=%g k2[j]=%g",
                     j, k2[j - 1], k2[j]);
            return FAILURE;
        }
    }
    //E
    //E
    
    //B
    size_t n1 = (size_t)N1;
    size_t n2 = (size_t)N2;
    size_t halfn2p1 = (size_t)(halfN2 + 1);

    if (n1 > SIZE_MAX / n2) {
        snprintf(errmsg, errmsg_size, "FFTLog grid size overflows: N1=%ld N2=%ld", N1, N2);
        return FAILURE;
    }

    size_t grid_count = n1 * n2;

    if (grid_count > SIZE_MAX / sizeof(double)) {
        snprintf(errmsg, errmsg_size, "FFTLog double allocation overflows: N1=%ld N2=%ld", N1, N2);
        return FAILURE;
    }

    if (n1 > SIZE_MAX / halfn2p1 ||
        n1 * halfn2p1 > SIZE_MAX / sizeof(fftw_complex)) {
        snprintf(errmsg, errmsg_size, "FFTLog complex allocation overflows: N1=%ld N2=%ld", N1, N2);
        return FAILURE;
    }

    size_t fft_count = n1 * halfn2p1;
    //E
    
    double k10, k20, r10,r20;
    k10 = k1[0];
    k20 = k2[0];
    
    double dlnk1, dlnk2;
    dlnk1 = log(k1[1]/k10);
    dlnk2 = log(k2[1]/k20);
    
    // Only calculate the m>=0 part
    //B
    double *eta_m = NULL;
    double *eta_n = NULL;
    double complex *g1 = NULL;
    double complex *g2 = NULL;
    
    long i, j;
    eta_m = calloc((size_t)(halfN1 + 1), sizeof(*eta_m));
    eta_n = calloc((size_t)(halfN2 + 1), sizeof(*eta_n));
    g1 = calloc((size_t)(halfN1 + 1), sizeof(*g1));
    g2 = calloc((size_t)(halfN2 + 1), sizeof(*g2));

    if (eta_m == NULL || eta_n == NULL || g1 == NULL || g2 == NULL) {
        snprintf(errmsg, errmsg_size,
                 "failed to allocate FFTLog work arrays for N1=%ld N2=%ld",
                 N1, N2);
        free(eta_m);
        free(eta_n);
        free(g1);
        free(g2);
        return FAILURE;
    }

    for (i = 0; i <= halfN1; i++)
        eta_m[i] = 2.0 * M_PI / dlnk1 / N1 * i;
    for (j = 0; j <= halfN2; j++)
        eta_n[j] = 2.0 * M_PI / dlnk2 / N2 * j;

    g_l_smooth(config->l1 - 0.5, config->nu1, eta_m, g1,
               halfN1 + 1, smooth_dlnr, dimension + 0.5);
    g_l_smooth(config->l2 - 0.5, config->nu2, eta_n, g2,
               halfN2 + 1, smooth_dlnr, dimension + 0.5);
    //E

    double s_d_lambda = (exp(dimension*smooth_dlnr) - 1.) / dimension;
    
    // calculate r1,r2 arrays
    for(i=0; i<N1; i++) {r1[i] = 1. / k1[N1-1-i];}
    for(i=0; i<N2; i++) {r2[i] = 1. / k2[N2-1-i];}
    r10 = r1[0];
    r20 = r2[0];
    
    // biased input func
    double *Pb;
    Pb = malloc(grid_count * sizeof(*Pb));

    if (Pb == NULL) {
        snprintf(errmsg, errmsg_size,
                 "failed to allocate Pb array for %ld x %ld FFTLog grid",
                 N1, N2);
        
        free(eta_m);
        free(eta_n);
        free(g1);
        free(g2);
        
        return FAILURE;
    }

    for(i=0; i<N1; i++) {
        for(j=0; j<N2; j++) {
            Pb[(size_t)i * n2 + (size_t)j] =
                            fk1k2[i][j] / pow(k1[i], config->nu1 -0.5)
                            / pow(k2[j], config->nu2 -0.5) ;
        }
    }
    
    fftw_complex *out;
    fftw_plan plan_forward, plan_backward;
    out = fftw_malloc(fft_count * sizeof(*out));

    if (out == NULL) {
        snprintf(errmsg, errmsg_size,
                 "fftw_malloc failed for forward transform output, N1=%ld N2=%ld",
                 N1, N2);
        free(Pb);
        
        free(eta_m);
        free(eta_n);
        free(g1);
        free(g2);
        
        return FAILURE;
    }

    plan_forward = fftw_plan_dft_r2c_2d(N1, N2, Pb, out, FFTW_ESTIMATE);
    if (plan_forward == NULL) {
        snprintf(errmsg, errmsg_size,
                 "fftw_plan_dft_r2c_2d failed for N1=%ld N2=%ld",
                 N1, N2);
        fftw_free(out);
        free(Pb);
        free(eta_m);
        free(eta_n);
        free(g1);
        free(g2);
        return FAILURE;
    }

    fftw_execute(plan_forward);
    fftw_destroy_plan(plan_forward);
    free(Pb);
    
    c_window_2d(out, config->c_window_width, halfN1, halfN2);
    
    long ij;
    for(i=0; i<=halfN1; i++) {
        for(j=0; j<=halfN2; j++) {
            ij = i*(halfN2+1) + j;
            out[ij] *= cpow(k20*r20, -I*eta_n[j]) * g2[j]
            * cpow(k10*r10, -I*eta_m[i]) * g1[i] ;
            out[ij] = conj(out[ij]);
        }
    }
    for(i=halfN1+1; i<N1; i++) {
        for(j=0; j<=halfN2; j++) {
            ij = i*(halfN2+1) + j;
            out[ij] *= cpow(k20*r20, -I*eta_n[j]) * g2[j] * cpow(k10*r10, I*eta_m[N1-i]) * conj(g1[N1-i]) ;
            out[ij] = conj(out[ij]);
        }
    }
    double *out_ifft;
    out_ifft = malloc(grid_count * sizeof(*out_ifft));

    if (out_ifft == NULL) {
        snprintf(errmsg, errmsg_size,
                 "failed to allocate inverse FFT output for %ld x %ld grid",
                 N1, N2);
        fftw_free(out);
        
        free(eta_m);
        free(eta_n);
        free(g1);
        free(g2);
        return FAILURE;
    }

    plan_backward = fftw_plan_dft_c2r_2d(N1, N2, out, out_ifft, FFTW_ESTIMATE);
    if (plan_backward == NULL) {
        snprintf(errmsg, errmsg_size,
                 "fftw_plan_dft_c2r_2d failed for N1=%ld N2=%ld",
                 N1, N2);
        free(out_ifft);
        fftw_free(out);
        free(eta_m);
        free(eta_n);
        free(g1);
        free(g2);
        return FAILURE;
    }


    fftw_execute(plan_backward);
    fftw_destroy_plan(plan_backward);
    fftw_free(out);
    
    for(i=0; i<N1; i++) {
        for(j=0; j<N2; j++) {
            result[i][j] = out_ifft[(size_t)i * n2 + (size_t)j]
            / (8.*N1*N2 * pow(r2[j], config->nu2 -0.5)
               * pow(r1[i], config->nu1 -0.5))
            / s_d_lambda/s_d_lambda;

        }
    }
    
    free(eta_m);
    free(eta_n);
    free(g1);
    free(g2);
    
    free(out_ifft);
    return SUCCESS;
}
