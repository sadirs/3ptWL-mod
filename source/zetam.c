// ============================================================================
//        1          2          3          4        ^ 5          6          7

#include "zetam.h"
#include <stdlib.h>
#include <math.h>
#include <complex.h>
#include <limits.h>
#include <time.h>
#include <fftw3.h>

static int close_checked(FILE **fp, struct cmdline_data *cmd,
                         const char *routineName, const char *path)
{
    if (*fp == NULL)
        return SUCCESS;

    if (ferror(*fp)) {
        fclose(*fp);
        *fp = NULL;
        COSMO_FAIL(cmd, "%s: write error while writing %s\n",
                   routineName, path);
    }

    if (fclose(*fp) != 0) {
        *fp = NULL;
        COSMO_FAIL(cmd, "%s: error closing output file %s\n",
                   routineName, path);
    }

    *fp = NULL;
    return SUCCESS;
}

int get_zetam(struct cmdline_data* cmd,
               struct  global_data* gd)
{
    string routineName = "get_zetam";
    FILE *OUTtheta = NULL;
    FILE *fp = NULL;
    int status = SUCCESS;

    char outfilename_thetavector[100];

    int multipole;

    //B
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (cmd->mMax < 0 || cmd->mMax == INT_MAX)
        COSMO_FAIL(cmd, "%s: invalid mMax = %d\n", routineName, cmd->mMax);

    if (iv.Nell < 2)
        COSMO_FAIL(cmd, "%s: Nell must be at least 2, got %d\n",
                   routineName, iv.Nell);

    if (iv.ellT == NULL || iv.BmVectorsp == NULL ||
        zv.r1 == NULL || zv.r2 == NULL || zv.result == NULL)
        COSMO_FAIL(cmd, "%s: zetam input tables are not allocated\n",
                   routineName);

    if (cmd->rootDir == NULL || cmd->prefix == NULL)
        COSMO_FAIL(cmd, "%s: output path strings are not initialized\n",
                   routineName);

    if (!isfinite(iv.ellT[0]) || !isfinite(iv.ellT[1]) ||
        iv.ellT[0] <= 0.0 || iv.ellT[1] <= iv.ellT[0])
        COSMO_FAIL(cmd, "%s: invalid first ell values: ell[0]=%g ell[1]=%g\n",
                   routineName, iv.ellT[0], iv.ellT[1]);

    for (int i = 0; i < iv.Nell; i++) {
        if (!isfinite(iv.ellT[i]) || iv.ellT[i] <= 0.0)
            COSMO_FAIL(cmd, "%s: invalid ellT[%d]=%g\n",
                       routineName, i, iv.ellT[i]);

        if (i > 0 && iv.ellT[i] <= iv.ellT[i - 1])
            COSMO_FAIL(cmd, "%s: ell grid must be increasing at i=%d: ell[i-1]=%g ell[i]=%g\n",
                       routineName, i, iv.ellT[i - 1], iv.ellT[i]);

        if (zv.result[i] == NULL)
            COSMO_FAIL(cmd, "%s: zv.result[%d] is not allocated\n",
                       routineName, i);
    }

    for (int m = 0; m <= cmd->mMax; m++) {
        if (iv.BmVectorsp[m] == NULL)
            COSMO_FAIL(cmd, "%s: iv.BmVectorsp[%d] is not allocated\n",
                       routineName, m);
    }
    //E
    
    double dlnell = log(iv.ellT[1]/iv.ellT[0]);
    
    int Nell=iv.Nell;

    long n_data, ndatamax;

    config config_m;
        config_m.l1 = 0;
        config_m.l2 = 0;
        config_m.nu1 = 1.01;
        config_m.nu2 = 1.01;
        config_m.c_window_width = 0.25;
        config_m.sys_Flag = 0;

    double **Bmell1ell2 = NULL;
    int Bmell1ell2_rows = 0;
    double smooth_dlnr = dlnell;
    int dimension = 2;

    Bmell1ell2 = calloc((size_t)Nell, sizeof(*Bmell1ell2));
    if (Bmell1ell2 == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating Bmell1ell2 row pointers for Nell=%d\n",
                        routineName, Nell);
    }

    for (int i = 0; i < Nell; i++) {
        Bmell1ell2[i] = calloc((size_t)Nell, sizeof(**Bmell1ell2));
        if (Bmell1ell2[i] == NULL) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup,
                            "%s: not enough memory allocating Bmell1ell2 row %d for Nell=%d\n",
                            routineName, i, Nell);
        }
        Bmell1ell2_rows++;
    }

    clock_t start = clock();
    
    char int_str[20];
    char str[MAXLENGTHOFFILES];

    for (int multipole=0; multipole<cmd->mMax+1; multipole++){
        config_m.l1 = multipole;
        config_m.l2 = multipole;

        for(int i=0; i<iv.Nell ; i++){
            for(int j=0; j<iv.Nell ; j++){
                Bmell1ell2[i][j] =
                    iv.ellT[i]*iv.ellT[i]*iv.ellT[j]*iv.ellT[j]
                    * iv.BmVectorsp[multipole][i*iv.Nell+j] / m_2PI2;
            }
        }

        char fftlog_errmsg[_ERRORMSGSIZE_] = "";
        if (two_Bessel_binave(iv.ellT, iv.ellT, Bmell1ell2,
                              iv.Nell, iv.Nell,
                              &config_m, smooth_dlnr, dimension,
                              zv.r1, zv.r2, zv.result,
                              fftlog_errmsg, sizeof(fftlog_errmsg)) == FAILURE) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup,
                            "%s: FFTLog two_Bessel_binave failed: %s\n",
                            routineName, fftlog_errmsg);
        }

        //B
        for (int i=0; i<iv.Nell; i++) {
            if (!isfinite(zv.r1[i]) || !isfinite(zv.r2[i])) {
                status = FAILURE;
                COSMO_FAIL_GOTO(cmd, cleanup,
                                "%s: FFTLog produced non-finite theta value at i=%d: r1=%g r2=%g\n",
                                routineName, i, zv.r1[i], zv.r2[i]);
            }
        }

        for (int i=0; i<iv.Nell; i++) {
            for (int j=0; j<iv.Nell; j++) {
                if (!isfinite(zv.result[i][j])) {
                    status = FAILURE;
                    COSMO_FAIL_GOTO(cmd, cleanup,
                    "%s: FFTLog produced non-finite zetam value for multipole=%d at [%d,%d]: %g\n",
                    routineName, multipole, i, j, zv.result[i][j]);
                }
            }
        }
        //E
        
        if(multipole==0){
            int nwrite = snprintf(str, sizeof(str), "%s/%stheta_array.txt",
                                  cmd->rootDir,cmd->prefix);

            if (nwrite < 0 || (size_t)nwrite >= sizeof(str)) {
                status = FAILURE;
                COSMO_FAIL_GOTO(cmd, cleanup,
                                "%s: Bmells output path too long\n",
                                routineName);
            }

            OUTtheta = fopen(str, "w+");
            if (OUTtheta == NULL) {
                status = FAILURE;
                COSMO_FAIL_GOTO(cmd, cleanup,
                                "%s: cannot open output file %s\n",
                                routineName, str);
            }
            
            for (int i = 0; i < iv.Nell; i++) {
                if (fprintf(OUTtheta, "%lg\n", zv.r1[i]) < 0) {
                    status = FAILURE;
                    COSMO_FAIL_GOTO(cmd, cleanup,
                                    "%s: write failed for %s\n",
                                    routineName, str);
                }
            }

            if (close_checked(&OUTtheta, cmd, routineName, str) == FAILURE) {
                status = FAILURE;
                goto cleanup;
            }
        }
    
        int nint = snprintf(int_str, sizeof(int_str), "%d", multipole);
        if (nint < 0 || (size_t)nint >= sizeof(int_str)) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup, "%s: multipole index string too long\n",
                            routineName);
        }
        
        int nwrite = snprintf(str, sizeof(str), "%s/%szetam%s.txt",
                              cmd->rootDir, cmd->prefix,int_str);
        if (nwrite < 0 || (size_t)nwrite >= sizeof(str)) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup,
                            "%s: zetam output path too long\n",
                            routineName);
        }
        fp = fopen(str, "w+");
        if (fp == NULL) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup,
                            "%s: cannot open output file %s\n",
                            routineName, str);
        }

        for (int i = 0; i < iv.Nell; i++) {
            for (int j = 0; j < iv.Nell; j++) {
                if (fprintf(fp, "%lg  ", zv.result[i][j]) < 0) {
                    status = FAILURE;
                    COSMO_FAIL_GOTO(cmd, cleanup,
                                    "%s: write failed for %s\n",
                                    routineName, str);
                }
            }

            if (i != iv.Nell - 1) {
                if (fprintf(fp, "\n") < 0) {
                    status = FAILURE;
                    COSMO_FAIL_GOTO(cmd, cleanup,
                                    "%s: write failed for %s\n",
                                    routineName, str);
                }
            }
        }

        if (close_checked(&fp, cmd, routineName, str) == FAILURE) {
            status = FAILURE;
            goto cleanup;
        }
        
    };

    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    printf("time zetams: %f \n", seconds);

cleanup:
    if (OUTtheta != NULL)
        fclose(OUTtheta);

    if (fp != NULL)
        fclose(fp);
    
    for (int i = 0; i < Bmell1ell2_rows; i++)
        free(Bmell1ell2[i]);
    free(Bmell1ell2);
    
    return status;
}

