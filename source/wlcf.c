/*=============================================================================
 NAME: wlcf.c                [wlcf]
 Written by: A. Aviles et al.
 Starting date: february 2026
 Purpose: Main routine
 Language: C
 Major revision:
// ============================================================================*/
//        1          2          3          4        ^ 5          6          7

#include "globaldefs.h"
#include "procedures.h"
#include "background.h"
#include "zetam.h"

/*
 MainLoop routine:

 To be called in main:
    MainLoop(&cmd, &gd);

 This routine is in charge of the computational flow:
    -background
    -BmKspace
    -Bmell
    -get_zetam

 Arguments:
    * `cmd`: Input: structure cmdline_data pointer
    * `gd`: Input: structure global_data pointer
 Return (the error status):
    int SUCCESS or FAILURE
 */
/*int MainLoop(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "MainLoop";
    double cpustart = CPUTIME;
    double cpumiddle;
*/
    
//B
int MainLoop(struct cmdline_data* cmd, struct global_data* gd)
{
    string routineName = "MainLoop";
    double cpustart = CPUTIME;
    double cpumiddle;

    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (gd->tables_allocated != TRUE)
        COSMO_FAIL(cmd, "%s: runtime tables are not allocated; call StartRun_Common first\n",
                   routineName);

    if (!isfinite(gd->Dpz0) || gd->Dpz0 == 0.0)
        COSMO_FAIL(cmd, "%s: invalid Dplus normalization Dpz0=%g; call Initial first\n",
                   routineName, gd->Dpz0);
//E

    cpumiddle = CPUTIME;
    if (background(cmd, gd, cmd->zbin) == FAILURE)
        return FAILURE;

	if(cmd->verbose>1){
		printf("time evaluating background: %lf s\n", CPUTIME - cpumiddle);
		printf("iv.chiMaxInt = %f Mpc/h\n", iv.chiMaxInt);
		printf("gv.chiBin    = %f Mpc/h\n", gv.chiBin);
	 }

    cpumiddle = CPUTIME;
	gv.z=cmd->zbin;
    gv.Dp = Dplusf(cmd, gd, gv.z) / gd->Dpz0;
    if (!isfinite(gv.Dp))
        COSMO_FAIL(cmd, "%s: non-finite Dplus at z=%g\n", routineName, gv.z);
    
    gv.r_sigma = calcrsigma(cmd, gd, gv.Dp, 0.001, 8., 100);
    if (!isfinite(gv.r_sigma) || gv.r_sigma <= 0.0)
        COSMO_FAIL(cmd, "%s: invalid r_sigma=%g\n", routineName, gv.r_sigma);

    gv.n_eff = n_eff_func(cmd, gd, gv.r_sigma, gv.Dp, 0.001, 8., 100);
    if (!isfinite(gv.n_eff))
        COSMO_FAIL(cmd, "%s: invalid n_eff=%g\n", routineName, gv.n_eff);
    
	if (BmKspace(cmd, gd, cmd->mMax, 0.001, 1.0, 100, cmd->GLpoints, gv.z,gv.Dp,gv.r_sigma,gv.n_eff) == FAILURE)
        return FAILURE;
    printf("time= %lf \n",  CPUTIME - cpumiddle);

    cpumiddle = CPUTIME;
	if (Bmell(cmd, gd) == FAILURE)
        return FAILURE;
	if(cmd->verbose>1)
        printf("Integration time= %lf \n", CPUTIME - cpumiddle);

	if (get_zetam(cmd, gd) == FAILURE)
        return FAILURE;

	if (write(cmd, gd) == FAILURE)
        return FAILURE;

    if (cmd->verbose_log>0 && gd->rootDirFlag == TRUE) {
        verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                            "\n%s: CPU time %g %s\n",
                            routineName, CPUTIME - cpustart, PRNUNITOFTIMEUSED);
    }

    return SUCCESS;
}

//B socket:
#ifdef ADDONS
#include "wlcf_include_05.h"
#endif
//E

