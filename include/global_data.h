/*==============================================================================
 HEADER: global_data.h		[wlcf]
 Written by: Mario A. Rodriguez-Meza
 Starting date: 15.02.2026
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

#include "common_defs.h"

#ifndef n_data_max
#define n_data_max 10000
#endif
#ifndef n_chi_data_max
#define n_chi_data_max 10000
#endif

#ifndef MAXLENGTHOFPSPATH
#define MAXLENGTHOFPSPATH 100
#endif

struct global_data{
    double cpuinit;
    long cpurealinit;                               // get time of the day
    double dx;
    int method_int;
    int quadmethod_int;
    string headline0;
    string headline1;
    string headline2;
    string headline3;
    FILE *outlog;
    char mode[2];
    char fnamePSPath[MAXLENGTHOFPSPATH];
    double k_data[n_data_max], pkz0_data[n_data_max];
    int n_data;
    double Dpz0;
    double sigma8; // This is sigma8 for input pk at z=0;
    double chi_data[n_chi_data_max];
    double Wg_chi_data[n_chi_data_max];
    int n_chi_data;

    bool startrun_cputime;
    bool cmd_allocated;
    bool gd_allocated;
    bool tables_allocated;
    INTEGER bytes_tot;
    bool flagPrint;
    bool rootDirFlag;
    char logfilePath[MAXLENGTHOFFILES];
    char tmpDir[MAXLENGTHOFFILES];

    bool rootDirFlagFree;
    bool fWgchiFlag;
    bool optionsFlag;
    bool prefixFlag;
    bool fnamePSFlag;

    //B socket:
    #ifdef ADDONS
    #include "global_data_include.h"
    #endif
    //E

};

#endif // ! _global_data_h

