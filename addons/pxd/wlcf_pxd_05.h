// Use:
//#include "wlcf_pxd_05.h"

// included in addons/addons_include/source/cballs_include_05.h

#ifndef _wlcf_pxd_05_h
#define _wlcf_pxd_05_h

//B parameters section

/*
//B flags
int get_tree_allocated(struct global_data* gd, short *value)
{
    *value = gd->tree_allocated;
    return SUCCESS;
}

int get_allocated_2(struct global_data* gd, short *value)
{
    *value = gd->gd_allocated_2;
    return SUCCESS;
}

int get_bodytable_allocated(struct global_data* gd, short *value)
{
    *value = gd->bodytable_allocated;
    return SUCCESS;
}

int get_histograms_allocated(struct global_data* gd, short *value)
{
    *value = gd->histograms_allocated;
    return SUCCESS;
}

int get_gd_allocated(struct global_data* gd, short *value)
{
    *value = gd->gd_allocated;
    return SUCCESS;
}

int get_cmd_allocated(struct global_data* gd, short *value)
{
    *value = gd->cmd_allocated;
    return SUCCESS;
}
//E
*/

int get_nthreads(struct  cmdline_data* cmd, int *value)
{
    *value = cmd->numthreads;
    return SUCCESS;
}

/*
//B version 1.0.1
int get_nmonopoles(struct  cmdline_data* cmd, int *value)
{
    *value = cmd->mChebyshev;
    return SUCCESS;
}
//E

int get_theta(struct  cmdline_data* cmd, real *theta)
{
    *theta = cmd->theta;
    return SUCCESS;
}

int get_rsmooth(struct  global_data* gd, real *value)
{
    *value = gd->rsmooth[0];
    return SUCCESS;
}

int get_cputime(struct  global_data* gd, real *cputime)
{
    *cputime = gd->cpusearch;
    return SUCCESS;
}

int get_sizeHistN(struct  cmdline_data* cmd, int *sizeHistN)
{
    *sizeHistN = cmd->sizeHistN;
    return SUCCESS;
}

// use same version as is in cmdline_defs.h and/or in addons/class_lib/common.h
//  see also setup.py
int get_version(struct  cmdline_data* cmd, char *param)
{
    sprintf(param,"%s","1.0.1");
    return SUCCESS;
}

int get_rootDir(struct  cmdline_data* cmd, char *value)
{
    sprintf(value,"%s",cmd->rootDir);
    return SUCCESS;
}
//E parameters section


//B histograms section
int get_rBins(struct  cmdline_data* cmd, struct  global_data* gd)
{
    real rBin, rbinlog;
    int n;

    for (n=1; n<=cmd->sizeHistN; n++) {
        if (cmd->useLogHist) {
            if (cmd->rminHist==0) {
                rbinlog = ((real)(n-cmd->sizeHistN))/cmd->logHistBinsPD + rlog10(cmd->rangeN);
            } else {
                rbinlog = rlog10(cmd->rminHist) + ((real)(n)-0.5)*gd->deltaR;
            }
            rBin=rpow(10.0,rbinlog);
        } else {
            rBin = cmd->rminHist + ((real)n-0.5)*gd->deltaR;
        }
        gd->rBins[n] = rBin;
    }

    return SUCCESS;
}

int get_HistNN(struct  cmdline_data* cmd, struct  global_data* gd)
{
    int n;

    for (n=1; n<=cmd->sizeHistN; n++) {
        gd->vecPXD[n] = gd->histNN[n];
    }

    return SUCCESS;
}

int get_HistCF(struct  cmdline_data* cmd, struct  global_data* gd)
{
    int n;

    for (n=1; n<=cmd->sizeHistN; n++) {
        gd->vecPXD[n] = gd->histCF[n];
    }

    return SUCCESS;
}

int get_HistXi2pcf(struct  cmdline_data* cmd, struct  global_data* gd)
{
    int n;

    for (n=1; n<=cmd->sizeHistN; n++) {
        gd->vecPXD[n] = gd->histXi2pcf[n];
    }

    return SUCCESS;
}


// get matrix ZetaM for each m multipole
#define _COS_         1
#define _SIN_         2
#define _SINCOS_      3
#define _COSSIN_      4

int get_HistZetaMsincos(struct  cmdline_data* cmd,
                     struct  global_data* gd,
                     int m, int type, ErrorMsg errmsg)
{
    int n1, n2;

    class_test((m <= 0 || m > cmd->mChebyshev + 1),
        errmsg,"\nget_HistZetaM_sincos: not allowed value of m = %d\n",m);
    for (n1=1; n1<=cmd->sizeHistN; n1++) {
        for (n2=1; n2<=cmd->sizeHistN; n2++) {
            gd->matPXD[n1][n2] = gd->histZetaMcos[m][n1][n2];
        }
    }

    switch(type) {
        case _COS_:
            for (n1=1; n1<=cmd->sizeHistN; n1++) {
                for (n2=1; n2<=cmd->sizeHistN; n2++) {
                    gd->matPXD[n1][n2] = gd->histZetaMcos[m][n1][n2];
                }
            }
            break;
        case _SIN_:
            for (n1=1; n1<=cmd->sizeHistN; n1++) {
                for (n2=1; n2<=cmd->sizeHistN; n2++) {
                    gd->matPXD[n1][n2] = gd->histZetaMsin[m][n1][n2];
                }
            }
            break;
        case _SINCOS_:
            for (n1=1; n1<=cmd->sizeHistN; n1++) {
                for (n2=1; n2<=cmd->sizeHistN; n2++) {
                    gd->matPXD[n1][n2] = gd->histZetaMsincos[m][n1][n2];
                }
            }
            break;
        case _COSSIN_:
            for (n1=1; n1<=cmd->sizeHistN; n1++) {
                for (n2=1; n2<=cmd->sizeHistN; n2++) {
                    gd->matPXD[n1][n2] = gd->histZetaMcossin[m][n1][n2];
                }
            }
            break;
        default:
            for (n1=1; n1<=cmd->sizeHistN; n1++) {
                for (n2=1; n2<=cmd->sizeHistN; n2++) {
                    gd->matPXD[n1][n2] = gd->histZetaMcos[m][n1][n2];
                }
            }
            break;
    }

    return SUCCESS;
}

#undef _COS_
#undef _SIN_
#undef _SINCOS_
#undef _COSSIN_
//E histograms section

//E PXD functions

*/

#endif	// ! _wlcf_pxd_05_h
