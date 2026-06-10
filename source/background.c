//==========================================================================
//        1          2          3          4          5          6          7
/*
============================================================================
NAME: background.c                           [wlcf]
Written by: S. Aviles et al.
Starting date: February 2026
Purpose: Background cosmology routines
Language: C
============================================================================
*/

#include "background.h"


/*
background routine:
*/
int background(struct cmdline_data* cmd, struct  global_data* gd,
               double zBin)
{
    double chiMax;
    double H0 = 0.00033356409519815205; // H0: Hubble constant in units of h/Mpc
    int export=1;

    gv.NstepsforgL=1000;
    if(cmd->Wg==1) compute_gL(cmd, gd);

    
    gv.zMax = 3.*zBin;
    // gv.zMax: maximum redshift used to tabulate background quantities
    gv.zMin = 0.0;
    // gv.zMax: maximum redshift used to tabulate background quantities
    gv.Nz   = 200;
    
    gv.zT        = malloc(gv.Nz * sizeof(double));
    gv.chiOfzT   = malloc(gv.Nz * sizeof(double));
    gv.DpT       = malloc(gv.Nz * sizeof(double));

    // Make z Array.
    for    (int i=0;i<gv.Nz;i++) gv.zT[i] = gv.zMin + i*(gv.zMax-gv.zMin)/(gv.Nz-1);
    
    // Make chi(z) Array
    chiArray_all(cmd, gd, gv.chiOfzT, gv.zT);

    // Make Dp Array.
    for(int i=0;i<gv.Nz;i++) gv.DpT[i] = Dplusf(cmd, gd, gv.zT[i])/ gd->Dpz0;
    // gv.DpT[i]: linear growth factor normalized to its value at z = 0
    // gd->Dpz0 : linear growth factor evaluated at redshift z = 0
    gv.chiBin = chiOfz_func(zBin);


    //~ printf("\n");
    //~ for(int i=0;i<11;i++) printf("{%d, %f}, ", -5+i, chiOfz_func(zBin - 5. * 0.00001 + i * 0.00001));
    //~ printf("\n\n");
    
    chiMaxforInt(cmd, gd);
    ArraysforChiQuad(cmd, gd);
    //~ ArraysforChiQuadLog();

    if(export==1){
        FILE *fp;
        char str[100];

        sprintf(str,"%s/%sbackground_functions.txt",cmd->path_Bells,cmd->prefix);
        fp = fopen (str, "w+");

        fprintf(fp, "%15s  %15s  %15s  %15s  %15s  %15s  \n",
                "chi[Mpc/h]", "z", "Dplus", "rsigma[Mpc/h]",
                "neff", "q(chi)");

        for(int i=0;i<iv.chiQuadSteps;i++){
            fprintf(fp, "%15.15f  %15.15f  %15.15f  %15.15f  %15.15f  %15.15f  \n",
            iv.chiT_chiint[i], iv.zT_chiint[i],iv.DpT_chiint[i],
            iv.rsigma_chiint[i], iv.neff_chiint[i], iv.q_chiint[i]
            );
        }
        fclose(fp);
    }
    
    //printf("\n%5.15f \n\n",iv.chiMaxInt-gv.chiBin);
    
    return SUCCESS;
};

/*
chi(z) array construction routine:
*/
int chiArray_all(struct cmdline_data* cmd, struct  global_data* gd,
                  double chiOfzT[], double zT[])
{
    double z,zprev,deltaz;
    double H0 = 0.00033356409519815205; //H0 in h/Mpc
    
    deltaz=zT[1]-zT[0];
    
    double xp=0.0, xA=0.0, xB=0.0;
    
    z=zT[0];
    xA = 1./HoverH0(cmd, gd, z)/H0;
    chiOfzT[0]=xA*z;
    zprev = z;
    
    for (int i=1;i<gv.Nz;i++){
        z=zT[i];
        xB = 1./HoverH0(cmd, gd, z)/H0;
        xp += 0.5*(xA + xB) * deltaz;
        chiOfzT[i] = xp;
        xA=xB;
        zprev = z;
    }
    
    return SUCCESS;
}
/*
Dimensionless Hubble function:

This routine returns H(z)/H0 for a flat wCDM cosmology.

*/
double HoverH0(struct cmdline_data* cmd, struct  global_data* gd, double z)
{
    return sqrt( cmd->Omm*pow(1+z,3) + (1-cmd->Omm)*pow(1+z,3*(1+cmd->w)));
}
/*
Lensing kernel selector:
*/
double gL(struct cmdline_data* cmd, struct  global_data* gd, double chi)
{
    
    if (cmd->Wg==0){
        return gLDiracDelta(chi);
    } else if (cmd->Wg==1) {
        return gL_func(chi);
    } else {
        return 0;
    }
}
/*
Dirac-delta lensing kernel:
*/
double gLDiracDelta(double chi) // This is gL for Wg a Dirac delta:
{
    if (chi>gv.chiBin){
         return 0;
    } else {
        return 1.- chi/gv.chiBin;
    }
}
/*
Lensing weight function q(chi):
*/
double q(struct cmdline_data* cmd, struct  global_data* gd, double chi)
{
    double result, a;
    double H0 = 0.00033356409519815205; //H0 in h/Mpc
    a = aOfchi_func(chi);
    result = (3./2.)*cmd->Omm*H0*H0 * gL(cmd, gd, chi) * chi / a;
    return result;
}

/*
Maximum chi for integration routine:
*/
int chiMaxforInt(struct cmdline_data* cmd, struct  global_data* gd)
{
    double qMV, qr, zMax, verysmall;
    qMV = q(cmd, gd, gv.chiBin/2);
    verysmall = 0.0001;
    for    (int i=0;i<gv.Nz;i++){
        if ( gv.chiOfzT[i] > gv.chiBin/2 && q(cmd, gd, gv.chiOfzT[i])< verysmall* qMV) break;
        iv.chiMaxInt = gv.chiOfzT[i];
    }
    //~ printf("%f, ",iv.chiMaxInt);
    
    return SUCCESS;
}

/*
Maximum chi for integration routine:
*/
int ArraysforChiQuad(struct cmdline_data* cmd, struct  global_data* gd)
{
    double chiv,zv,Dpv,rsigmav,neffv;
    double Deltachi;
    
    iv.chiQuadSteps=cmd->chiQuadSteps;
    //~ iv.chiMinInt = 0.001;
    iv.chiMinInt = 10;
    iv.zT_chiint     = malloc(iv.chiQuadSteps * sizeof(double));
    iv.chiT_chiint   = malloc(iv.chiQuadSteps * sizeof(double));
    iv.DpT_chiint    = malloc(iv.chiQuadSteps * sizeof(double));
    iv.rsigma_chiint = malloc(iv.chiQuadSteps * sizeof(double));
    iv.neff_chiint   = malloc(iv.chiQuadSteps * sizeof(double));
    iv.q_chiint      = malloc(iv.chiQuadSteps * sizeof(double));
    
    Deltachi = (iv.chiMaxInt - iv.chiMinInt) / (iv.chiQuadSteps-1);
    
    
    for(int i=0;i<iv.chiQuadSteps;i++){
        chiv = iv.chiMinInt + i * Deltachi;
        zv = zOfchi_func (chiv);
        Dpv= Dplusf(cmd, gd, zv) / gd->Dpz0;
        rsigmav = calcrsigma(cmd, gd, Dpv, 0.001,8.,100);
        iv.chiT_chiint[i]   = chiv;
        iv.zT_chiint[i]     = zv;
        iv.DpT_chiint[i]    = Dpv;
        iv.rsigma_chiint[i] = rsigmav;
        iv.neff_chiint[i]   = n_eff_func(cmd, gd, rsigmav,Dpv,0.001,8.,100);
        iv.q_chiint[i]      = q(cmd, gd, chiv);
    }

    return SUCCESS;
}

/*
Logarithmic chi-quadrature array construction:
*/
int ArraysforChiQuadLog(struct cmdline_data* cmd, struct  global_data* gd)
{
    
    double chiv,zv,Dpv,rsigmav,neffv;
    double Deltachi;
        
    iv.chiQuadSteps=cmd->chiQuadSteps;
    iv.chiMinInt = 0.001;
    iv.zT_chiint     = malloc(iv.chiQuadSteps * sizeof(double));
    iv.chiT_chiint   = malloc(iv.chiQuadSteps * sizeof(double));
    iv.DpT_chiint    = malloc(iv.chiQuadSteps * sizeof(double));
    iv.rsigma_chiint = malloc(iv.chiQuadSteps * sizeof(double));
    iv.neff_chiint   = malloc(iv.chiQuadSteps * sizeof(double));
    iv.q_chiint      = malloc(iv.chiQuadSteps * sizeof(double));
    
    Deltachi = log(iv.chiMaxInt/iv.chiMinInt)/(iv.chiQuadSteps-1);
    
    for(int i=0;i<iv.chiQuadSteps;i++){
        chiv = iv.chiMinInt* exp(i*Deltachi);
        zv = zOfchi_func (chiv);
        Dpv= Dplusf(cmd, gd, zv) / gd->Dpz0;
        rsigmav = calcrsigma(cmd, gd, Dpv, 0.001,8.,100);
        iv.chiT_chiint[i]   = chiv;
        iv.zT_chiint[i]     = zv;
        iv.DpT_chiint[i]    = Dpv;
        iv.rsigma_chiint[i] = rsigmav;
        iv.neff_chiint[i]   = n_eff_func(cmd, gd, rsigmav,Dpv,0.001,8.,100);
        iv.q_chiint[i]      = q(cmd, gd, chiv);
    }

    return SUCCESS;
}


//B  Interpolation...

/*
chi(z) interpolation routine:
*/
double chiOfz_func(double z)
{
    return interpolation1(z, gv.zT, gv.chiOfzT, gv.Nz);
}
/*
z(chi) interpolation routine:
*/
double zOfchi_func(double chi)
{
    return interpolation1(chi, gv.chiOfzT, gv.zT, gv.Nz);
}
/*
Scale factor interpolation routine:
*/
double aOfchi_func(double chi)
{
    return 1/(zOfchi_func(chi)+1.);
}
/*
Growth factor interpolation routine:
*/
double DpOfchi_func(double chi)
{
    return interpolation1(chi, gv.chiOfzT, gv.DpT, gv.Nz);
}
//E

/*
Read input Wg(chi) routine:
*/
// Read input Wg(chi)
int read_inputWgchi(struct cmdline_data* cmd, struct  global_data* gd)
{
    FILE *fp;
    gd->n_chi_data=0;
    fp=fopen(cmd->fWgchi,"r");   // Wg(chi) table  c#1: chi, c#2: Wg
    
    if (NULL == fp) {
        printf("\n\nWg file can't be opened \n\n");
    }
    
    if(fp!=NULL){   // input: chi[Mpc/h]   Wg(chi)
        while(fscanf(fp, "%lf %lf", &gd->chi_data[gd->n_chi_data], &gd->Wg_chi_data[gd->n_chi_data])!=EOF){
            gd->n_chi_data++;
            if(gd->n_data>n_chi_data_max) printf("Wg: n_data_max should be larger than the number of data lines \n");
        }
    fclose(fp);
    }

    return SUCCESS;
}


/*
Interpolated Wg(chi) routine:
*/
// Input Wg of chi
double Wg_func(struct cmdline_data* cmd, struct  global_data* gd,
               double chi)   // Wg(chi) interpolated
{
    return interpolation1(chi, gd->chi_data, gd->Wg_chi_data, gd->n_chi_data);
}
/*
Interpolated gL(chi) routine:
*/
double gL_func(double chi)   // gL(chi) interpolated
{
    return interpolation1(chi, gv.chiforgLT, gv.gLT, gv.NstepsforgL);
}
/*
Compute gL(chi) routine:
*/
int compute_gL(struct cmdline_data* cmd, struct  global_data* gd)
{
    //double chiMaxInt=2000.
    read_inputWgchi(cmd, gd);
    //~ for(int i=0;i<gd.n_chi_data;i++){
    //~ printf("%f, %f \n",gd.chi_data[i],gd.Wg_chi_data[i]);
    //~ }
    double chiminint, chimaxint;
    
    chiminint=gd->chi_data[0];
    chimaxint=gd->chi_data[gd->n_chi_data-1];
    
    if(cmd->chatty>1) printf("\nInput file Wg with %d lines. chiMin=%f, chiMax=%f \n\n",
                            gd->n_chi_data,chiminint,chimaxint);
    
    int Nsteps=gv.NstepsforgL;
    
    gv.chiforgLT = malloc(Nsteps * sizeof(double));
    gv.gLT       = malloc(Nsteps * sizeof(double));
    
    for    (int i=0;i<Nsteps;i++) gv.chiforgLT[i] = chiminint + i*(chimaxint-chiminint)/(Nsteps-1);
    //~ for    (int i=0;i<Nsteps;i++) printf("%f, ",gv.chiforgLT[i]);
    
    //~ printf("\n %f \n",Wg_func(1350.5));
    
    double gLA, gLB, gLp, chiv, chip, chi, deltachi, chiprev;
    
    for    (int i=0;i<Nsteps;i++){
        
        chi= gv.chiforgLT[i];
        gLp=0; gLA=0; gLB=0;
        
        chiprev=chi;
        
        for( int j=i; j<Nsteps; j++){
            
            chiv=gv.chiforgLT[j];
            deltachi=chiv-chiprev;
            gLB=(chiv-chi)/chiv * Wg_func(cmd, gd, chiv);
            gLp=gLp + (gLA+gLB)/2.*deltachi;
            gLA=gLB;
            chiprev=chiv;
        }
        
        gv.gLT[i]=gLp;
        
    }
    
    //~ for (int i=0;i<Nsteps;i++) printf("{%f, %f},",gv.chiforgLT[i],gv.gLT[i]);
    return SUCCESS;
}

