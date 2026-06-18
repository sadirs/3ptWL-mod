/*==============================================================================
 HEADER: cmdline_data.h		[wlcf]
 Written by: Mario A. Rodriguez-Meza
 Starting date: april 2023
 Purpose: Definitions of global variables and parameters
 Language: C
 Use: '#include "cmdline_data.h"
 Major revisions:
 ==============================================================================*/
//        1          2          3          4        ^ 5          6          7

//
// lines where there is a "//B socket:" string are places to include module files
//  that can be found in addons/addons_include folder
//

#ifndef _cmdline_data_h
#define _cmdline_data_h

#include "common_defs.h"

struct cmdline_data{
    double z;                                       //Evaluation redshift
    //B Background cosmology:
    double Omm;
    double ns;
    double Omb;
    double h;
    double w;
    double Omc;
    double sigma8;
    double Omnu;
    double Omw;
    //E

    //B k table
    string prefix;
    string fnamePS;
    double kmin;
    double kmax;
    int Nk;
    //E
          
    double zbin;

    int chiQuadSteps;                               // For trapezoidal integration
    int GLpoints;                                   // Gaussian Legendre quad
    int mMax;                                       // Bm moments upto mMax
    int Nell;
    double ellmin,ellmax;
    int tree_level;
    bool writevectors;

    int Wg;
    string fWgchi;
    
    //B Output parameters
    string rootDir;
    //E

    //B Miscellaneous parameters
    short verbose;
    short verbose_log;
    int numthreads;
    string options;
    //E

    string version;

    string paramfile;

//B socket:
#ifdef ADDONS
#include "cmdline_data_include.h"
#endif
//E

//B make this correction in setup.py
//      so cwlcfpy.pxd.in produce it
#ifndef CLASSLIB
    ErrorMsg error_message;
#endif
//E

};

#endif // ! _cmdline_data_h
