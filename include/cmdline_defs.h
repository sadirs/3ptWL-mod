/* ==============================================================================
    HEADER: cmdline_defs.h		[wlcf]
    Written by: Mario A. Rodriguez-Meza
    Starting date: 15.02.2026
    Purpose: Definitions for importing arguments from the command line
    Language: C
    Use: '#include "...."
    Major revisions:
 ==============================================================================*/
//        1          2          3          4          5          6          7

//
// lines where there is a "//B socket:" string are places to include module files
//  that can be found in addons/addons_include folder
//

/*
If you need to add a parameter, the pattern is a line like the following:
 
 "parameter_name=default_value",   ";Parameter comment/or brief explanation", ":parameter_shortname",

Instert a line like above inside the defv array.
 
Note: it is not necessary to have the parameter_shortname item. Then in this case the line for a parameter is:
 
 "parameter_name=default_value",   ";Parameter comment/or brief explanation",

After adding line for the new parameter you need to add corresponding line(s) in four routines in startrun.c:
 
 - ReadParametersCmdline
 - startrun_ParamStat
 - ReadParameterFile
 - PrintParameterFile

and if necessary in
 - CheckParameters
 
 */


#ifndef _cmdline_defs_h
#define _cmdline_defs_h

#define HEAD1	"LSST/S3"
#define HEAD2	"wlcf code for computing models of (2,3)pcf"
#define HEAD3	"..."

string defv[] = {  ";"HEAD1": " HEAD2 "\n\t " HEAD3,
    "paramfile=",			            ";Parameter input file. Overwrite what follows",

    //B Parameters related to the background cosmology (like WMAP 2009)
    "z=1.0334",                                     ";Redshift",
    "h=0.7",                                        ";Hubble parameter",
    "sigma8=0.82",                                  ";Sigma8", ":s8",
    "Omb=0.046",                                    ";Baryon content: Omega baryon",
    "Omc=0.233",                                     ";Dark matter content: Omega CDM",
    "ns=0.97",                                      ";Spectral index of linear P(k)",
    "w=-1.0",                                       ";Equation of state of dark energy",
    "Omnu=0.0",                                     ";Massive neutrinos: Omega nu",
    //E

    //B Parameters to control the I/O file(s)
    // Input PS
    "fnamePS=./input/linear_pk_Takahashi_z0.txt",   ";Linear power spectrum", ":ps",
    // Output parameters
    "rootDir=Output",                               ";Output dir, where output files will be written", ":root",
    "prefix=run1_",                                 ";Output file of the points analysed (default ext: .txt)", ":pre",
    "tree_level=3",                                 ";= 1 second order PT, = 2 B=pk^, = 3 uses EFT ctr, = other Halo model",":tlev",
    "zbin=0.5078",                                  ";zbins",
    "mMax=5",                                       ";Bm moments upto mMax",
    "chiQuadSteps=300",                             ";Bm moments upto mMax",":chiqst",
    "GLpoints=64",                                  ";GL points",":gl",
    //E

    "Nell=128",                                     ";Bm moments upto mMax",
    "ellmax=10000.0",                               ";Bm moments upto mMax",
    "ellmin=0.001",                                 ";Bm moments upto mMax",
    "Wg=0",                                         ";=0 Wg(chi) Dirac Delta at chibin=chi(zBin), =1 Wg(chi) from file fWgchi",
    "fWgchi=./input/Wg_Takahashi_z05078.txt",       ";Takahashi file name", ":fwgchi",


    //B Miscellaneous parameters
    "writevectors=true",                ";If true, will be written vectors",
    "verbose=2",                        ";Option to activate the amount of information sent to standard output", ":verb",
    "verbose_log=1",                    ";Option to activate the amount of information sent to log file", ":verblog",
#ifdef OPENMPCODE
    "numberThreads=16",                  ";To set the number of threads to use (OpenMP)", ":nthreads",
#else
    "numberThreads=1",                  ";Default setting for no OpenMP", ":nthreads",
#endif
    "options=",                         ";Various control options, i.e.,... , etc.", ":opt",
    //E

//B socket:
#ifdef ADDONS
#include "cmdline_defs_include.h"
#endif
//E

    "Version=1.0.0",			        ";A. Aviles et al. (2026--)",
    NULL,
};

#endif // ! _cmdline_defs_h
