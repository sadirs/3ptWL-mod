/*==============================================================================
 HEADER: cmdline_data.h		[wlcf]
 Written by: Mario A. Rodriguez-Meza
 Starting date: april 2023
 Purpose: Definitions of global variables and parameters
 Language: C
 Use: '#include "cmdline_data.h"
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
    string prefix;
    double z; //Evaluation redshift
// Background cosmology:
    double Omm;
    double ns;
    double Omb;
    double h;
    double w;
    double Omc;
    double sigma8;
    double Omnu;
    double Omw;
// k table
    string fnamePS;
    string path_Bells;
    double kmin;
    double kmax;
    int Nk;
          
    double zbin;  //

    int chiQuadSteps; // For trapezoidal integration
    int GLpoints; // For Gaussian Legendre integration
    int mMax; // Bm moments upto mMax
    int Nell;
    double ellmin,ellmax;
    int tree_level, writevectors;
    
    int chatty; // =0,1,2.
    
    int Wg;
    string fWgchi;
    
    // Output parameters
    string rootDir;
    //E

    //B Miscellaneous parameters
    string preScript;
    string posScript;
    short verbose;
    short verbose_log;
//#ifdef OPENMPCODE
    int numthreads;
//#endif
    string options;
    //E

    string version;

    string paramfile;

};

#endif // ! _cmdline_data_h
