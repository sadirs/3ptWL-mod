//==========================================================================
//        1          2          3          4        ^ 5          6          7

/*
============================================================================
NAME: zetam.c                                [wlcf]
Written by: S. Aviles et al.
Starting date: February 2026
Purpose: Routines for computing zeta_m from Bm(ell1,ell2)
Language: C
Major revision:
*/
//==========================================================================

#include "zetam.h"

#include <math.h>
#include <complex.h>
#include <string.h>

#include <time.h>
#include <fftw3.h>
/*
Allocation routine for zeta_m arrays:
*/
void allocate_zv(void)
{
    //~ double *r1, *r2, **result;
    zv.r1 = malloc(iv.Nell * sizeof(double));
    zv.r2 = malloc(iv.Nell * sizeof(double));
    zv.result = malloc(iv.Nell * sizeof(double *));
    for(int i=0; i<iv.Nell; i++) {
        zv.result[i] = malloc(iv.Nell * sizeof(double));
    }
    
}
/*
zeta_m computation routine:
*/
int get_zetam(struct cmdline_data* cmd,
               struct  global_data* gd)
{
    char outfilename_thetavector[100];

    int multipole;

    double dlnell = log(iv.ellT[1]/iv.ellT[0]);
    
    int Nell=iv.Nell;

    long size = Nell*Nell;
    long n_data, ndatamax;
    double mat[Nell][Nell];
        
    config config_m;
        config_m.l1 = 0;
        config_m.l2 = 0;
        config_m.nu1 = 1.01;
        config_m.nu2 = 1.01;
        config_m.c_window_width = 0.25;
        config_m.sys_Flag = 0;

    double **Bmell1ell2;
    double smooth_dlnr = dlnell;
    int dimension = 2;

    Bmell1ell2 = malloc(iv.Nell * sizeof(double *));
    for(int i=0; i<iv.Nell; i++)
        Bmell1ell2[i] = malloc(iv.Nell * sizeof(double));

    clock_t start = clock();
    
    char int_str[20];
    char str[100];

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
        
        two_Bessel_binave(iv.ellT, iv.ellT, Bmell1ell2,
                          iv.Nell, iv.Nell,
                          &config_m, smooth_dlnr, dimension,
                          zv.r1, zv.r2, zv.result);

        if(multipole==0){
            sprintf(outfilename_thetavector,"%s/%stheta_array.txt",
                    cmd->path_Bells,cmd->prefix);
            FILE *OUTtheta = fopen(outfilename_thetavector, "w");
            for(int i=0; i<iv.Nell; i++)
                fprintf(OUTtheta, "%lg\n", zv.r1[i]);
            fclose(OUTtheta);
        }
    
        FILE *fp;
        sprintf(int_str, "%d", multipole);
        sprintf(str,"%s/%szetam%s.txt",cmd->path_Bells,
                cmd->prefix,int_str);
        fp = fopen (str, "w+");
        for(int i=0; i<iv.Nell; i++) {
            for(int j=0; j<iv.Nell; j++){
             fprintf(fp, "%lg  ", zv.result[i][j]);
            }
            if(i!=iv.Nell-1) fprintf(fp, "\n");
        }
        //~ for(int ij=0; ij<iv.Nell*iv.Nell ; ij++){
            //~ fprintf(fp, "%e \n", iv.BmVectorsp[m][ij]);
        //~ }
        fclose (fp);
    };

    clock_t end = clock();
    float seconds = (float)(end - start) / CLOCKS_PER_SEC;
    printf("time zetams: %f \n", seconds);

    //~ free(zv.r1);
    //~ free(zv.r2);
    //~ for (int i = 0; i < Nell; ++i){
        //~ free(Bmell1ell2[i]);
        //~ free(zv.result[i]);
    //~ }
    //~ free(Bm0ell1ell2);
    //~ free(zv.result);


    return SUCCESS;
}




/////////////////////////////////////////////
/* free variables */
/////////////////////////////////////////////

/*
Free allocated variables routine:
*/

int free_variables(struct cmdline_data* cmd,
                        struct  global_data* gd)
{
            for (int m=0; m<cmd->mMax+1; m++){
                free(iv.BmVectorsp[m]);
                free(iv.BmVectors [m]);
            }
            free(iv.BmVectorsp);
            free(iv.BmVectors );

    return SUCCESS;
}


