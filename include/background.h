#include "globaldefs.h"
#include "procedures.h"
#include "functions.h"

int background(struct cmdline_data* cmd, struct  global_data* gd,
               double zBin);

int chiArray_all(struct cmdline_data* cmd, struct  global_data* gd,
                 double chiOfzT[], double zT[]);
double HoverH0(struct cmdline_data* cmd, struct  global_data* gd, double z);
double  chiOfz_func(double z);
double  zOfchi_func(double chi);
double  aOfchi_func(double chi);
double DpOfchi_func(double chi);

double gL(struct cmdline_data* cmd, struct  global_data* gd, double chi);
double gLDiracDelta(double chi);
double q(struct cmdline_data* cmd, struct  global_data* gd, double chi);

int chiMaxforInt(struct cmdline_data* cmd, struct  global_data* gd);
int ArraysforChiQuad(struct cmdline_data* cmd, struct  global_data* gd);
int ArraysforChiQuadLog(struct cmdline_data* cmd,
                        struct  global_data* gd);

// for photo-z file
int compute_gL(struct cmdline_data* cmd, struct  global_data* gd);
int read_inputWgchi(struct cmdline_data* cmd, struct  global_data* gd);
double Wg_func(struct cmdline_data* cmd, struct  global_data* gd,
               double chi);   // Wg(chi) interpolated
double gL_func(double chi);

