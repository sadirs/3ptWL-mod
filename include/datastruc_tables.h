/*==============================================================================
 HEADER: datastruc_tables.h		[wlcf]
 Written by: Mario A. Rodriguez-Meza
 Starting date: 15.02.2026
 Purpose: Definitions of global variables and parameters
 Language: C
 Use: '#include "datastruc_tables.h"
 Major revisions:
 ==============================================================================*/
//        1          2          3          4        ^ 5          6          7

//
// lines where there is a "//B socket:" string are places to include module files
//  that can be found in addons/addons_include folder
//

#ifndef _datastruc_tables_h
#define _datastruc_tables_h

//B Structure definitions for tables
//

// use in: wlcf, background and procedures
typedef struct
{
    double Dp, r_sigma, n_eff;
    double z;
    double *chiOfzT, *zT, *DpT;
    double zMin, zMax;
    int Nz;
    double chiBin;
    double *gLT, *chiforgLT;
    int NstepsforgL;
} global_vars, *global_vars_ptr;

global global_vars gv;


// use in: wlcf, zetam, background and procedures
typedef struct
{
    double chiMaxInt, chiMinInt;
    double *chiT;
    int chiQuadSteps, Nell;
    double *chiT_chiint, *zT_chiint, *DpT_chiint, *rsigma_chiint, *neff_chiint;
    double *q_chiint, *kT, *ellT;
    double **BmVectors, **BmVectorsp;
    double ellmin, ellmax;
} integration_vars, *integration_vars_ptr;

global integration_vars iv;


// use in: wlcf and zetam
typedef struct
{
    double *r1, *r2, **result;
} zeta_vars, *zeta_vars_ptr;

global zeta_vars zv;

//B socket:
#ifdef ADDONS
#include "datastruc_tables_include.h"
#endif
//E

//
//E Structure definitions for tables

#endif // ! _datastruc_tables_h

