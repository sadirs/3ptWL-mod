/*==============================================================================
 HEADER: procedures.h				[gsm]
 ==============================================================================*/

#ifndef _procedures_h
#define _procedures_h


//int Initial(struct cmdline_data* cmd, struct  global_data* gd);

// integration
//void allocate_iv(void);
int allocate_iv(struct cmdline_data* cmd, struct  global_data* gd);

//void Bmell(void);
int Bmell(struct cmdline_data* cmd, struct  global_data* gd);
//void Bm(double chi, double z, double Dp, double r_sigma, double n_eff);
int Bm(struct cmdline_data* cmd, struct  global_data* gd,
       double chi, double z, double Dp, double r_sigma, double n_eff);

// others
//void BmKspace(int Maxm, double kmin, double kmax, int Nk, int GLpoints, double z, double Dp, double r_sigma, double n_eff);
int BmKspace(struct cmdline_data* cmd, struct  global_data* gd,
              int Maxm, double kmin, double kmax, int Nk,
              int GLpoints, double z, double Dp, double r_sigma,
              double n_eff);


// routines
//void read_inputpk(void);
int read_inputpk(struct cmdline_data* cmd, struct  global_data* gd);
void gaussleg(double x1, double x2, double x[], double w[], int n);
//~ void GLvectors(void);
double interpolationlog(double x, double xT[], double yT[], int n_data);
double interpolation1(double x, double xT[], double yT[], int n_data);


//void write(void);
int write(struct cmdline_data* cmd, struct  global_data* gd);

//void free_variables(void);
int free_variables(struct cmdline_data* cmd,
                   struct  global_data* gd);


// Tests:  in tests.c
void tests2(void);
void vectortomatrix(void);
//void testTakahashiBispectrum(void);
int testTakahashiBispectrum(struct cmdline_data* cmd,
                            struct  global_data* gd);




//B added by bolas...
#ifdef __cplusplus
extern "C" {
#endif

int StartRun(struct cmdline_data* cmd, struct  global_data* gd,
             string, string, string, string);
int StartRun_Common(struct cmdline_data*, struct  global_data*);
int PrintParameterFile(struct cmdline_data *,
                       struct  global_data*, char *);
//B If uncommented there will be a warning in the setup.py process
//#ifdef OPENMPCODE
int SetNumberThreads(struct cmdline_data *cmd);
//E

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

