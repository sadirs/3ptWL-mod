/*==============================================================================
 HEADER: procedures.h				[gsm]
 ==============================================================================*/

#ifndef _procedures_h
#define _procedures_h


int Initial(struct cmdline_data* cmd, struct  global_data* gd);

// integration
int Bmell(struct cmdline_data* cmd, struct  global_data* gd);

#ifdef OPENMPCODE
int Bm(struct cmdline_data* cmd, struct  global_data* gd,
       double chi, double z, double Dp, double r_sigma, double n_eff, double **BmVectors);
#else
int Bm(struct cmdline_data* cmd, struct  global_data* gd,
       double chi, double z, double Dp, double r_sigma, double n_eff);
#endif

// others
int BmKspace(struct cmdline_data* cmd, struct  global_data* gd,
              int Maxm, double kmin, double kmax, int Nk,
              int GLpoints, double z, double Dp, double r_sigma,
              double n_eff);


// routines
int read_inputpk(struct cmdline_data* cmd, struct  global_data* gd);
void gaussleg(double x1, double x2, double x[], double w[], int n);
double interpolationlog(double x, double xT[], double yT[], int n_data);
double interpolation1(double x, double xT[], double yT[], int n_data);


int write(struct cmdline_data* cmd, struct  global_data* gd);

int free_variables(struct cmdline_data* cmd,
                   struct  global_data* gd);


// Tests:  in tests.c
void tests2(void);
void vectortomatrix(void);
int testTakahashiBispectrum(struct cmdline_data* cmd,
                            struct  global_data* gd);

int MainLoop(struct cmdline_data* cmd, struct  global_data* gd);


//B added by bolas...
#ifdef __cplusplus
extern "C" {
#endif

int StartRun(struct cmdline_data* cmd, struct  global_data* gd,
             string, string, string, string);
int StartRun_Common(struct cmdline_data*, struct  global_data*);
int PrintParameterFile(struct cmdline_data *,
                       struct  global_data*, char *);
int SetNumberThreads(struct cmdline_data *cmd);

//B socket:
#ifdef ADDONS
// If you have an addon that need global proto definitions
//  go to this file and add the addon item.
#include "protodefs_include.h"
#endif
//E

#ifdef __cplusplus
}
#endif


#endif // ! _procedures_h

