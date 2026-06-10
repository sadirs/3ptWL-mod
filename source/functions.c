//==========================================================================
//        1          2          3          4        ^ 5          6          7

/*
============================================================================
NAME: functions.c                            [wlcf]
Written by: S. Aviles et al.
Starting date: February 2026
Purpose: Cosmological functions, power spectrum utilities, bispectrum
         models, and growth-factor routines
Language: C
//==========================================================================
*/


#include "functions.h"

#define invH0     2997.92458   //This is H_0^{-1} in Mpc/h units
#define m_PI   3.1415926535897932384626433   
#define PI2     9.8696044010893586188

double norm=1.;

//~ norm=1.;  
//~ norm*=sigma8/sigmam(8.,0);   // P(k) amplitude normalized by sigma8


/*
Takahashi bispectrum routine:

This routine computes the nonlinear matter bispectrum using the
Takahashi fitting formula.

*/
double Bispec_Takahashi(struct cmdline_data* cmd, struct  global_data* gd,
                        double k1, double k2, double k3, double z,
                        double Dp, double r_sigma, double n_eff)
// non-linear BS w/o baryons [(Mpc/h)^6]
{
  int i,j;
  double q[4],k[4],kt,logsigma8z,r1,r2;
  double an,bn,cn,en,fn,gn,hn,mn,nn,pn,alphan,betan,mun,nun,BS1h,BS3h,PSE[4];
  double ns;
  // ns: scalar spectral index of primordial perturbations
  
  ns = cmd->ns;

  if(z>10.) return Bispec_tree(cmd, gd, k1,k2,k3,Dp);
  
  q[1]=k1*r_sigma, q[2]=k2*r_sigma, q[3]=k3*r_sigma;  // dimensionless wavenumbers

  k[1]=k1, k[2]=k2, k[3]=k3;
  // sorting k[i] so that k[1]>=k[2]>=k[3]
  for(i=1;i<=3;i++){
    for(j=i+1;j<=3;j++){
      if(k[i]<k[j]){
        kt=k[j];
        k[j]=k[i];
        k[i]=kt;
      }}}
  r1=k[3]/k[1], r2=(k[2]+k[3]-k[1])/k[1];   // Eq.(B8)
  if(k[1]>k[2]+k[3]) printf("k1=%f, k2=%f, k3=%f. Error: triangle is not formed \n",k[1],k[2],k[3]);

  logsigma8z=log10(Dp*gd->sigma8);
  
  // 1-halo term parameters in Eq.(B7)
  an=pow(10.,-2.167-2.944*logsigma8z-1.106*pow(logsigma8z,2)-2.865*pow(logsigma8z,3)-0.310*pow(r1,pow(10.,0.182+0.57*n_eff)));
  bn=pow(10.,-3.428-2.681*logsigma8z+1.624*pow(logsigma8z,2)-0.095*pow(logsigma8z,3));
  cn=pow(10.,0.159-1.107*n_eff);
  alphan=pow(10.,-4.348-3.006*n_eff-0.5745*pow(n_eff,2)+pow(10.,-0.9+0.2*n_eff)*pow(r2,2));
  if(alphan>1.-(2./3.)*ns) alphan=1.-(2./3.)*ns;
  betan=pow(10.,-1.731-2.845*n_eff-1.4995*pow(n_eff,2)-0.2811*pow(n_eff,3)+0.007*r2);

  // 1-halo term bispectrum in Eq.(B4)
  BS1h=1.;
  for(i=1;i<=3;i++){
    BS1h*=1./(an*pow(q[i],alphan)+bn*pow(q[i],betan))/(1.+1./(cn*q[i]));
  }
  
  // 3-halo term parameters in Eq.(B9)
  fn=pow(10.,-10.533-16.838*n_eff-9.3048*pow(n_eff,2)-1.8263*pow(n_eff,3));
  gn=pow(10.,2.787+2.405*n_eff+0.4577*pow(n_eff,2));
  hn=pow(10.,-1.118-0.394*n_eff);
  mn=pow(10.,-2.605-2.434*logsigma8z+5.71*pow(logsigma8z,2));
  nn=pow(10.,-4.468-3.08*logsigma8z+1.035*pow(logsigma8z,2));
  mun=pow(10.,15.312+22.977*n_eff+10.9579*pow(n_eff,2)+1.6586*pow(n_eff,3));
  nun=pow(10.,1.347+1.246*n_eff+0.4525*pow(n_eff,2));
  pn=pow(10.,0.071-0.433*n_eff);
  en=pow(10.,-0.632+0.646*n_eff);

  for(i=1;i<=3;i++){
    PSE[i]=(1.+fn*pow(q[i],2))/(1.+gn*q[i]+hn*pow(q[i],2))*pow(Dp,2)
      *linear_pkz0(cmd, gd, q[i]/r_sigma)
    +1./(mn*pow(q[i],mun)+nn*pow(q[i],nun))/(1.+pow(pn*q[i],-3));  // enhanced P(k) in Eq.(B6)
  }

  // 3-halo term bispectrum in Eq.(B5)
  BS3h=2.*(F2(cmd, gd, k1,k2,k3,z,Dp,r_sigma)*PSE[1]*PSE[2]
            +F2(cmd, gd, k2,k3,k1,z,Dp,r_sigma)*PSE[2]*PSE[3]
            +F2(cmd, gd, k3,k1,k2,z,Dp,r_sigma)*PSE[3]*PSE[1]);
 
  for(i=1;i<=3;i++) BS3h*=1./(1.+en*q[i]);
   
  return BS1h+BS3h;
}


/*
Modified F2 kernel routine:
*/
double F2(struct cmdline_data* cmd, struct  global_data* gd,
          double k1, double k2, double k3, double z, double Dp, double r_sigma)
{
  double a,q,dn,omz,logsigma8z;
  // a          : cosmological scale factor
  // q          : dimensionless wave number q = k3 * r_sigma
  // omz        : matter density parameter at redshift z
  // logsigma8z : log10(Dp * sigma8)
  q=k3*r_sigma;
  
  logsigma8z=log10(Dp*gd->sigma8);
  a=1./(1.+z);
  omz=cmd->Omm/(cmd->Omm+cmd->Omw*pow(a,-3.*cmd->w));   // Omega matter at z
  // cmd->Omm : present-day matter density parameter
  // cmd->Omw : present-day dark energy density parameter
  // cmd->w   : dark energy equation-of-state parameter
  dn=pow(10.,-0.483+0.892*logsigma8z-0.086*omz);

  return F2_tree(k1,k2,k3)+dn*q;
}

/*
Tree-level F2 kernel routine:
*/
double F2_tree(double k1, double k2, double k3)  // F2 kernel in tree level
{
  double costheta12=0.5*(k3*k3-k1*k1-k2*k2)/(k1*k2);
  return (5./7.)+0.5*costheta12*(k1/k2+k2/k1)+(2./7.)*costheta12*costheta12;
}

/*
Tree-level bispectrum routine:
*/
double Bispec_tree(struct cmdline_data* cmd, struct  global_data* gd,
                   double k1, double k2, double k3, double Dp)
// tree-level BS [(Mpc/h)^6]
{
  return pow(Dp,4)*2.*(
               F2_tree(k1,k2,k3)*linear_pkz0(cmd, gd, k1)
                       *linear_pkz0(cmd, gd, k2)
              +F2_tree(k2,k3,k1)*linear_pkz0(cmd, gd, k2)
                       *linear_pkz0(cmd, gd, k3)
              +F2_tree(k3,k1,k2)*linear_pkz0(cmd, gd, k3)
                       *linear_pkz0(cmd, gd, k1)
              );
}

/*
Tree-level EFT bispectrum routine:
*/
double Bispec_tree_EFT(struct cmdline_data* cmd, struct  global_data* gd,
                       double k1, double k2, double k3, double Dp,
                       double ctr)
// tree-level BS [(Mpc/h)^6]
{
  return pow(Dp,4)*2.*(
               F2_tree(k1,k2,k3)*linear_pkz0(cmd, gd, k1)
                    *linear_pkz0(cmd, gd, k2)*(1+ ctr*k1*k1)*(1+ ctr*k2*k2)
              +F2_tree(k2,k3,k1)*linear_pkz0(cmd, gd, k2)
                    *linear_pkz0(cmd, gd, k3)*(1+ ctr*k2*k2)*(1+ ctr*k3*k3)
              +F2_tree(k3,k1,k2)*linear_pkz0(cmd, gd, k3)
                    *linear_pkz0(cmd, gd, k1)*(1+ ctr*k3*k3)*(1+ ctr*k1*k1)
              );
}

/*
P2 bispectrum approximation routine:
*/
double Bispec_P2(struct cmdline_data* cmd, struct  global_data* gd,
                 double k1, double k2, double k3, double Dp)
// tree-level BS [(Mpc/h)^6]
{
  return pow(Dp,4)*linear_pkz0(cmd, gd, k1)*linear_pkz0(cmd, gd, k2);
}

/*
Nonlinear scale routine:
*/
double calcrsigma(struct cmdline_data* cmd, struct  global_data* gd,
                  double Dp, double kini, double kfin, int Nk)
// return r_sigma[Mpc/h] (=1/k_nl)
{
    double k,k1,k2, sgG;

    k1=k2=1.;
    for(;;){
        sgG= sigmaRGaussian(cmd, gd, 1./k1, kini, kfin, Nk);
        if(Dp*sgG<1.) break;
        k1*=0.5;
    }
    for(;;){
        sgG= sigmaRGaussian(cmd, gd, 1./k2, kini, kfin, Nk);
        if(Dp*sgG>1.) break;
        k2*=2.;
    }
    
    for(;;){
        k=0.5*(k1+k2);
        sgG= sigmaRGaussian(cmd, gd, 1./k, kini, kfin, Nk);
        if(Dp*sgG<1.) k1=k;
        else if(Dp*sgG>1.) k2=k;
        if(Dp*sgG==1. || fabs(k2/k1-1.)<eps*0.1) break;
  }

  return 1./k;
}


/*
Original Takahashi sigma routine:
*/
double sigmam(struct cmdline_data* cmd, struct  global_data* gd,
              double r, int j)
// r[Mpc/h]
{
  int n,i,l;
  double k1,k2,xx,xxp,xxpp,k,a,b,hh,x;

  k1=2.*M_PI/r;
  k2=2.*M_PI/r;

  xxpp=-1.0;
  for(;;){
    k1=k1/10.0;
    k2=k2*2.0;

    a=log(k1),b=log(k2);
    
    xxp=-1.0;
    n=2;
    for(;;){
      n=n*2;
      hh=(b-a)/(double)n;

      xx=0.;
      for(i=1;i<n;i++){
    k=exp(a+hh*i);
    xx+=k*k*k*linear_pkz0(cmd, gd, k)*pow(window(k*r,j),2);
      }
      xx+=0.5*(k1*k1*k1
               *linear_pkz0(cmd, gd, k1)*pow(window(k1*r,j),2)
               +k2*k2*k2*linear_pkz0(cmd, gd, k2)*pow(window(k2*r,j),2));
      xx*=hh;

      if(fabs((xx-xxp)/xx)<eps) break;
      xxp=xx;
    }

    if(fabs((xx-xxpp)/xx)<eps) break;
    xxpp=xx;
  }

  return sqrt(xx/(2.0*M_PI*M_PI));
}

/*
Top-hat sigma routine:

This routine computes the variance sigma(r) using a top-hat
window function in real space. In this code we use the following sigma (and below).
*/
double sigmaRTH(struct cmdline_data* cmd, struct  global_data* gd,
                double r, double kini, double kfin, int Nk)
// r[Mpc/h]
{
    double xp,xA,xB,kr,window;
    double kv, deltak;
    double kT[Nk];

    for(int j=0; j<Nk;j++){
        kT[j] = exp(log(kini) + j*log(kfin/kini)/(Nk-1.0));
    };

    xp=0.0; xA=0.0; xB=0.0;
    kv=kT[0];
    kr = kv*r;
    window = 3./pow(kr,3.) * ( sin(kr) - kr*cos(kr) );
    xA = kv*kv*kv*linear_pkz0(cmd, gd, kv)*window*window;
    
    for(int i=1;i<Nk;i++){
        kv=kT[i];
        deltak=log(kv/kT[i-1]);
        kr=kv*r;
        window = 3./pow(kr,3.) * (sin(kr) - kr*cos(kr));
        xB = kv*kv*kv*linear_pkz0(cmd, gd, kv)*window*window;
        xp = xp + 0.5*(xA+xB)*deltak;
        xA=xB;
    }
    return sqrt(xp/(2.0*M_PI*M_PI));
}
/*
Gaussian sigma routine:
*/
double sigmaRGaussian(struct cmdline_data* cmd, struct  global_data* gd,
                      double r, double kini, double kfin, int Nk)
// r[Mpc/h]
{
    double xp,xA,xB,kr,window;
    double k, deltak;
    double kT[Nk];

    for(int j=0; j<Nk;j++){
        kT[j] = exp(log(kini) + j*log(kfin/kini)/(Nk-1.0));
    };
     
    xp=0.0; xA=0.0; xB=0.0;
    k=kT[0];
    kr = k*r;
    window = exp(-0.5*kr*kr);
    xA = k*k*k*linear_pkz0(cmd, gd, k)*window*window;

    for(int i=1;i<Nk;i++){
        k=kT[i];
        deltak=log(k/kT[i-1]);
        kr=k*r;
        window = exp(-0.5*kr*kr);
        xB = pow(k,3.)*linear_pkz0(cmd, gd, k)*window*window;
        xp += 0.5*(xA+xB)*deltak;
        xA=xB;
    }
    return sqrt(xp/(2.0*M_PI*M_PI));
}
/*
First-derivative Gaussian sigma routine:
*/
double sigmaRGaussian1stDeriv(struct cmdline_data* cmd, struct  global_data* gd,
                              double r, double kini, double kfin, int Nk)
// r[Mpc/h]
{
    double xp,xA,xB,kr,window;
    double k, deltak;
    double kT[Nk];

    for(int j=0; j<Nk;j++){
        kT[j] = exp(log(kini) + j*log(kfin/kini)/(Nk-1.0));
    };
     
    xp=0.0; xA=0.0; xB=0.0;
    k=kT[0];
    kr = k*r;
    window = kr*exp(-0.5*kr*kr);
    xA = k*k*k*linear_pkz0(cmd, gd, k)*window*window;
    
    for(int i=1;i<Nk;i++){
        k=kT[i];
        deltak=log(k/kT[i-1]);
        kr=k*r;
        window = kr*exp(-0.5*kr*kr);
        xB = pow(k,3.)*linear_pkz0(cmd, gd, k)*window*window;
        xp += 0.5*(xA+xB)*deltak;
        xA=xB;
    }
    return sqrt(xp/(2.0*M_PI*M_PI));
}

/*
Effective spectral index routine:
*/
double n_eff_func(struct cmdline_data* cmd, struct  global_data* gd,
                  double r_sigma, double Dp, double kini, double kfin, int Nk)
{
    return -3.+2.*pow(Dp*sigmaRGaussian1stDeriv(cmd, gd, r_sigma, kini,kfin,Nk),2);
}
/*
Window routine:

*/
double window(double x, int i)
{
  if(i==0) return 3.0/pow(x,3)*(sin(x)-x*cos(x));  // top hat
  if(i==1) return exp(-0.5*x*x);   // gaussian
  if(i==2) return x*exp(-0.5*x*x);  // 1st derivative gaussian
}


/*
Linear growth factor routine:

This routine computes the linear growth factor Dplus(z),
not normalized at z = 0.

*/

double Dplusf(struct cmdline_data* cmd, struct  global_data* gd,
              double z)
// linear growth factor at z (not normalized at z=0)
{
    string routineName = "Dplusf";
  int i,j,n;
  double a,a0,x,h,yp;
  double k1[2],k2[2],k3[2],k4[2],y[2],y2[2],y3[2],y4[2];

    debug_tracking_s("001", routineName);
    debug_tracking_r("002", z);

  a=1./(1.+z);
  a0=1./1100.;

  yp=-1.;
  n=10;

  for(;;){
    n*=2;
    h=(log(a)-log(a0))/n;
  
    x=log(a0);
    y[0]=1., y[1]=0.;
    for(i=0;i<n;i++){
      for(j=0;j<2;j++) k1[j]=h*Dplusf_func(cmd, gd, j,x,y);

      for(j=0;j<2;j++) y2[j]=y[j]+0.5*k1[j];
      for(j=0;j<2;j++) k2[j]=h*Dplusf_func(cmd, gd, j,x+0.5*h,y2);

      for(j=0;j<2;j++) y3[j]=y[j]+0.5*k2[j];
      for(j=0;j<2;j++) k3[j]=h*Dplusf_func(cmd, gd, j,x+0.5*h,y3);

      for(j=0;j<2;j++) y4[j]=y[j]+k3[j];
      for(j=0;j<2;j++) k4[j]=h*Dplusf_func(cmd, gd, j,x+h,y4);
      
      for(j=0;j<2;j++) y[j]+=(k1[j]+k4[j])/6.+(k2[j]+k3[j])/3.;
      x+=h;
    }

    if(fabs(y[0]/yp-1.)<0.1*eps) break;
    yp=y[0];
  }

    debug_tracking_r("003", a*y[0]);
    debug_tracking_s("004... final", routineName);

    return a*y[0];
}

/*
Linear growth differential-equation routine:
*/

double Dplusf_func(struct cmdline_data* cmd, struct  global_data* gd,
                   int j, double la, double y[2])
{
  if(j==0) return y[1];
  
  double om, w, ow;
  om=cmd->Omm;
  ow=cmd->Omw;
  w=cmd->w;
  
  double g,a;
  a=exp(la);
  g=-0.5*(5.*om+(5.-3*w)*ow*pow(a,-3.*w))*y[1]-1.5*(1.-w)*ow*pow(a,-3.*w)*y[0];
  g=g/(om+ow*pow(a,-3.*w));
  if(j==1) return g;
}


/*
Interpolated linear power spectrum from input table:
*/

double linear_pkz0_data(struct cmdline_data* cmd, struct  global_data* gd,
                        double k)
// linear P(k) interpolated from the given table,  k[h/Mpc]  P(k)[(Mpc/h)^3]
{
  int j,j1,j2,jm;
  double lk,dlk,f;
  double norm=1.;


  lk=log10(k);
  if(k<gd->k_data[0]) return 0.;
  if(k>gd->k_data[gd->n_data-1]) return 0.;
  
  j1=0, j2=gd->n_data-1, jm=(j1+j2)/2;
  for(;;){
    if(k>gd->k_data[jm]) j1=jm;
    else j2=jm;
    jm=(j1+j2)/2;

    if(j2-j1==1) break;
  }
  j=j1;

  f=(log10(gd->pkz0_data[j+1])-log10(gd->pkz0_data[j]))/(log10(gd->k_data[j+1])
      -log10(gd->k_data[j]))*(lk-log10(gd->k_data[j]))+log10(gd->pkz0_data[j]);
  
  return norm*norm*pow(10.,f);
}

/*
Linear power spectrum selector:
*/
double linear_pkz0(struct cmdline_data* cmd, struct  global_data* gd,
                   double k)
// linear P(k)   k[h/Mpc], P(k)[(Mpc/h)^3]
{
  //~ if(gd.n_data!=0) return linear_pkz0_data(k);
  //~ else return linear_pkz0_eh(k);
    return linear_pkz0_data(cmd, gd, k);
}

/*
Eisenstein & Hu linear power spectrum routine:

This routine is intended to compute the Eisenstein & Hu (1999)
fitting formula without wiggles.
*/
double linear_pkz0_eh(struct cmdline_data* cmd,
                      struct  global_data* gd, double k)
// Eisenstein & Hu (1999) fitting formula without wiggle,
//  k[h/Mpc], P(k)[(Mpc/h)^3]
{
  
  double omc, om, h, ns,omb;
  
  omc=cmd->Omc;
  om=cmd->Omm;
  omb=cmd->Omb;
  h=cmd->h;
  ns=cmd->ns;
  
  k*=h;  // unit conversion from [h/Mpc] to [1/Mpc]
  
  return 0.0;
}

