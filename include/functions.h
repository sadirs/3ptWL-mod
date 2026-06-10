#include "global.h"
#include "procedures.h"

#define eps 0.0001

//double Bispec_Takahashi(double k1, double k2, double k3, double z, double Dp, double r_sigma, double n_eff);
double Bispec_Takahashi(struct cmdline_data* cmd, struct  global_data* gd,
                        double k1, double k2, double k3, double z, double Dp, double r_sigma, double n_eff);

//double F2(double k1, double k2, double k3, double z, double Dp, double r_sigma);
double F2(struct cmdline_data* cmd, struct  global_data* gd,
          double k1, double k2, double k3, double z, double Dp, double r_sigma);

double F2_tree(double k1, double k2, double k3);
//double Bispec_tree(double k1, double k2, double k3, double Dp);
double Bispec_tree(struct cmdline_data* cmd, struct  global_data* gd,
                   double k1, double k2, double k3, double Dp);
//double Bispec_P2(double k1, double k2, double k3, double Dp);
double Bispec_P2(struct cmdline_data* cmd, struct  global_data* gd,
                 double k1, double k2, double k3, double Dp);
//double Bispec_tree_EFT(double k1, double k2, double k3, double Dp, double ctr);
double Bispec_tree_EFT(struct cmdline_data* cmd, struct  global_data* gd,
                       double k1, double k2, double k3, double Dp, double ctr);


//double calcrsigma(double Dp, double kini, double kfin, int Nk);
double calcrsigma(struct cmdline_data* cmd, struct  global_data* gd,
                  double Dp, double kini, double kfin, int Nk);

//double sigmaRTH(double r, double kini, double kfin, int Nk);
double sigmaRTH(struct cmdline_data* cmd, struct  global_data* gd,
                double r, double kini, double kfin, int Nk);
//double sigmaRGaussian(double r, double kini, double kfin, int Nk);
double sigmaRGaussian(struct cmdline_data* cmd, struct  global_data* gd,
                      double r, double kini, double kfin, int Nk);
//double sigmaRGaussian1stDeriv(double r, double kini, double kfin, int Nk);
double sigmaRGaussian1stDeriv(struct cmdline_data* cmd,
                              struct  global_data* gd,
                              double r, double kini, double kfin, int Nk);
//double n_eff_func(double r_sigma, double Dp, double kini, double kfin, int Nk);
double n_eff_func(struct cmdline_data* cmd, struct  global_data* gd,
                  double r_sigma, double Dp, double kini,
                  double kfin, int Nk);

//double sigmam(double r, int j);
double sigmam(struct cmdline_data* cmd, struct  global_data* gd,
              double r, int j);
double window(double x, int i);

//double linear_pkz0(double k);
double linear_pkz0(struct cmdline_data* cmd, struct  global_data* gd,
                   double k);
//double linear_pkz0_data(double k);
double linear_pkz0_data(struct cmdline_data* cmd, struct  global_data* gd,
                        double k);
//double linear_pkz0_eh(double k);
double linear_pkz0_eh(struct cmdline_data* cmd, struct  global_data* gd, double k);

//double Dplusf(double z);
double Dplusf(struct cmdline_data* cmd, struct  global_data* gd,
              double z);
//double Dplusf_func(int j, double la, double y[2]);
double Dplusf_func(struct cmdline_data* cmd, struct  global_data* gd,
                   int j, double la, double y[2]);

