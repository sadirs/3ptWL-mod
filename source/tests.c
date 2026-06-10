//==========================================================================
//        1          2          3          4        ^ 5          6          7

/*
============================================================================
NAME: tests.c                                [wlcf]
Written by: S. Aviles et al.
Starting date: February 2026
Purpose: Test routines for matrix/vector conversion and bispectrum checks
Language: C
*/
//==========================================================================

#include "functions.h"
#include "procedures.h"


void vectortomatrix(void);

void tests2()
{

};

void vectortomatrix()
{
    // TEST DO MATRICES TO VECTORS//
    double *ptest;
    int sizetest = 4;
    ptest = malloc(sizetest*sizetest * sizeof(double));
    double mat[sizetest][sizetest];
    double val;
    
    for(int i=0; i<sizetest ; i++){
    for(int j=i; j<sizetest ; j++){
        val = i*sizetest+j;
        ptest[i*sizetest+j] = val;
        if(j!=i) ptest[j*sizetest+i] = val;
    }
    }
    printf("\n");
    
    
    for(int i=0; i<sizetest ; i++){
    for(int j=0; j<sizetest ; j++){
        mat[i][j] = ptest[j*sizetest+i];
    }
    }
    
    for(int i=0; i<sizetest ; i++){
    for(int j=0; j<sizetest ; j++){
        printf("%f, ", mat[i][j]);
        if(j==sizetest-1) printf("\n");
    }
    }
    
    printf("\n");
}

/*
Test Takahashi bispectrum routine:

This routine performs diagnostic tests for several cosmological
quantities and bispectrum-related functions.

Tested quantities include:
    - linear growth factor
    - nonlinear scale
    - effective spectral index
    - sigma-related functions
    - F2 kernel
    - tree-level bispectrum
    - Takahashi bispectrum

Arguments:
    'cmd': Input: structure cmdline_data pointer
    'gd' : Input: structure global_data pointer

Return (the error status):
    int SUCCESS or FAILURE
*/


int testTakahashiBispectrum(struct cmdline_data* cmd,
                            struct  global_data* gd)
{
    double k1,k2,k3,test;
    // k1,k2,k3: Fourier wave numbers forming the bispectrum triangle   
    k1=0.1;
    k2=0.2;
    k3=0.3;
  //  printf("F2tree = %lf \n", F2_tree(k1,k2,k3));

    printf(" \n Tests Takahashi bispectrum: \n\n");
    

    // TEST input PS...
    printf("%d \n", gd->n_data);
    printf("%lf %lf \n", gd->k_data[gd->n_data-5], gd->pkz0_data[gd->n_data-5]);

    printf(" \n");

    
    //  TEST Dplus, rsigma, neff
    gv.time=clock();
    gv.Dp=Dplusf(cmd, gd, cmd->z)/gd->Dpz0;
    // gv.Dp   : linear growth factor normalized to its value at z = 0
    // cmd->z  : input redshift
    // gd->Dpz0: linear growth factor evaluated at redshift z = 0
    printf("z=%lf , Dplus(z)= %lf, time= %lf \n", gv.z, gv.Dp, (double)(clock() - gv.time) / CLOCKS_PER_SEC );
    gv.time=clock();
    gv.r_sigma = calcrsigma(cmd, gd, gv.Dp, 0.001,8.,100);
    printf("rsigma= %lf, time= %lf \n", gv.r_sigma, (double)(clock() - gv.time) / CLOCKS_PER_SEC );
    gv.time=clock();
    gv.n_eff= n_eff_func(cmd, gd, gv.r_sigma,gv.Dp,0.001,8.,100);   // n_eff in Eq.(B2)
    printf("n_eff=%lf , time= %f15 \n", gv.n_eff, (double)(clock() - gv.time) / CLOCKS_PER_SEC  );
 
    printf(" \n");

    //  TEST sigmas
    gv.time=clock();
    test = sigmam(cmd, gd, 8,2);
    // test: derivative of the Gaussian-smoothed variance at 8 Mpc/h
    printf("sigma8taka = %lf, time = %lf15 \n", test, (double)(clock() - gv.time) / CLOCKS_PER_SEC );
    
    gv.time=clock();
    test = sigmaRGaussian1stDeriv(cmd, gd, 8,0.001,8.,100);
    // test: derivative of the Gaussian-smoothed variance at 8 Mpc/h
    printf("sigma8taka = %lf, time = %lf15 \n", test, (double)(clock() - gv.time) / CLOCKS_PER_SEC );

    printf(" \n");
    //  TEST F2

    gv.time=clock();
    test= F2(cmd, gd, k1, k2, k3, gv.z, gv.Dp, gv.r_sigma);
    // test      : second-order mode-coupling kernel
    // gv.z      : redshift
    // gv.Dp     : linear growth factor
    // gv.r_sigma: nonlinear scale parameter
    printf("F2tree=%lf,  F2=%lf , F2 time= %f15 \n", F2_tree(k1,k2,k3), test, (double)(clock() - gv.time) / CLOCKS_PER_SEC  );
    
    
    ////////  TEST bispectrum tree ///////
    gv.time=clock();
    test= Bispec_tree(cmd, gd, k1, k2, k3, gv.Dp);
    printf("Btree=%lf , Btree time= %f15 \n", test, (double)(clock() - gv.time) / CLOCKS_PER_SEC  );

    // TEST Bispectrum Takahashi
    gv.time=clock();
    for(int i=0;i<1000;i++){
    test=Bispec_Takahashi(cmd, gd, k1,k2,k3,gv.z,gv.Dp,gv.r_sigma,gv.n_eff);
    // test      : Takahashi matter bispectrum
    // gv.z      : redshift
    // gv.Dp     : linear growth factor
    // gv.r_sigma: nonlinear scale parameter
    // gv.n_eff  : effective spectral index
    };
    printf("B Taka=%lf , time= %f15 \n", test, (double)(clock() - gv.time) / CLOCKS_PER_SEC  );

    printf("\n end tests \n\n");
    
    return SUCCESS;
}

