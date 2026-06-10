/*==============================================================================
 HEADER: global_data.h		[wlcf]
 Written by: Mario A. Rodriguez-Meza
 Starting date: april 2023
 Purpose: Definitions of global variables and parameters
 Language: C
 Use: '#include "global_data.h"
 Major revisions:
 ==============================================================================*/
//        1          2          3          4        ^ 5          6          7

//
// lines where there is a "//B socket:" string are places to include module files
//  that can be found in addons/addons_include folder
//

#ifndef _global_data_h
#define _global_data_h

//#include "vectdefs.h"
//#include "datastruc_defs.h"
#include "common_defs.h"

#ifndef n_data_max
#define n_data_max 10000
#endif
#ifndef n_chi_data_max
#define n_chi_data_max 10000
#endif

struct global_data{
    double cpuinit;
    double dx;
    int method_int;
    int quadmethod_int;
    clock_t time;
    string headline0;
    string headline1;
    string headline2;
    string headline3;
    FILE *outlog;
    char mode[2];
    char fnamePSPath[100];
    double k_data[n_data_max], pkz0_data[n_data_max];
    int n_data;
    double Dpz0;
    double sigma8; // This is sigma8 for input pk at z=0;
    //~ int Nk; // number of log spaced k in Bm(k1,k2), Check if used
    //~ double kmin, kmax; // kmin and kmax in Bm(k1,k2). Check if used
    //~ double *kT; // Array of k used for Bm(k1,k2). Check if used
    //~ double *veckBm0, *veckBm1, *veckBm2;
    double chi_data[n_chi_data_max], Wg_chi_data[n_chi_data_max];
    int n_chi_data;

    bool cmd_allocated;
    real cputotalinout;
    real cputotal;
    INTEGER bytes_tot;
    bool flagPrint;
    bool rootDirFlag;
    char logfilePath[MAXLENGTHOFFILES];
    char tmpDir[MAXLENGTHOFFILES];

    bool rootDirFlagFree;
    bool fWgchiFlag;
    bool optionsFlag;
    bool path_BellsFlag;
    bool prefixFlag;
    bool fnamePSFlag;

};

#endif // ! _global_data_h

