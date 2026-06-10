#include "global.h"
#include "procedures.h"
#include "functions.h"

int background(struct cmdline_data* cmd, struct  global_data* gd,
               double zBin);

//void   chiArray_all(double chiOfzT[], double zT[]);
int chiArray_all(struct cmdline_data* cmd, struct  global_data* gd,
                 double chiOfzT[], double zT[]);
//double      HoverH0(double z);
double HoverH0(struct cmdline_data* cmd, struct  global_data* gd, double z);
double  chiOfz_func(double z);
double  zOfchi_func(double chi);
double  aOfchi_func(double chi);
double DpOfchi_func(double chi);

//double gL(double chi);
double gL(struct cmdline_data* cmd, struct  global_data* gd, double chi);
double gLDiracDelta(double chi);
//double q(double chi);
double q(struct cmdline_data* cmd, struct  global_data* gd, double chi);

//void chiMaxforInt(void);
int chiMaxforInt(struct cmdline_data* cmd, struct  global_data* gd);
//void ArraysforChiQuad(void);
int ArraysforChiQuad(struct cmdline_data* cmd, struct  global_data* gd);
//void ArraysforChiQuadLog(void);
int ArraysforChiQuadLog(struct cmdline_data* cmd,
                        struct  global_data* gd);

// for photo-z file
int compute_gL(struct cmdline_data* cmd, struct  global_data* gd);
//void read_inputWgchi(void);
int read_inputWgchi(struct cmdline_data* cmd, struct  global_data* gd);
//double Wg_func(double chi);
double Wg_func(struct cmdline_data* cmd, struct  global_data* gd,
               double chi);   // Wg(chi) interpolated
double gL_func(double chi);

