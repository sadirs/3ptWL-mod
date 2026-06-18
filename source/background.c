// ============================================================================
//        1          2          3          4        ^ 5          6          7

#include <math.h>
#include "background.h"

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
 background routine:

 To be called in the MainLoop:
    background(&cmd, &gd, zBin);

 This routine is in charge of computing the background cosmology

 Arguments:
    * `cmd`: Input: structure cmdline_data pointer
    * `gd`: Input: structure global_data pointer
    * `zBin`: Input: double, redshift of bin
 Return (the error status):
    int SUCCESS or FAILURE
 */
int background(struct cmdline_data* cmd, struct  global_data* gd,
               double zBin)
{
    string routineName = "background";
    double chiMax;
    double H0 = 0.00033356409519815205; //H0 in h/Mpc
    int export=1;

    //B initial checking
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (gd->tables_allocated != TRUE)
        COSMO_FAIL(cmd,
                   "%s: runtime tables are not allocated; call StartRun_Common first\n",
                   routineName);

    if (gv.Nz < 2)
        COSMO_FAIL(cmd, "%s: gv.Nz must be at least 2, got %d\n",
                   routineName, gv.Nz);

    if (gv.zT == NULL || gv.chiOfzT == NULL || gv.DpT == NULL)
        COSMO_FAIL(cmd, "%s: background z/chi/Dplus tables are not allocated\n",
                   routineName);

    if (iv.chiT_chiint == NULL || iv.zT_chiint == NULL ||
        iv.DpT_chiint == NULL || iv.rsigma_chiint == NULL ||
        iv.neff_chiint == NULL || iv.q_chiint == NULL)
        COSMO_FAIL(cmd, "%s: chi quadrature tables are not allocated\n",
                   routineName);

    if (!isfinite(gd->Dpz0) || gd->Dpz0 == 0.0)
        COSMO_FAIL(cmd, "%s: invalid Dplus normalization Dpz0=%g; call Initial first\n",
                   routineName, gd->Dpz0);

    if (cmd->rootDir == NULL || cmd->prefix == NULL)
        COSMO_FAIL(cmd, "%s: output path strings are not initialized\n",
                   routineName);

    if (cmd->Wg == 1 &&
        (gv.chiforgLT == NULL || gv.gLT == NULL || gv.NstepsforgL < 2))
        COSMO_FAIL(cmd, "%s: Wg/gL tables are not allocated or invalid\n",
                   routineName);
    //E

    
    // NstepsforgL=1000 was initialized in startrun...

    if(cmd->Wg==1)
        if (compute_gL(cmd, gd) == FAILURE)
            return FAILURE;

    // Make z Array.
    for (int i=0;i<gv.Nz;i++)
        gv.zT[i] = gv.zMin + i*(gv.zMax-gv.zMin)/(gv.Nz-1);
    
    // Make chi(z) Array
    chiArray_all(cmd, gd, gv.chiOfzT, gv.zT);

    // Make Dp Array.
    for (int i = 0; i < gv.Nz; i++) {
        gv.DpT[i] = Dplusf(cmd, gd, gv.zT[i]) / gd->Dpz0;
        if (!isfinite(gv.DpT[i]))
            COSMO_FAIL(cmd, "%s: non-finite Dplus at z=%g\n",
                       routineName, gv.zT[i]);
    }

    gv.chiBin = chiOfz_func(zBin);

    if (chiMaxforInt(cmd, gd) == FAILURE)
        return FAILURE;

    if (ArraysforChiQuad(cmd, gd) == FAILURE)
        return FAILURE;

    if(export==1){
        FILE *fp;
        char str[MAXLENGTHOFFILES];
        int nwrite = snprintf(str, sizeof(str),
                              "%s/%sbackground_functions.txt",
                              cmd->rootDir,cmd->prefix);
        if (nwrite < 0 || (size_t)nwrite >= sizeof(str))
            COSMO_FAIL(cmd, "%s: output path too long\n", routineName);
        fp = fopen(str, "w+");
        if (fp == NULL)
            COSMO_FAIL(cmd, "%s: cannot open output file %s\n", routineName, str);
        
        if (fprintf(fp, "%15s  %15s  %15s  %15s  %15s  %15s  \n",
                    "chi[Mpc/h]", "z", "Dplus", "rsigma[Mpc/h]",
                    "neff", "q(chi)") < 0) {
            fclose(fp);
            COSMO_FAIL(cmd, "%s: write failed for %s\n", routineName, str);
        }

        for (int i = 0; i < iv.chiQuadSteps; i++) {
            if (fprintf(fp, "%15.15f  %15.15f  %15.15f  %15.15f  %15.15f  %15.15f  \n",
                        iv.chiT_chiint[i], iv.zT_chiint[i], iv.DpT_chiint[i],
                        iv.rsigma_chiint[i], iv.neff_chiint[i], iv.q_chiint[i]) < 0) {
                fclose(fp);
                COSMO_FAIL(cmd, "%s: write failed for %s\n", routineName, str);
            }
        }
        
        if (close_checked(&fp, cmd, routineName, str) == FAILURE)
            return FAILURE;
    }
    
    return SUCCESS;
};


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

double HoverH0(struct cmdline_data* cmd, struct  global_data* gd, double z)
{
    return sqrt( cmd->Omm*pow(1+z,3) + (1-cmd->Omm)*pow(1+z,3*(1+cmd->w)));
}

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

double gLDiracDelta(double chi) // This is gL for Wg a Dirac delta:
{
    if (chi>gv.chiBin){
         return 0;
    } else {
        return 1.- chi/gv.chiBin;
    }
}

double q(struct cmdline_data* cmd, struct  global_data* gd, double chi)
{
    double result, a;
    double H0 = 0.00033356409519815205; //H0 in h/Mpc
    a = aOfchi_func(chi);
    result = (3./2.)*cmd->Omm*H0*H0 * gL(cmd, gd, chi) * chi / a;
    return result;
}

int chiMaxforInt(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "chiMaxforInt";
    double qMV, verysmall;

    qMV = q(cmd, gd, gv.chiBin/2);
    verysmall = 0.0001;

    for (int i=0; i<gv.Nz; i++) {
        if (gv.chiOfzT[i] > gv.chiBin/2 &&
            q(cmd, gd, gv.chiOfzT[i]) < verysmall*qMV)
            break;

        iv.chiMaxInt = gv.chiOfzT[i];
    }

    if (cmd->Wg == 1) {
        double chiWgMax = gv.chiforgLT[gv.NstepsforgL - 1];

        if (!isfinite(chiWgMax) || chiWgMax <= 0.0) {
            COSMO_FAIL(cmd,
                "%s: invalid Wg/gL chi upper limit: chiWgMax=%g\n",
                routineName, chiWgMax);
        }

        if (iv.chiMaxInt > chiWgMax)
            iv.chiMaxInt = chiWgMax;
    }

    return SUCCESS;
}

int ArraysforChiQuad(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "ArraysforChiQuad";
    double chiv,zv,Dpv,rsigmav;
    double Deltachi;

    //B
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (iv.chiT_chiint == NULL ||
        iv.zT_chiint == NULL ||
        iv.DpT_chiint == NULL ||
        iv.rsigma_chiint == NULL ||
        iv.neff_chiint == NULL ||
        iv.q_chiint == NULL) {
        COSMO_FAIL(cmd, "%s: chi quadrature tables are not allocated\n",
                   routineName);
    }

    //B
    if (gv.Nz < 2 || gv.zT == NULL || gv.chiOfzT == NULL || gv.DpT == NULL) {
        COSMO_FAIL(cmd, "%s: background interpolation tables are not allocated\n",
                   routineName);
    }

    if (!isfinite(gv.chiBin) || gv.chiBin <= 0.0) {
        COSMO_FAIL(cmd, "%s: invalid chiBin=%g\n",
                   routineName, gv.chiBin);
    }

    if (cmd->Wg == 1 &&
        (gv.chiforgLT == NULL || gv.gLT == NULL || gv.NstepsforgL < 2)) {
        COSMO_FAIL(cmd, "%s: Wg/gL tables are not allocated or invalid\n",
                   routineName);
    }
    //E
    
    if (!isfinite(gd->Dpz0) || gd->Dpz0 == 0.0) {
        COSMO_FAIL(cmd, "%s: invalid Dplus normalization Dpz0=%g\n",
                   routineName, gd->Dpz0);
    }
    //E

    //B Use an adaptive value based on iv.chiMaxInt,
    //      because small valid `zbin` values can make chiMaxInt <= 10.
    //  use below modification. But, test it thoroughly
    iv.chiMinInt = 10;
    //E

    //B
    if (!isfinite(iv.chiMaxInt) || iv.chiMaxInt <= 0.0)
        COSMO_FAIL(cmd,
                   "%s: invalid chiMaxInt (%g)\n",
                   routineName, iv.chiMaxInt);

    //B if instead of iv.chiMinInt = 10 above
    //      use these three lines get quite different results
    // iv.chiMinInt = fmax(1.0e-6, 1.0e-4 * iv.chiMaxInt);
    // if (iv.chiMinInt >= iv.chiMaxInt)
    //        iv.chiMinInt = 0.5 * iv.chiMaxInt;
    //E

    if (iv.chiMaxInt <= iv.chiMinInt)
        COSMO_FAIL(cmd,
                   "%s: chiMaxInt (%g) must be greater than chiMinInt (%g)\n",
                   routineName, iv.chiMaxInt, iv.chiMinInt);
    //E
    
    if (iv.chiQuadSteps < 2) {
        COSMO_FAIL(cmd,
                   "\n%s: chiQuadSteps (%d) must be at least 2.\n",
                   routineName, iv.chiQuadSteps);
    }
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

        if (!isfinite(iv.chiT_chiint[i]) ||
            !isfinite(iv.zT_chiint[i]) ||
            !isfinite(iv.DpT_chiint[i]) ||
            !isfinite(iv.rsigma_chiint[i]) ||
            !isfinite(iv.neff_chiint[i]) ||
            !isfinite(iv.q_chiint[i])) {
            COSMO_FAIL(cmd,
                "%s: non-finite quadrature value at i=%d: chi=%g z=%g Dp=%g rsigma=%g neff=%g q=%g\n",
                routineName, i,
                iv.chiT_chiint[i], iv.zT_chiint[i], iv.DpT_chiint[i],
                iv.rsigma_chiint[i], iv.neff_chiint[i], iv.q_chiint[i]);
        }
    }

    return SUCCESS;
}

int ArraysforChiQuadLog(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "ArraysforChiQuadLog";
    double chiv, zv, Dpv, rsigmav;
    double Deltachi;

    //B
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (iv.chiT_chiint == NULL ||
        iv.zT_chiint == NULL ||
        iv.DpT_chiint == NULL ||
        iv.rsigma_chiint == NULL ||
        iv.neff_chiint == NULL ||
        iv.q_chiint == NULL) {
        COSMO_FAIL(cmd, "%s: chi quadrature tables are not allocated\n",
                   routineName);
    }

    if (gv.Nz < 2 || gv.zT == NULL || gv.chiOfzT == NULL || gv.DpT == NULL) {
        COSMO_FAIL(cmd, "%s: background interpolation tables are not allocated\n",
                   routineName);
    }

    if (!isfinite(gd->Dpz0) || gd->Dpz0 == 0.0) {
        COSMO_FAIL(cmd, "%s: invalid Dplus normalization Dpz0=%g\n",
                   routineName, gd->Dpz0);
    }
    //E

    //B see ArraysforChiQuad above to implement an adaptive value forchMinInt
    iv.chiMinInt = 0.001;
    //E

    //B
    if (!isfinite(iv.chiMaxInt) || iv.chiMaxInt <= 0.0)
        COSMO_FAIL(cmd,
                   "%s: invalid chiMaxInt (%g)\n",
                   routineName, iv.chiMaxInt);

    //    iv.chiMinInt = fmax(1.0e-6, 1.0e-4 * iv.chiMaxInt);
    //    if (iv.chiMinInt >= iv.chiMaxInt)
    //        iv.chiMinInt = 0.5 * iv.chiMaxInt;

    if (iv.chiMaxInt <= iv.chiMinInt)
        COSMO_FAIL(cmd,
                   "%s: chiMaxInt (%g) must be greater than chiMinInt (%g)\n",
                   routineName, iv.chiMaxInt, iv.chiMinInt);
    //E

    
    if (!isfinite(iv.chiMinInt) || !isfinite(iv.chiMaxInt) ||
        iv.chiMinInt <= 0.0 || iv.chiMaxInt <= iv.chiMinInt) {
        COSMO_FAIL(cmd,
                   "%s: invalid chi range: chiMinInt=%g chiMaxInt=%g\n",
                   routineName, iv.chiMinInt, iv.chiMaxInt);
    }

    if (iv.chiQuadSteps < 2) {
        COSMO_FAIL(cmd,
                   "%s: chiQuadSteps must be at least 2, got %d\n",
                   routineName, iv.chiQuadSteps);
    }

    Deltachi = log(iv.chiMaxInt / iv.chiMinInt) / (iv.chiQuadSteps - 1);

    if (!isfinite(Deltachi)) {
        COSMO_FAIL(cmd,
                   "%s: non-finite logarithmic chi step: chiMinInt=%g chiMaxInt=%g chiQuadSteps=%d\n",
                   routineName, iv.chiMinInt, iv.chiMaxInt, iv.chiQuadSteps);
    }

    for (int i = 0; i < iv.chiQuadSteps; i++) {
        chiv = iv.chiMinInt * exp(i * Deltachi);
        zv = zOfchi_func(chiv);
        Dpv = Dplusf(cmd, gd, zv) / gd->Dpz0;
        rsigmav = calcrsigma(cmd, gd, Dpv, 0.001, 8., 100);

        iv.chiT_chiint[i] = chiv;
        iv.zT_chiint[i] = zv;
        iv.DpT_chiint[i] = Dpv;
        iv.rsigma_chiint[i] = rsigmav;
        iv.neff_chiint[i] = n_eff_func(cmd, gd, rsigmav, Dpv, 0.001, 8., 100);
        iv.q_chiint[i] = q(cmd, gd, chiv);

        if (!isfinite(iv.chiT_chiint[i]) ||
            !isfinite(iv.zT_chiint[i]) ||
            !isfinite(iv.DpT_chiint[i]) ||
            !isfinite(iv.rsigma_chiint[i]) ||
            !isfinite(iv.neff_chiint[i]) ||
            !isfinite(iv.q_chiint[i])) {
            COSMO_FAIL(cmd,
                       "%s: non-finite quadrature value at i=%d: chi=%g z=%g Dp=%g rsigma=%g neff=%g q=%g\n",
                       routineName, i,
                       iv.chiT_chiint[i], iv.zT_chiint[i], iv.DpT_chiint[i],
                       iv.rsigma_chiint[i], iv.neff_chiint[i], iv.q_chiint[i]);
        }
    }

    return SUCCESS;
}


//B  Interpolation...
double chiOfz_func(double z)
{
    return interpolation1(z, gv.zT, gv.chiOfzT, gv.Nz);
}

double zOfchi_func(double chi)
{
    return interpolation1(chi, gv.chiOfzT, gv.zT, gv.Nz);
}

double aOfchi_func(double chi)
{
    return 1/(zOfchi_func(chi)+1.);
}

double DpOfchi_func(double chi)
{
    return interpolation1(chi, gv.chiOfzT, gv.DpT, gv.Nz);
}
//E

// Read input Wg(chi)
int read_inputWgchi(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "read_inputWgchi";
    FILE *fp;
    
    if (cmd == NULL || gd == NULL)
        return FAILURE;
    if (cmd->fWgchi == NULL) {
        COSMO_FAIL(cmd,
                   "\n%s: Wg(chi) file name is NULL\n\n",
                   routineName);
        return FAILURE;
    }
    gd->n_chi_data=0;
    
    fp = fopen(cmd->fWgchi, "r");

    if (fp == NULL && cmd->fWgchi[0] != '/') {
        char fallback[MAXLENGTHOFFILES];

        int nwrite = snprintf(fallback, sizeof(fallback),
                              "%s/tests/%s", __WLCFDIR__, cmd->fWgchi);

        if (nwrite >= 0 && (size_t)nwrite < sizeof(fallback)) {
            fp = fopen(fallback, "r");
        }
    }

    if (fp == NULL) {
        COSMO_FAIL(cmd,
                   "\n%s: Wg(chi) file can't be opened (%s)\n\n",
                   routineName, cmd->fWgchi);
    }
    
    double chi_data;
    double Wg_chi_data;
    int line;
    if(fp!=NULL){   // input: chi[Mpc/h]   Wg(chi)
        while ((line = fscanf(fp, "%lf %lf", &chi_data, &Wg_chi_data)) != EOF) {
            if (line != 2) {
                fclose(fp);
                COSMO_FAIL(cmd, "%s: Wgchi file must have two columns of values\n", routineName);
            }
            
            if (gd->n_chi_data >= n_chi_data_max) {
                fclose(fp);
                COSMO_FAIL(cmd,
                        "%s: n_chi_data_max should be larger than the number of data lines\n",
                        routineName);
            }

            gd->chi_data[gd->n_chi_data] = chi_data;
            gd->Wg_chi_data[gd->n_chi_data] = Wg_chi_data;
            gd->n_chi_data++;
        }
    fclose(fp);
    } else
        COSMO_FAIL(cmd, "\n%s: Wg file can't be opened (%s)\n\n",
                   routineName, cmd->fWgchi);

    if (gd->n_chi_data < 2)
        COSMO_FAIL(cmd, "%s: Wgchi file must have at least two rows of values... exiting\n\n",
                   routineName);

    for (int i=0; i<gd->n_chi_data; i++) {
        if (!isfinite(gd->chi_data[i]))
            COSMO_FAIL(cmd, "%s: Wgchi file must have finite chi values... exiting\n\n",
                       routineName);
        if (!isfinite(gd->Wg_chi_data[i]))
            COSMO_FAIL(cmd, "%s: Wgchi file must have finite Wg values... exiting\n\n",
                       routineName);
        if (gd->chi_data[i] <= 0.)
            COSMO_FAIL(cmd, "%s: Wgchi file must have chi values positive... exiting\n\n",
                       routineName);
    }

//B  if Wg_chi_data is allowed to be zero or negative physically,
//      leaving that positivity check commented is fine.
//      If it must be finite, add isfinite() checks for both columns
//
//    for (int i=0; i<gd->chi_data; i++) {
//        if (gd->Wg_chi_data[i] <= 0.)
//            error("\nWg file must have positive Wg values... %s\n\n",
//                  "exiting");
//    }
//E
    for (int i=0; i<gd->n_chi_data-1; i++) {
        if (gd->chi_data[i] >= gd->chi_data[i+1])
            COSMO_FAIL(cmd,
                   "%s: Wgchi file must have chi values in ascending order... exiting\n\n",
                    routineName);
    }

    return SUCCESS;
}


// Input Wg of chi
double Wg_func(struct cmdline_data* cmd, struct  global_data* gd,
               double chi)   // Wg(chi) interpolated
{
    return interpolation1(chi, gd->chi_data, gd->Wg_chi_data, gd->n_chi_data);
}

double gL_func(double chi)   // gL(chi) interpolated
{
    return interpolation1(chi, gv.chiforgLT, gv.gLT, gv.NstepsforgL);
}

int compute_gL(struct cmdline_data* cmd, struct global_data* gd)
{
    string routineName = "compute_gL";

    //B
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (gv.NstepsforgL < 2 || gv.chiforgLT == NULL || gv.gLT == NULL) {
        COSMO_FAIL(cmd, "%s: Wg/gL tables are not allocated or invalid: NstepsforgL=%d\n",
                   routineName, gv.NstepsforgL);
    }
    //E
    
    if (read_inputWgchi(cmd, gd) == FAILURE)
        return FAILURE;

    double chiminint, chimaxint;
    
    chiminint=gd->chi_data[0];
    chimaxint=gd->chi_data[gd->n_chi_data-1];
    
    if(cmd->verbose>1)
        printf("\nInput file Wg with %d lines. chiMin=%f, chiMax=%f \n\n",
               gd->n_chi_data,chiminint,chimaxint);
    
    int Nsteps=gv.NstepsforgL;

    for (int i=0; i<Nsteps; i++) {
        gv.chiforgLT[i] = chiminint + i*(chimaxint-chiminint)/(Nsteps-1);

        if (!isfinite(gv.chiforgLT[i])) {
            COSMO_FAIL(cmd, "%s: non-finite chiforgLT[%d]=%g\n",
                       routineName, i, gv.chiforgLT[i]);
        }
    }
        
    double gLA, gLB, gLp, chiv, chip, chi, deltachi, chiprev;
    
    for (int i=0;i<Nsteps;i++){
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
        
        if (!isfinite(gv.gLT[i])) {
            COSMO_FAIL(cmd, "%s: non-finite gLT[%d]=%g at chi=%g\n",
                       routineName, i, gv.gLT[i], chi);
        }
    }

    return SUCCESS;
}

