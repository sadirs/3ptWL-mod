/*==============================================================================
 MODULE: wlcfio.c		[wlcf]
 Written by: Mario A. Rodriguez-Meza
 Starting date:	15.02.2026
 Purpose: Routines to drive input and output data
 Language: C
 Use:
 Major revisions:
 ==============================================================================*/
//        1          2          3          4        ^ 5          6          7

//
// lines where there is a "//B socket:" string are places to include module files
//  that can be found in addons/addons_include folder
//

#include "globaldefs.h"
#include <limits.h>

/*
 EndRun routine:

 To be called in main:
    EndRun(&cmd, &gd);

 This routine is in charge of closing log file, printing a summary
    of the run and freeing the allocated memory

 Arguments:
    * `cmd`: Input: structure cmdline_data pointer
    * `gd`: Input: structure global_data pointer
 Return (the error status):
    int SUCCESS or FAILURE
 */
int EndRun(struct cmdline_data* cmd, struct  global_data* gd)
{
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (gd->startrun_cputime == TRUE && cmd->numthreads > 0) {
        real cpuTotal = (CPUTIME - gd->cpuinit)/cmd->numthreads;
        verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "\nFinal CPU time : %lf %s\n",
                               cpuTotal, PRNUNITOFTIMEUSED);
        verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "Final real time: %ld",
                               (rcpu_time()-gd->cpurealinit));
        verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               " %s\n\n", PRNUNITOFTIMEUSED); // Only work this way
    }

    if (gd->outlog != NULL) {
        fclose(gd->outlog);
        gd->outlog = NULL;
    }

    if (EndRun_FreeMemory(cmd, gd) == FAILURE)
        return FAILURE;

    return SUCCESS;
}

//
// We must check the order of memory allocation and freeing!!!
//
int EndRun_FreeMemory(struct cmdline_data* cmd,
                             struct  global_data* gd)
{
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (gd->tables_allocated == TRUE)
        EndRun_FreeMemory_tables(cmd, gd);

    if (gd->gd_allocated == TRUE)
        EndRun_FreeMemory_gd(cmd, gd);

    if (gd->cmd_allocated == TRUE)
        EndRun_FreeMemory_cmd(cmd, gd);

    return SUCCESS;
}

int EndRun_FreeMemory_tables(struct cmdline_data* cmd,
                             struct  global_data* gd)
{
    //B initial checking
    if (cmd == NULL || gd == NULL)
        return FAILURE;
    //E

    if (gd->tables_allocated != TRUE)
        return SUCCESS;
    
    if (zv.result != NULL) {
        if (iv.Nell > 0) {
            for (int i = iv.Nell - 1; i >= 0; i--)
                free(zv.result[i]);
        }
        free(zv.result);
        zv.result = NULL;
    }

    free(zv.r2);
    zv.r2 = NULL;

    free(zv.r1);
    zv.r1 = NULL;

    if (cmd->mMax >= 0 && cmd->mMax != INT_MAX) {
        int NumMoments = cmd->mMax + 1;

        if (iv.BmVectorsp != NULL) {
            for (int m = NumMoments - 1; m >= 0; m--)
                free(iv.BmVectorsp[m]);
            free(iv.BmVectorsp);
            iv.BmVectorsp = NULL;
        }

        if (iv.BmVectors != NULL) {
            for (int m = NumMoments - 1; m >= 0; m--)
                free(iv.BmVectors[m]);
            free(iv.BmVectors);
            iv.BmVectors = NULL;
        }
    }

    free(iv.ellT);
    iv.ellT = NULL;

    free(iv.q_chiint);
    iv.q_chiint = NULL;

    free(iv.neff_chiint);
    iv.neff_chiint = NULL;

    free(iv.rsigma_chiint);
    iv.rsigma_chiint = NULL;

    free(iv.DpT_chiint);
    iv.DpT_chiint = NULL;

    free(iv.chiT_chiint);
    iv.chiT_chiint = NULL;

    free(iv.zT_chiint);
    iv.zT_chiint = NULL;

    free(gv.DpT);
    gv.DpT = NULL;

    free(gv.chiOfzT);
    gv.chiOfzT = NULL;

    free(gv.zT);
    gv.zT = NULL;

    free(gv.gLT);
    gv.gLT = NULL;

    free(gv.chiforgLT);
    gv.chiforgLT = NULL;

    gd->tables_allocated = FALSE;

    return SUCCESS;
}


int EndRun_FreeMemory_gd(struct cmdline_data* cmd,
                             struct  global_data* gd)
{
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    gd->gd_allocated = FALSE;

    return SUCCESS;
}

int EndRun_FreeMemory_cmd(struct cmdline_data* cmd,
                             struct  global_data* gd)
{
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    //B be aware of the allocation order...
    if (gd->optionsFlag == TRUE) {
        free(cmd->options);
        cmd->options = NULL;
        gd->optionsFlag = FALSE;
    }
    if (gd->rootDirFlagFree==TRUE) {
        free(cmd->rootDir);
        cmd->rootDir = NULL;
        gd->rootDirFlagFree = FALSE;
    }
    if (gd->prefixFlag==TRUE) {
        free(cmd->prefix);
        cmd->prefix = NULL;
        gd->prefixFlag = FALSE;
    }
    if (gd->fWgchiFlag==TRUE) {
        free(cmd->fWgchi);
        cmd->fWgchi = NULL;
        gd->fWgchiFlag = FALSE;
    }
    if (gd->fnamePSFlag==TRUE) {
        free(cmd->fnamePS);
        cmd->fnamePS = NULL;
        gd->fnamePSFlag = FALSE;
    }

//B socket:
#ifdef ADDONS
#include "wlcfio_include_03.h"
#endif
//E
    //E

    gd->cmd_allocated = FALSE;

    return SUCCESS;
}

