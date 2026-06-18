/*==============================================================================
 NAME: procedures.c                [wlcf]
 Written by: A. Aviles et al.
 Starting date: 15.02.2026
 Purpose: Initial routine
 Language: C
 Major revision:
 ==============================================================================*/
//        1          2          3          4        ^ 5          6          7

#include <stdint.h>
#include <limits.h>

#include "functions.h"
#include "procedures.h"

#define m_PI   3.1415926535897932384626433

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

/*
 Initial routine:

 To be called in main:
    Initial(&cmd, &gd);

 This routine is in charge of the initialization

 Arguments:
    * `cmd`: Input: structure cmdline_data pointer
    * `gd`: Input: structure global_data pointer
 Return (the error status):
    int SUCCESS or FAILURE
 */
int Initial(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "Initial";

    if (cmd == NULL || gd == NULL) return FAILURE;
    gd->Dpz0 = Dplusf(cmd, gd, 0.0);
    if (!isfinite(gd->Dpz0))
        COSMO_FAIL(cmd, "%s: invalid Dplus(z=0): %g\n", routineName, gd->Dpz0);

    if (read_inputpk(cmd, gd) == FAILURE)           // Read linear PS
        return FAILURE;                             //  power spectrum from CAMB

    gd->sigma8 = sigmaRTH(cmd, gd, 8,0.001,8.,100); // Computation of sigma8
    if (!isfinite(gd->sigma8) || gd->sigma8 <= 0.0)
        COSMO_FAIL(cmd, "%s: invalid sigma8 from input P(k): %g\n",
                   routineName, gd->sigma8);

    return SUCCESS;
}

// Needs allocated vectors iv.BmVectors[m]
int Bmell(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "Bmell";
    double chimax, chimin;

    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (cmd->mMax < 0 || cmd->mMax == INT_MAX)
        COSMO_FAIL(cmd, "%s: invalid mMax = %d\n", routineName, cmd->mMax);

    if (iv.Nell < 2 || iv.chiQuadSteps < 2)
        COSMO_FAIL(cmd, "%s: invalid integration table sizes: Nell=%d chiQuadSteps=%d\n",
                   routineName, iv.Nell, iv.chiQuadSteps);

    if (iv.ellT == NULL ||
        iv.chiT_chiint == NULL ||
        iv.zT_chiint == NULL ||
        iv.DpT_chiint == NULL ||
        iv.rsigma_chiint == NULL ||
        iv.neff_chiint == NULL ||
        iv.q_chiint == NULL ||
        iv.BmVectors == NULL ||
        iv.BmVectorsp == NULL)
        COSMO_FAIL(cmd, "%s: integration tables are not allocated\n", routineName);

    size_t Nell = (size_t)iv.Nell;
    size_t NumMoments = (size_t)cmd->mMax + 1;
    size_t size_max = (size_t)-1;

    if (Nell > (size_t)INT_MAX / Nell)
        COSMO_FAIL(cmd, "%s: Nell (%d) is too large for int loops\n",
                   routineName, iv.Nell);

    size_t Nell2 = Nell * Nell;

    if (Nell2 > size_max / sizeof(double))
        COSMO_FAIL(cmd, "%s: Nell (%d) is too large for Nell*Nell arrays\n",
                    routineName, iv.Nell);

    if (NumMoments > size_max / sizeof(double *))
        COSMO_FAIL(cmd, "%s: mMax (%d) is too large for row pointers\n",
                    routineName, cmd->mMax);

    if (NumMoments > size_max / Nell2 / sizeof(double))
        COSMO_FAIL(cmd, "%s: Nell (%d) and mMax (%d) request too much Bm storage\n",
                   routineName, iv.Nell, cmd->mMax);

    int NumMomentsInt = cmd->mMax + 1;
    int Nell2Int = iv.Nell * iv.Nell;

    for (int m = 0; m < NumMomentsInt; m++) {
        if (iv.BmVectors[m] == NULL || iv.BmVectorsp[m] == NULL)
            COSMO_FAIL(cmd, "%s: Bm vector row %d is not allocated\n",
                       routineName, m);
    }
    
    //B
    for (int i = 0; i < iv.chiQuadSteps; i++) {
        if (!isfinite(iv.chiT_chiint[i]) || iv.chiT_chiint[i] <= 0.0)
            COSMO_FAIL(cmd, "%s: invalid chi value at i=%d: %g\n",
                       routineName, i, iv.chiT_chiint[i]);

        if (i > 0 && iv.chiT_chiint[i] <= iv.chiT_chiint[i - 1])
            COSMO_FAIL(cmd, "%s: chi grid must be increasing at i=%d: chi[i-1]=%g chi[i]=%g\n",
                       routineName, i, iv.chiT_chiint[i - 1], iv.chiT_chiint[i]);

        if (!isfinite(iv.zT_chiint[i]) ||
            !isfinite(iv.DpT_chiint[i]) ||
            !isfinite(iv.rsigma_chiint[i]) ||
            !isfinite(iv.neff_chiint[i]) ||
            !isfinite(iv.q_chiint[i]))
            COSMO_FAIL(cmd, "%s: non-finite integration table value at i=%d: z=%g Dp=%g rsigma=%g neff=%g q=%g\n",
                       routineName, i,
                       iv.zT_chiint[i],
                       iv.DpT_chiint[i],
                       iv.rsigma_chiint[i],
                       iv.neff_chiint[i],
                       iv.q_chiint[i]);
    }
    //E

    int status = SUCCESS;
//E

    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
    "\n%s: Computing Bm(ell1,ell2) for symmetric %d x %d array of ell values\n",
    routineName, iv.Nell, iv.Nell);
    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                        "    ellmin = %f, ellmax = %f  \n",
                        iv.ellmin, iv.ellmax);
    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                        "    Number of moments = %d\n", cmd->mMax+1);
    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                        "    Quadrature chi steps= %d\n", iv.chiQuadSteps);

    chimax = iv.chiT_chiint[iv.chiQuadSteps-1];
    chimin = iv.chiT_chiint[0];

    double chi, z, Dp, rsigma, neff, qv;
    double chi_im1;
    double chiprev, deltachi;
    
    //B
    double **BmvectorsB = NULL;
    double **BmvectorsA = NULL;
    int Bmvectors_rows = 0;

    #ifdef OPENMPCODE
    double **BmVectors = NULL;
    double **BmVectorsp = NULL;
    int BmVectors_rows = 0;
    #endif

    BmvectorsB = calloc((size_t)NumMoments, sizeof(*BmvectorsB));
    BmvectorsA = calloc((size_t)NumMoments, sizeof(*BmvectorsA));
    if (BmvectorsB == NULL || BmvectorsA == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating Bmell scratch row pointers\n",
                        routineName);
    }

    for (int m = 0; m < NumMoments; m++) {
        BmvectorsB[m] = calloc((size_t)iv.Nell * (size_t)iv.Nell,
                               sizeof(**BmvectorsB));
        BmvectorsA[m] = calloc((size_t)iv.Nell * (size_t)iv.Nell,
                               sizeof(**BmvectorsA));
        if (BmvectorsB[m] == NULL || BmvectorsA[m] == NULL) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup,
                            "%s: not enough memory allocating Bmell scratch row %d\n",
                            routineName, m);
        }
        Bmvectors_rows++;
    }

    #ifdef OPENMPCODE
    BmVectors = calloc((size_t)NumMoments, sizeof(*BmVectors));
    BmVectorsp = calloc((size_t)NumMoments, sizeof(*BmVectorsp));
    if (BmVectors == NULL || BmVectorsp == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating OpenMP Bm row pointers\n",
                        routineName);
    }

    for (int m = 0; m < NumMoments; m++) {
        BmVectors[m] = calloc((size_t)iv.Nell * (size_t)iv.Nell,
                              sizeof(**BmVectors));
        BmVectorsp[m] = calloc((size_t)iv.Nell * (size_t)iv.Nell,
                               sizeof(**BmVectorsp));
        if (BmVectors[m] == NULL || BmVectorsp[m] == NULL) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup,
                            "%s: not enough memory allocating OpenMP Bm row %d\n",
                            routineName, m);
        }
        BmVectors_rows++;
    }
    #endif
    //E
    
        clock_t localtime;
        chiprev = 0.0; //?

        for (int i=0;i<iv.chiQuadSteps;i++){
            localtime=clock();
            
            chi    = iv.chiT_chiint[i];
            z      = iv.zT_chiint[i];
            Dp     = iv.DpT_chiint[i];
            rsigma = iv.rsigma_chiint[i];
            neff   = iv.neff_chiint[i];
            qv     = iv.q_chiint[i];
            if(i>0)
                chi_im1 = iv.chiT_chiint[i-1];
            else
                chi_im1 = 0.0;

            deltachi = chi - chi_im1;
            
#ifdef OPENMPCODE
    if (Bm(cmd, gd, chi, z, Dp, rsigma, neff, BmVectors) == FAILURE) {
#else
    if (Bm(cmd, gd, chi, z, Dp, rsigma, neff) == FAILURE) {
#endif
             
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup, "%s: Bm failed at chi step %d\n",
                        routineName, i);
    }

        for (int m = 0; m < NumMomentsInt; m++) {
            for (int ij = 0; ij < Nell2Int; ij++) {
                    
#ifdef OPENMPCODE
                    BmvectorsB[m][ij] = pow(qv,3.)/pow(chi,4.) * BmVectors[m][ij];
#else
                    BmvectorsB[m][ij] = pow(qv,3.)/pow(chi,4.) * iv.BmVectors[m][ij];
#endif
                }
            }
            
            if(i==0){
                for (int m = 0; m < NumMomentsInt; m++)
                    for (int ij = 0; ij < Nell2Int; ij++)
                        BmvectorsA[m][ij] = BmvectorsB[m][ij];
                deltachi = chi;
            }

#ifdef OPENMPCODE
            for (int m = 0; m < NumMomentsInt; m++)
                for (int ij = 0; ij < Nell2Int; ij++)
                    BmVectorsp[m][ij] += 0.5*(BmvectorsA[m][ij]
                                            +BmvectorsB[m][ij])* deltachi;
#else
            for (int m = 0; m < NumMomentsInt; m++)
                for (int ij = 0; ij < Nell2Int; ij++)
                    iv.BmVectorsp[m][ij] += 0.5*(BmvectorsA[m][ij]
                                            +BmvectorsB[m][ij])* deltachi;
#endif

            for (int m = 0; m < NumMomentsInt; m++)
                for (int ij = 0; ij < Nell2Int; ij++)
                    BmvectorsA[m][ij] = BmvectorsB[m][ij];
        } // end loop i // end pragma for loop

#ifdef OPENMPCODE
        for (int m = 0; m < NumMomentsInt; m++)
            for (int ij = 0; ij < Nell2Int; ij++)
                iv.BmVectorsp[m][ij] += BmVectorsp[m][ij];

#endif
    
    //B TEST diagnostic output, non-fatal
    FILE *fp;
    char str[MAXLENGTHOFFILES];

    int nwrite = snprintf(str, sizeof(str), "%s/tests", cmd->rootDir);
    if (nwrite < 0 || (size_t)nwrite >= sizeof(str)) {
        verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "\n%s: warning!! tests directory path too long; skipping diagnostic output.\n",
                               routineName);
        goto skip_diagnostic;
    }

    if (mkdir_p(str, 0755) != 0) {
        verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "\n%s: warning!! directory %s can not be created; skipping diagnostic output.\n",
                               routineName, str);
        goto skip_diagnostic;
    }

    nwrite = snprintf(str, sizeof(str),
                      "%s/tests/i_z_chi_qv_Dp.txt", cmd->rootDir);
    if (nwrite < 0 || (size_t)nwrite >= sizeof(str)) {
        verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "\n%s: warning!! diagnostic output path too long; skipping diagnostic output.\n",
                               routineName);
        goto skip_diagnostic;
    }

    fp = fopen(str, "w+");
    if (fp != NULL) {
        for (int i = 0; i < iv.chiQuadSteps; i++) {
            fprintf(fp, "%d %15e %15e %15e %15e \n",
                    i, iv.zT_chiint[i], iv.chiT_chiint[i],
                    iv.q_chiint[i], iv.DpT_chiint[i]);
        }
        fclose(fp);
    } else {
        verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "\n%s: warning!! file %s can not be opened; skipping diagnostic output.\n",
                               routineName, str);
    }

    skip_diagnostic:
    //E TEST

    //B freeing memory
cleanup:
#ifdef OPENMPCODE
    for (int m = 0; m < BmVectors_rows; m++) {
        free(BmVectorsp[m]);
        free(BmVectors[m]);
    }
    free(BmVectorsp);
    free(BmVectors);
#endif

    for (int m = 0; m < Bmvectors_rows; m++) {
        free(BmvectorsA[m]);
        free(BmvectorsB[m]);
    }
    free(BmvectorsA);
    free(BmvectorsB);
    //E

    return status;
}


#ifdef OPENMPCODE
int Bm(struct cmdline_data* cmd, struct  global_data* gd,
        double chi, double z, double Dp, double r_sigma, double n_eff, double **BmVectors)
{
    string routineName = "Bm";
    
    //B
    if (cmd == NULL || gd == NULL || BmVectors == NULL)
        return FAILURE;

    if (cmd->mMax < 0 || cmd->mMax == INT_MAX)
        COSMO_FAIL(cmd, "%s: invalid mMax = %d\n", routineName, cmd->mMax);

    if (cmd->GLpoints < 2)
        COSMO_FAIL(cmd, "%s: GLpoints (%d) must be at least 2\n",
                   routineName, cmd->GLpoints);

    if (iv.Nell < 2 || iv.ellT == NULL)
        COSMO_FAIL(cmd, "%s: ell table is not allocated or has invalid size\n",
                   routineName);

    if (!isfinite(chi) || chi <= 0.0 ||
        !isfinite(z) ||
        !isfinite(Dp) ||
        !isfinite(r_sigma) || r_sigma <= 0.0 ||
        !isfinite(n_eff))
        COSMO_FAIL(cmd, "%s: invalid Bm inputs: chi=%g z=%g Dp=%g r_sigma=%g n_eff=%g\n",
                   routineName, chi, z, Dp, r_sigma, n_eff);

    size_t Nell = (size_t)iv.Nell;
    size_t NumMomentsSize = (size_t)cmd->mMax + 1;
    size_t size_max = (size_t)-1;

    if (Nell > (size_t)INT_MAX / Nell)
        COSMO_FAIL(cmd, "%s: Nell (%d) is too large for int indexing\n",
                   routineName, iv.Nell);

    size_t Nell2 = Nell * Nell;

    if (Nell2 > size_max / sizeof(double) ||
        NumMomentsSize > size_max / sizeof(double *) ||
        NumMomentsSize > size_max / Nell2 / sizeof(double))
        COSMO_FAIL(cmd, "%s: Bm output dimensions are too large\n", routineName);

    int NumMoments = cmd->mMax + 1;

    for (int m = 0; m < NumMoments; m++) {
        if (BmVectors[m] == NULL)
            COSMO_FAIL(cmd, "%s: BmVectors[%d] is not allocated\n",
                       routineName, m);
    }
    //E

    int status = SUCCESS;
    double *xGL = NULL;
    double *wGL = NULL;

    double k1,k2,varphi,w,k3,BT, ell1,ell2;
    int m=0;
    double EFTctr;
    double alphaEFT;
    
    xGL = calloc((size_t)cmd->GLpoints, sizeof(*xGL));
    wGL = calloc((size_t)cmd->GLpoints, sizeof(*wGL));

    if (xGL == NULL || wGL == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating Gauss-Legendre arrays\n",
                        routineName);
    }
    
    gaussleg(-m_PI, m_PI, xGL, wGL, cmd->GLpoints);

    alphaEFT=-3.0;  // =cmd.alphaEFT

    EFTctr=alphaEFT*pow(Dp,2);

    int failed = 0;
    
#ifdef OPENMPCODE
#pragma omp parallel for schedule(dynamic) default(none) \
    shared(cmd, gd, iv, BmVectors, xGL, wGL, NumMoments, chi, z, Dp, r_sigma, n_eff, EFTctr, failed)
#endif
for (int i = 0; i < iv.Nell; i++) {
    for (int j = i; j < iv.Nell; j++) {
        
        int local_failed;
        #pragma omp atomic read
        local_failed = failed;
        if (local_failed)
            continue;

        double ell1 = iv.ellT[i];
        double ell2 = iv.ellT[j];
        double k1 = ell1 / chi;
        double k2 = ell2 / chi;
        
        double *val = calloc((size_t)NumMoments, sizeof(*val));
        if (val == NULL) {
            #pragma omp atomic write
            failed = 1;
            continue;
        }

        for (int m = 0; m < NumMoments; m++)
            val[m] = 0.0;

        for (int p = 0; p < cmd->GLpoints; p++) {
            double varphi = xGL[p];
            double w = wGL[p];
            double k3 = sqrt(k1*k1 + k2*k2 - 2.0*k1*k2*cos(varphi));

            double BT;
            if (cmd->tree_level == 1) {
                BT = Bispec_tree(cmd, gd, k1, k2, k3, Dp);
            } else if (cmd->tree_level == 2) {
                BT = Bispec_P2(cmd, gd, k1, k2, k3, Dp);
            } else if (cmd->tree_level == 3) {
                BT = Bispec_tree_EFT(cmd, gd, k1, k2, k3, Dp, EFTctr);
            } else {
                BT = Bispec_Takahashi(cmd, gd, k1, k2, k3, z, Dp, r_sigma, n_eff);
            }

            if (!isfinite(BT)) {
                 #pragma omp atomic write
                 failed = 1;
                 continue;
             }
            
            for (int m = 0; m < NumMoments; m++)
                val[m] += w * BT * cos(m * varphi);
        }

        for (int m = 0; m < NumMoments; m++) {
            double out = val[m] / (2.0 * m_PI);
            
            if (!isfinite(out)) {
                 #pragma omp atomic write
                 failed = 1;
                 continue;
             }

            
            BmVectors[m][i * iv.Nell + j] = out;
            if (j != i)
                BmVectors[m][j * iv.Nell + i] = out;
        }
        free(val);
    }
}

    if (failed) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: non-finite value while computing BmVectors\n",
                        routineName);
    }
        
cleanup:
    free(wGL);
    free(xGL);
    
    return status;
}
#else
int Bm(struct cmdline_data* cmd, struct  global_data* gd,
        double chi, double z, double Dp, double r_sigma, double n_eff)
{
    string routineName = "Bm";
    
    //B initial checking
    if (cmd == NULL || gd == NULL || iv.BmVectors == NULL)
        return FAILURE;

    if (cmd->mMax < 0 || cmd->mMax == INT_MAX)
        COSMO_FAIL(cmd, "%s: invalid mMax = %d\n", routineName, cmd->mMax);

    if (cmd->GLpoints < 2)
        COSMO_FAIL(cmd, "%s: GLpoints (%d) must be at least 2\n",
                   routineName, cmd->GLpoints);

    if (iv.Nell < 2 || iv.ellT == NULL)
        COSMO_FAIL(cmd, "%s: ell table is not allocated or has invalid size\n",
                   routineName);

    if (!isfinite(chi) || chi <= 0.0 ||
        !isfinite(z) ||
        !isfinite(Dp) ||
        !isfinite(r_sigma) || r_sigma <= 0.0 ||
        !isfinite(n_eff))
        COSMO_FAIL(cmd, "%s: invalid Bm inputs: chi=%g z=%g Dp=%g r_sigma=%g n_eff=%g\n",
                   routineName, chi, z, Dp, r_sigma, n_eff);

    size_t Nell = (size_t)iv.Nell;
    size_t NumMomentsSize = (size_t)cmd->mMax + 1;
    size_t size_max = (size_t)-1;

    if (Nell > (size_t)INT_MAX / Nell)
        COSMO_FAIL(cmd, "%s: Nell (%d) is too large for int indexing\n",
                   routineName, iv.Nell);

    size_t Nell2 = Nell * Nell;

    if (Nell2 > size_max / sizeof(double) ||
        NumMomentsSize > size_max / sizeof(double *) ||
        NumMomentsSize > size_max / Nell2 / sizeof(double))
        COSMO_FAIL(cmd, "%s: Bm output dimensions are too large\n", routineName);

    int NumMoments = cmd->mMax + 1;

    for (int m = 0; m < NumMoments; m++) {
        if (iv.BmVectors[m] == NULL)
            COSMO_FAIL(cmd, "%s: iv.BmVectors[%d] is not allocated\n",
                       routineName, m);
    }
    //E

    int status = SUCCESS;
    double *xGL = NULL;
    double *wGL = NULL;

    double k1,k2,varphi,w,k3,BT, ell1,ell2;

    int m=0;
    double *val = NULL;
    val = calloc((size_t)NumMoments, sizeof(*val));
    if (val == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating val\n",
                        routineName);
    }

    double EFTctr;
    double alphaEFT;

    xGL = calloc((size_t)cmd->GLpoints, sizeof(*xGL));
    wGL = calloc((size_t)cmd->GLpoints, sizeof(*wGL));

    if (xGL == NULL || wGL == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating Gauss-Legendre arrays\n",
                        routineName);
    }
    
    gaussleg(-m_PI, m_PI, xGL, wGL, cmd->GLpoints);

    alphaEFT=-3.0;  // =cmd.alphaEFT

    EFTctr=alphaEFT*pow(Dp,2);

    
    for(int i=0; i<iv.Nell; i++) {
        for(int j=i; j<iv.Nell; j++) {
            ell1=iv.ellT[i];
            ell2=iv.ellT[j];
            k1 = ell1/chi;
            k2 = ell2/chi;
            for(int m=0;m<NumMoments;m++) val[m] = 0.0;

            for (int p = 0; p < cmd->GLpoints; p++) {
                varphi = xGL[p];
                w   = wGL[p];
                k3  = sqrt( k1*k1 + k2*k2 - 2.*k1*k2 * cos(varphi) );
                if (cmd->tree_level==1) {
                    BT = Bispec_tree(cmd, gd, k1, k2, k3, Dp);
                } else if (cmd->tree_level==2) {
                    BT = Bispec_P2(cmd, gd, k1, k2, k3, Dp);
                } else if (cmd->tree_level==3) {
                    BT  = Bispec_tree_EFT(cmd, gd, k1, k2, k3, Dp, EFTctr);
                } else {
                    BT  = Bispec_Takahashi(cmd, gd, k1, k2,
                                           k3, z, Dp, r_sigma, n_eff);
                }
                
                if (!isfinite(BT)) {
                    status = FAILURE;
                    COSMO_FAIL_GOTO(cmd, cleanup,
                                    "%s: non-finite bispectrum value at ell i=%d j=%d p=%d: k1=%g k2=%g k3=%g z=%g BT=%g\n",
                                    routineName, i, j, p, k1, k2, k3, z, BT);
                }

                for (int m = 0; m < NumMoments; m++) {
                    double term = w * BT * cos(m * varphi);

                    if (!isfinite(term)) {
                        status = FAILURE;
                        COSMO_FAIL_GOTO(cmd, cleanup,
                                        "%s: non-finite Bm quadrature term for m=%d at i=%d j=%d p=%d\n",
                                        routineName, m, i, j, p);
                    }

                    val[m] += term;
                }
            }

            for(int m=0; m<NumMoments; m++) {
                double out = val[m] / (2.0 * m_PI);

                if (!isfinite(out)) {
                    status = FAILURE;
                    COSMO_FAIL_GOTO(cmd, cleanup,
                                    "%s: non-finite Bm value for m=%d at i=%d j=%d: %g\n",
                                    routineName, m, i, j, out);
                }

                iv.BmVectors[m][i * iv.Nell + j] = out;
                if (j != i)
                    iv.BmVectors[m][j * iv.Nell + i] = out;
            }
        }
    }

cleanup:
    free(val);
    free(wGL);
    free(xGL);

    return status;

}
#endif

int BmKspace(struct cmdline_data* cmd, struct  global_data* gd,
              int Maxm, double kmin, double kmax, int Nk,
              int GLpoints, double z, double Dp, double r_sigma, double n_eff)
{
    string routineName = "BmKspace";
    double cpumiddle;
    FILE *fp = NULL;
    int status = SUCCESS;

    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if(cmd->verbose>0) printf("\nComputing Bm(k1,k2) for symmetric array of %d x %d k1,k2 values  \n", Nk, Nk);
    
    double k1,k2,varphi,w,k3,BT;
    
    double *kT = NULL;
    double *xGL = NULL;
    double *wGL = NULL;
    
    double ***mat = NULL;
    
    int mat_m_allocated = 0;
    int *mat_rows_allocated = NULL;

    double *val = NULL;
    
    //B checking arguments
    if (Maxm < 0 || Maxm == INT_MAX)
        COSMO_FAIL(cmd, "%s: invalid Maxm = %d\n", routineName, Maxm);

    if (Nk < 2) {
        COSMO_FAIL(cmd, "%s: Nk must be at least 2, got %d\n",
                   routineName, Nk);
    }

    if (GLpoints < 2) {
        COSMO_FAIL(cmd, "%s: GLpoints must be at least 2, got %d\n",
                   routineName, GLpoints);
    }

    if (!isfinite(kmin) || !isfinite(kmax) || kmin <= 0.0 || kmax <= kmin) {
        COSMO_FAIL(cmd, "%s: invalid k range: kmin=%g kmax=%g\n",
                   routineName, kmin, kmax);
    }

    if (!isfinite(z) || !isfinite(Dp) || !isfinite(r_sigma) ||
        !isfinite(n_eff) || Dp <= 0.0 || r_sigma <= 0.0) {
        COSMO_FAIL(cmd,
                   "%s: invalid cosmology inputs: z=%g Dp=%g r_sigma=%g n_eff=%g\n",
                   routineName, z, Dp, r_sigma, n_eff);
    }

    //B Last instead of first one
    if ((size_t)(Maxm + 1) > SIZE_MAX / sizeof(*val)) {
        COSMO_FAIL(cmd, "%s: Maxm too large for val allocation: %d\n",
                   routineName, Maxm);
    }
    
    size_t NumMoments = (size_t)Maxm + 1;

    if (NumMoments > SIZE_MAX / sizeof(*val)) {
        COSMO_FAIL(cmd, "%s: Maxm too large for val allocation: %d\n",
                   routineName, Maxm);
    }
    //E

    if ((size_t)Nk > SIZE_MAX / sizeof(*kT) ||
        (size_t)GLpoints > SIZE_MAX / sizeof(*xGL)) {
        COSMO_FAIL(cmd, "%s: Nk or GLpoints too large: Nk=%d GLpoints=%d\n",
                   routineName, Nk, GLpoints);
    }
    //E
    
    val = calloc(NumMoments, sizeof(*val));

    int m=0;

    //B
    mat = calloc(NumMoments, sizeof(*mat));

    if (mat == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup, "%s: not enough memory allocating mat\n", routineName);
    }

    mat_rows_allocated = calloc(NumMoments, sizeof(*mat_rows_allocated));

    if (mat_rows_allocated == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup, "%s: not enough memory allocating mat row counters\n", routineName);
    }

    if (val == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating val\n",
                        routineName);
    }

    for (int m = 0; m <= Maxm; m++) {
        mat[m] = calloc((size_t)Nk, sizeof(*mat[m]));
        if (mat[m] == NULL) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup, "%s: not enough memory allocating mat[%d]\n", routineName, m);
        }
        mat_m_allocated++;

        for (int i = 0; i < Nk; i++) {
            mat[m][i] = calloc((size_t)Nk, sizeof(*mat[m][i]));
            if (mat[m][i] == NULL) {
                status = FAILURE;
                COSMO_FAIL_GOTO(cmd, cleanup, "%s: not enough memory allocating mat[%d][%d]\n",
                                routineName, m, i);
            }
            mat_rows_allocated[m]++;
        }
    }
    
    kT = calloc((size_t)Nk, sizeof(*kT));
    xGL = calloc((size_t)GLpoints, sizeof(*xGL));
    wGL = calloc((size_t)GLpoints, sizeof(*wGL));

    if (kT == NULL || xGL == NULL || wGL == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup, "%s: not enough memory allocating quadrature arrays\n",
                        routineName);
    }
    //E

    gaussleg(-m_PI, m_PI, xGL, wGL, GLpoints);
    
    if(cmd->verbose==2)
        printf("Maxm=%d, kmin=%f ,kmax=%f, Nk=%d, GLpoints=%d \n",
               Maxm,    kmin,    kmax,    Nk,    GLpoints);
    if(cmd->verbose==2)
        printf("z=%f, Dp=%f, rsigma=%f, neff=%f \n", z,Dp,r_sigma,n_eff);
    
    for(int i=0; i<Nk ; i++){
        kT[i]= exp(log(kmin) + i*log(kmax/kmin)/(Nk-1.0));
    }

    cpumiddle = CPUTIME;

    for(int i=0; i<Nk ; i++){
    for(int j=i; j<Nk ; j++){
        k1=kT[i];
        k2=kT[j];
        for(int m=0;m<=Maxm;m++) val[m] = 0.0;
        
        for (int p = 0; p < GLpoints; p++) {
            varphi = xGL[p];
            w = wGL[p];
            k3 = sqrt(k1*k1 + k2*k2 - 2.0*k1*k2*cos(varphi));

            BT = Bispec_Takahashi(cmd, gd, k1, k2, k3, z, Dp, r_sigma, n_eff);
            if (!isfinite(BT)) {
                status = FAILURE;
                COSMO_FAIL_GOTO(cmd, cleanup,
                                "%s: non-finite Bispec_Takahashi value at i=%d j=%d p=%d: k1=%g k2=%g k3=%g z=%g BT=%g\n",
                                routineName, i, j, p, k1, k2, k3, z, BT);
            }

            for (int m = 0; m <= Maxm; m++) {
                double term = w * BT * cos(m * varphi);
                if (!isfinite(term)) {
                    status = FAILURE;
                    COSMO_FAIL_GOTO(cmd, cleanup,
                                    "%s: non-finite Bnk quadrature term for m=%d at i=%d j=%d p=%d\n",
                                    routineName, m, i, j, p);
                }
                val[m] += term;
            }
        }

        for (int m = 0; m <= Maxm; m++) {
            if (!isfinite(val[m])) {
                status = FAILURE;
                COSMO_FAIL_GOTO(cmd, cleanup,
                                "%s: non-finite Bnk value for m=%d at i=%d j=%d: %g\n",
                                routineName, m, i, j, val[m]);
            }

            mat[m][i][j] = val[m];
            if (j != i)
                mat[m][j][i] = val[m];
        }
        
    }
    }
    
    if(cmd->verbose==2)
        printf("Bnk time = %f15 \n", CPUTIME - cpumiddle);
    if(cmd->verbose==2)
        printf("\n");
    
    // Write
    char int_str[20];
    char str[MAXLENGTHOFFILES];
    for (int m=0; m<Maxm+1; m++){
        int nint = snprintf(int_str, sizeof(int_str), "%d", m);
        if (nint < 0 || (size_t)nint >= sizeof(int_str)) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup, "%s: moment index string too long\n",
                            routineName);
        }
        
        int nwrite = snprintf(str, sizeof(str),
                            "%s/%sBnk_%s.txt",cmd->rootDir,cmd->prefix,int_str);
        if (nwrite < 0 || (size_t)nwrite >= sizeof(str)){
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup, "%s: Bnk output path too long\n",
                            routineName);
        }

        fp = fopen(str, "w+");
        if (fp == NULL) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup, "%s: cannot open output file %s\n",
                            routineName, str);
        }
        
        for (int i = 0; i < Nk; i++) {
            for (int j = 0; j < Nk; j++) {
                if (fprintf(fp, "%15e   ", mat[m][i][j]) < 0) {
                    status = FAILURE;
                    COSMO_FAIL_GOTO(cmd, cleanup,
                                    "%s: write failed for %s\n",
                                    routineName, str);
                }

                if (j == Nk - 1 && i != Nk - 1) {
                    if (fprintf(fp, " \n") < 0) {
                        status = FAILURE;
                        COSMO_FAIL_GOTO(cmd, cleanup,
                                        "%s: write failed for %s\n",
                                        routineName, str);
                    }
                }
            }
        }
        
        if (close_checked(&fp, cmd, routineName, str) == FAILURE) {
            status = FAILURE;
            goto cleanup;
        }
    }

    int nwrite = snprintf(str, sizeof(str),
                          "%s/%skArray.txt",cmd->rootDir,cmd->prefix);
    if (nwrite < 0 || (size_t)nwrite >= sizeof(str)) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup, "%s: kArray output path too long\n",
                        routineName);
    }
    fp = fopen(str, "w+");
    if (fp == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup, "%s: cannot open output file %s\n",
                        routineName, str);
    }

    for (int i = 0; i < Nk; i++) {
        if (fprintf(fp, "%15e\n", kT[i]) < 0) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup,
                            "%s: write failed for %s\n",
                            routineName, str);
        }
    }
    
    if (close_checked(&fp, cmd, routineName, str) == FAILURE) {
        status = FAILURE;
        goto cleanup;
    }

cleanup:
    if (fp != NULL)
        fclose(fp);

    free(val);
    free(wGL);
    free(xGL);
    free(kT);

    if (mat != NULL) {
        for (int m = 0; m < mat_m_allocated; m++) {
            if (mat[m] != NULL) {
                for (int i = 0; i < mat_rows_allocated[m]; i++)
                    free(mat[m][i]);
                free(mat[m]);
            }
        }
    }
    free(mat_rows_allocated);
    free(mat);
    
    return status;
}


//B Routines...

#define EPSGL 3.0e-11
void gaussleg(double x1, double x2, double xGL[], double wGL[], int n)
{
    int m,j,i;
    double z1,z,xm,xl,pp,p3,p2,p1;
    
    m=(n+1)/2;
    xm=0.5*(x2+x1);
    xl=0.5*(x2-x1);
    for (i=1;i<=m;i++){
        z=cos(m_PI*(i-0.25)/(n+0.5));
        do {
            p1=1.0;
            p2=0.0;
            for (j=1;j<=n;j++) {
                p3=p2;
                p2=p1;
                p1=((2.0*j-1.0)*z*p2-(j-1.0)*p3)/j;
            }
            pp=n*(z*p1-p2)/(z*z-1.0);
            z1=z;
            z=z1-p1/pp;
        } while (fabs(z-z1) > EPSGL);
        xGL[i-1]=xm-xl*z;
        xGL[n+1-i-1]=xm+xl*z;
        wGL[i-1]=2.0*xl/((1.0-z*z)*pp*pp);
        wGL[n+1-i-1]=wGL[i-1];
    }
}
#undef EPSGL


//B input power spectrum
int read_inputpk(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "read_inputpk";
    FILE *fp;
    double k_data, pkz0_data;

    if (cmd == NULL || gd == NULL)
        return FAILURE;
    if (cmd->fnamePS == NULL) {
        COSMO_FAIL(cmd,
                   "\n%s: power spectrum file name is NULL\n\n",
                   routineName);
        return FAILURE;
    }

    gd->n_data=0;

    fp = fopen(cmd->fnamePS, "r");

    if (fp == NULL && cmd->fnamePS[0] != '/') {
        char fallback[MAXLENGTHOFFILES];

        int nwrite = snprintf(fallback, sizeof(fallback),
                              "%s/tests/%s", __WLCFDIR__, cmd->fnamePS);

        if (nwrite >= 0 && (size_t)nwrite < sizeof(fallback)) {
            fp = fopen(fallback, "r");
        }
    }

    if (fp == NULL) {
        COSMO_FAIL(cmd,
                   "\n%s: linear power spectrum can't be opened (%s)\n\n",
                   routineName, cmd->fnamePS);
    }

    //B input: k[h/Mpc] P(k)[(Mpc/h)^3]
    int line;
        while ((line = fscanf(fp, "%lf %lf", &k_data, &pkz0_data)) != EOF) {
            if (line != 2) {
                fclose(fp);
                COSMO_FAIL(cmd,
            "%s: linear power spectrum file must have two columns of values... exiting\n\n",
                           routineName);
            }

            if (gd->n_data >= n_data_max) {
                fclose(fp);
                COSMO_FAIL(cmd,
            "%s: n_data_max should be larger than the number of data lines\n",
                routineName);

            }

            gd->k_data[gd->n_data] = k_data;
            gd->pkz0_data[gd->n_data] = pkz0_data;
            gd->n_data++;
        }
        
        fclose(fp);

    if (gd->n_data < 2)
        COSMO_FAIL(cmd,
        "\n%s: LPS file must have at least two rows of values... exiting\n\n",
        routineName);

    for (int i=0; i<gd->n_data; i++) {
        if (!isfinite(gd->k_data[i]))
            COSMO_FAIL(cmd,
                "\n%s: LPS file must have finite k values... exiting\n\n",
                routineName);
        if (!isfinite(gd->pkz0_data[i]))
            COSMO_FAIL(cmd,
                "\n%s: LPS file must have finite pkz0 values... exiting\n\n",
                routineName);
        if (gd->k_data[i] <= 0.)
            COSMO_FAIL(cmd,
                "\n%s: LPS file must have k values positive... exiting\n\n",
                routineName);
        if (gd->pkz0_data[i] <= 0.)
            COSMO_FAIL(cmd,
                "\n%s: LPS file must have positive pkz0 values... exiting\n\n",
                routineName);
    }

    for (int i=0; i<gd->n_data-1; i++) {
        if (gd->k_data[i] >= gd->k_data[i+1])
        COSMO_FAIL(cmd,
        "\n%s: LPS file must have k values in ascending order... exiting\n\n",
        routineName);
    }

    const double k_required_min = 0.001;
    const double k_required_max = 8.0;

    if (gd->k_data[0] > k_required_min ||
        gd->k_data[gd->n_data - 1] < k_required_max) {
        COSMO_FAIL(cmd,
            "\n%s: LPS file k range [%g, %g] does not cover required range [%g, %g]\n\n",
            routineName,
            gd->k_data[0], gd->k_data[gd->n_data - 1],
            k_required_min, k_required_max);
    }

    //E

    return SUCCESS;
}


// interpolation order 1
double interpolation1(double x, double xT[], double yT[], int n_data)
{
  int j,j1,j2,jm;
  double f;
  
    if (n_data < 2) return 0.;
    if (x < xT[0]) return 0.;
    if (x > xT[n_data-1]) return 0.;
    if (x == xT[0]) return yT[0];
    if (x == xT[n_data-1]) return yT[n_data-1];
    
  j1=0, j2=n_data-1, jm=(j1+j2)/2;
  for(;;){
    if(x>xT[jm]) j1=jm;
    else j2=jm;
    jm=(j1+j2)/2;

    if(j2-j1==1) break;
  }
  j=j1;

  f=(yT[j+1]-yT[j])/(xT[j+1]-xT[j]) * (x -xT[j])+ yT[j];
  
  return f;
}

// interpolation in log space
double interpolationlog(double x, double xT[], double yT[], int n_data)
{
  int j,j1,j2,jm;
  double f;

    if (n_data < 2) return 0.;
    if (x < xT[0]) return 0.;
    if (x > xT[n_data-1]) return 0.;
    if (x == xT[0]) return yT[0];
    if (x == xT[n_data-1]) return yT[n_data-1];
    
  j1=0, j2=n_data-1, jm=(j1+j2)/2;
  for(;;){
    if(x>xT[jm]) j1=jm;
    else j2=jm;
    jm=(j1+j2)/2;

    if(j2-j1==1) break;
  }
  j=j1;

  f=(log10(yT[j+1])-log10(yT[j]))/(log10(xT[j+1])
      -log10(xT[j]))*(log10(x)-log10(xT[j]))+log10(yT[j]);
  
  return pow(10.,f);
}

//E


//B Write
int write(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "write";
    char int_str[20];
    char str[MAXLENGTHOFFILES];
    
    //B initial checking
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (cmd->mMax < 0 || cmd->mMax == INT_MAX)
        COSMO_FAIL(cmd, "%s: invalid mMax = %d\n", routineName, cmd->mMax);

    if (iv.Nell < 1)
        COSMO_FAIL(cmd, "%s: invalid Nell = %d\n", routineName, iv.Nell);

    if (cmd->rootDir == NULL || cmd->prefix == NULL)
        COSMO_FAIL(cmd, "%s: output path strings are not initialized\n",
                   routineName);

    if (iv.ellT == NULL || iv.BmVectorsp == NULL)
        COSMO_FAIL(cmd, "%s: output tables are not allocated\n", routineName);

    for (int i = 0; i < iv.Nell; i++) {
        if (!isfinite(iv.ellT[i]) || iv.ellT[i] <= 0.0)
            COSMO_FAIL(cmd, "%s: invalid ellT[%d]=%g\n",
                       routineName, i, iv.ellT[i]);

        if (i > 0 && iv.ellT[i] <= iv.ellT[i - 1])
            COSMO_FAIL(cmd, "%s: ell grid must be increasing at i=%d: ell[i-1]=%g ell[i]=%g\n",
                       routineName, i, iv.ellT[i - 1], iv.ellT[i]);
    }

    for (int m = 0; m <= cmd->mMax; m++) {
        if (iv.BmVectorsp[m] == NULL)
            COSMO_FAIL(cmd, "%s: iv.BmVectorsp[%d] is not allocated\n",
                       routineName, m);
    }
    //E
    
    for (int m=0; m<cmd->mMax+1; m++) {
        FILE *fp;
        int nint = snprintf(int_str, sizeof(int_str), "%d", m);
        if (nint < 0 || (size_t)nint >= sizeof(int_str))
            COSMO_FAIL(cmd, "%s: moment index string too long\n", routineName);
        
        int nwrite = snprintf(str, sizeof(str), "%s/%sBmells_%s.txt",
                              cmd->rootDir, cmd->prefix, int_str);
        if (nwrite < 0 || (size_t)nwrite >= sizeof(str))
            COSMO_FAIL(cmd, "%s: Bmells output path too long\n", routineName);
        
        fp = fopen(str, "w+");
        if (fp == NULL)
            COSMO_FAIL(cmd, "%s: cannot open output file %s\n",
                       routineName, str);

        for (int i = 0; i < iv.Nell; i++) {
            for (int j = 0; j < iv.Nell; j++) {
                if (fprintf(fp, "%15e ", iv.BmVectorsp[m][i*iv.Nell + j]) < 0) {
                    fclose(fp);
                    COSMO_FAIL(cmd, "%s: write failed for %s\n", routineName, str);
                }
                if (j == iv.Nell-1 && i != iv.Nell-1) {
                    if (fprintf(fp, " \n") < 0) {
                        fclose(fp);
                        COSMO_FAIL(cmd, "%s: write failed for %s\n", routineName, str);
                    }
                }
            }
        }
        if (close_checked(&fp, cmd, routineName, str) == FAILURE)
            return FAILURE;
    }

    FILE *fp2;
    int nwrite = snprintf(str, sizeof(str),
                          "%s/%sellArray.txt",cmd->rootDir,cmd->prefix);
    if (nwrite < 0 || (size_t)nwrite >= sizeof(str))
        COSMO_FAIL(cmd, "%s: ellArray output path too long\n",
                   routineName);
    fp2 = fopen (str, "w+");
    if (fp2 == NULL)
        COSMO_FAIL(cmd, "%s: cannot open output file %s\n",
                   routineName, str);
    
    for (int i = 0; i < iv.Nell; i++) {
        if (fprintf(fp2, "%15e\n", iv.ellT[i]) < 0) {
            fclose(fp2);
            COSMO_FAIL(cmd, "%s: write failed for %s\n", routineName, str);
        }
    }
    
    if (close_checked(&fp2, cmd, routineName, str) == FAILURE)
        return FAILURE;
        
    FILE *fp3;
    nwrite = snprintf(str, sizeof(str),
                          "%s/%sinfo.txt",cmd->rootDir,cmd->prefix);
    if (nwrite < 0 || (size_t)nwrite >= sizeof(str))
        COSMO_FAIL(cmd, "%s: info output path too long\n",
                   routineName);
    fp3 = fopen (str, "w+");
    if (fp3 == NULL)
        COSMO_FAIL(cmd, "%s: cannot open output file %s\n",
                   routineName, str);

    if (fprintf(fp3, "Cosmological Parameters: \n") < 0 ||
        fprintf(fp3, "    Omega_m = %f\n", cmd->Omm) < 0 ||
        fprintf(fp3, "         ns = %f\n", cmd->ns) < 0 ||
        fprintf(fp3, "\n") < 0 ||
        fprintf(fp3, "Computing Bm(ell1,ell2) for %d x %d array  \n",
                iv.Nell, iv.Nell) < 0 ||
        fprintf(fp3, "    ellmin = %f, ellmax = %f  \n",
                iv.ellmin, iv.ellmax) < 0 ||
        fprintf(fp3, "    Number of moments = %d\n", cmd->mMax + 1) < 0 ||
        fprintf(fp3, "    Quadrature chi steps= %d\n", iv.chiQuadSteps) < 0) {
        fclose(fp3);
        COSMO_FAIL(cmd, "%s: write failed for %s\n", routineName, str);
    }
    
    if (close_checked(&fp3, cmd, routineName, str) == FAILURE)
        return FAILURE;

    //B
    size_t total = (size_t)iv.Nell * (size_t)iv.Nell;
    //E
    
    if (cmd->writevectors ==1) {
        for (int m=0; m<cmd->mMax+1; m++) {
            FILE *fp;
            int nint = snprintf(int_str, sizeof(int_str), "%d", m);
            if (nint < 0 || (size_t)nint >= sizeof(int_str))
                COSMO_FAIL(cmd, "%s: moment index string too long\n", routineName);
            
            
            int nwrite = snprintf(str, sizeof(str), "%s/%sBmellsVector_%s.txt",
                                  cmd->rootDir,cmd->prefix,int_str);
            if (nwrite < 0 || (size_t)nwrite >= sizeof(str))
                COSMO_FAIL(cmd, "%s: Bmells output path too long\n",
                           routineName);
            fp = fopen(str, "w+");
            if (fp == NULL)
                COSMO_FAIL(cmd, "%s: cannot open output file %s\n",
                           routineName, str);

            for (size_t ij = 0; ij < total; ij++) {
                if (fprintf(fp, "%e \n", iv.BmVectorsp[m][ij]) < 0) {
                    fclose(fp);
                    COSMO_FAIL(cmd, "%s: write failed for %s\n", routineName, str);
                }
            }
            
            if (close_checked(&fp, cmd, routineName, str) == FAILURE)
                return FAILURE;
        }
    }

    return SUCCESS;
}
//E

