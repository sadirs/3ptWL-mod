// ============================================================================
//        1          2          3          4        ^ 5          6          7

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


int testTakahashiBispectrum(struct cmdline_data* cmd,
                            struct  global_data* gd)
{
    double cpumiddle;
    double k1,k2,k3,test;
    
    k1=0.1;
    k2=0.2;
    k3=0.3;

    printf(" \n Tests Takahashi bispectrum: \n\n");
    

    // TEST input PS...
    printf("%d \n", gd->n_data);
    printf("%lf %lf \n", gd->k_data[gd->n_data-5], gd->pkz0_data[gd->n_data-5]);

    printf(" \n");

    
    //  TEST Dplus, rsigma, neff
    cpumiddle = CPUTIME;
    gv.Dp=Dplusf(cmd, gd, cmd->z)/gd->Dpz0;
    printf("z=%lf , Dplus(z)= %lf, time= %lf \n",
           gv.z, gv.Dp, CPUTIME - cpumiddle);
    cpumiddle = CPUTIME;
    gv.r_sigma = calcrsigma(cmd, gd, gv.Dp, 0.001,8.,100);
    printf("rsigma= %lf, time= %lf \n", gv.r_sigma, CPUTIME - cpumiddle);
    cpumiddle = CPUTIME;
    gv.n_eff=
        n_eff_func(cmd, gd, gv.r_sigma,gv.Dp,0.001,8.,100);  // n_eff in Eq.(B2)
    printf("n_eff=%lf , time= %f15 \n", gv.n_eff, CPUTIME - cpumiddle);
 
    printf(" \n");

    //  TEST sigmas
    cpumiddle = CPUTIME;
    test = sigmam(cmd, gd, 8,2);
    printf("sigma8taka = %lf, time = %lf15 \n", test, CPUTIME - cpumiddle);
    
    cpumiddle = CPUTIME;
    test = sigmaRGaussian1stDeriv(cmd, gd, 8,0.001,8.,100);
    printf("sigma8taka = %lf, time = %lf15 \n", test, CPUTIME - cpumiddle);

    printf(" \n");
    //  TEST F2

    cpumiddle = CPUTIME;
    test= F2(cmd, gd, k1, k2, k3, gv.z, gv.Dp, gv.r_sigma);
    printf("F2tree=%lf,  F2=%lf , F2 time= %f15 \n",
           F2_tree(k1,k2,k3), test, CPUTIME - cpumiddle);
    
    
    //  TEST bispectrum tree
    cpumiddle = CPUTIME;
    test= Bispec_tree(cmd, gd, k1, k2, k3, gv.Dp);
    printf("Btree=%lf , Btree time= %f15 \n", test, CPUTIME - cpumiddle);

    // TEST Bispectrum Takahashi
    cpumiddle = CPUTIME;
    for(int i=0;i<1000;i++){
    test=Bispec_Takahashi(cmd, gd, k1,k2,k3,gv.z,gv.Dp,gv.r_sigma,gv.n_eff);
    };
    printf("B Taka=%lf , time= %f15 \n", test, CPUTIME - cpumiddle);

    printf("\n end tests \n\n");
    
    return SUCCESS;
}

