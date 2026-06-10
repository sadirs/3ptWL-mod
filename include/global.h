#ifndef _GLOBAL_H
#define _GLOBAL_H


#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#define n_data_max 10000
#define n_chi_data_max 10000

#include <string.h>
#include "stdinc.h"
//#include "vectdefs.h"
//#include "vectmath.h"
#include "getparam.h"
//#include "mathfns.h"
//#include "mathutil.h"
//#include "numrec.h"
//#include "inout.h"
//#include "constant.h"
#include "common_defs.h"


#if !defined(GLOBAL)
#  define GLOBAL extern
#  define global extern
#endif

//#ifdef USEGSL
//global gsl_rng * r_gsl;                             // seed for random generators
//#else
global long idum;                                   // seed for random generators
//#endif

// To use in inout and cballsio
global real *inout_xval;
global real *inout_yval;
global real *inout_zval;
global real *inout_uval;
global real *inout_vval;
global real *inout_wval;

typedef char *string;

#include "cmdline_data.h"

/*
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
#ifdef OPENMPCODE
    int numthreads;
#endif
    string options;
    //E

    string version;

    string paramfile;

};
*/





#include "global_data.h"

/*
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
*/




typedef struct 
{	
	clock_t time;    
	double Dp, r_sigma, n_eff;
	double z;
	double *chiOfzT, *zT, *DpT;
	double zMin, zMax;
	int Nz;
	double chiBin;  
	double *gLT, *chiforgLT; 
	int NstepsforgL;
} global_vars, *global_vars_ptr;

GLOBAL global_vars gv;


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

GLOBAL integration_vars iv;


typedef struct 
{	  
	double *r1, *r2, **result;
} zeta_vars, *zeta_vars_ptr;

GLOBAL zeta_vars zv;


//B CLASSLIB section
// standard libraries from Julien Lesgourges CLASS
#ifdef CLASSLIB
#include "common.h"
global ErrorMsg errmsg;
#else // ! CLASSLIB

#if !defined(TRUE)
#define TRUE 1                                      // integer associated to true
                                                    //  statement
#endif
#if !defined(FALSE)
#define FALSE 0                                     // integer associated to false
                                                    //  statement
#endif
#define SUCCESS 0                                   // integer returned after successful
                                                    //  call of a function
#define FAILURE 1                                   // integer returned after failure in
                                                    //  a function
#define _ERRORMSGSIZE_ 2048                         // generic error messages are cut
                                                    //  beyond this number of characters
typedef char ErrorMsg[_ERRORMSGSIZE_];              // Generic error messages
#ifndef MIN
#define MIN(a,b) (((a)<(b)) ? (a) : (b) )           // the usual "min" function
#endif
#ifndef MAX
#define MAX(a,b) (((a)<(b)) ? (b) : (a) )           // the usual "max" function
#endif
#define SIGN(a) (((a)>0) ? 1. : -1. )
#define NRSIGN(a,b) ((b) >= 0.0 ? fabs(a) : -fabs(a))
global ErrorMsg errmsg;

//B common section

//B Routines defined in general_libs/stdinc.c:
// needed because of weird openmp bug on macosx lion...
void class_protect_sprintf(char* dest, char* tpl,...);
void class_protect_fprintf(FILE* dest, char* tpl,...);
void* class_protect_memcpy(void* dest, void* from, size_t sz);
// some general functions
int get_number_of_titles(char * titlestring);
int file_exists(const char *fname);
int compare_doubles(const void * a,
                    const void * b);
int string_begins_with(char* thestring, char beginchar);
//E

#define class_build_error_string(dest,tmpl,...) {                       \
  ErrorMsg FMsg;                                                        \
  class_protect_sprintf(FMsg,tmpl,__VA_ARGS__);                         \
  class_protect_sprintf(dest,"%s(L:%d) :%s",__func__,__LINE__,FMsg);    \
}


#define class_test_message(err_out,extra,args...) {                      \
  ErrorMsg Optional_arguments;                                           \
  class_protect_sprintf(Optional_arguments,args);                        \
  class_build_error_string(err_out,"condition (%s) is true; %s",extra,Optional_arguments); \
}


#define class_test(condition, error_message_output, args...) {           \
  if (condition) {                                                       \
    class_test_message(error_message_output,#condition, args);           \
    return FAILURE;                                                      \
  }                                                                      \
}

//E
#endif // ! CLASSLIB
//E

#include "protodefs.h"

#endif  // ! _GLOBAL_H
