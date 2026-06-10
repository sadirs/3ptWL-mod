//==========================================================================
//        1          2          3          4        ^ 5          6          7

/*
============================================================================
NAME: procedures.c                           [wlcf]
Written by: S. Aviles et al.
Starting date: February 2026
Purpose: Procedures for initialization, allocation, integration, I/O,
         interpolation, quadrature, and output writing
Language: C
*/
//==========================================================================


#include "functions.h"
#include "procedures.h"

#define m_PI   3.1415926535897932384626433

/* 
Initial routine:

To be called in main:
    Initial(&cmd, &gd);

This routine is in charge of the initialization of the main
quantities required by the code. It computes the linear growth
factor normalization, reads the input power spectrum, and
evaluates sigma8.

*/
int Initial(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "Initial";
    debug_tracking_s("001", routineName);

    gd->Dpz0 = Dplusf(cmd, gd, 0.0);
    // Dpz0: linear growth factor evaluated at redshift z = 0

    debug_tracking_r("002", gd->Dpz0);
    read_inputpk(cmd, gd);

    gd->sigma8 =  sigmaRTH(cmd, gd, 8,0.001,8.,100);
    // sigma8: rms matter fluctuation amplitude in spheres of radius 8 Mpc/h

    debug_tracking_s("003... final", routineName);

    return SUCCESS;
}

/* 
Allocation routine for internal vectors:
*/
int allocate_iv(struct cmdline_data* cmd, struct  global_data* gd)
{
    iv.Nell   = cmd->Nell;
    iv.ellmax = cmd->ellmax;
    iv.ellmin = cmd->ellmin;
    iv.ellT   = malloc(iv.Nell * sizeof(double *));
    
    for(int i=0; i<iv.Nell ; i++){
        iv.ellT[i]= exp(log(iv.ellmin)
        + i*log(iv.ellmax/iv.ellmin)/(iv.Nell-1.0));
    }

    int NumMoments=cmd->mMax+1;
    iv.BmVectors  = malloc(NumMoments * sizeof(double *));
    iv.BmVectorsp = malloc(NumMoments * sizeof(double *));
    for(int m=0; m<NumMoments; m++) {
        iv.BmVectors [m] = malloc(iv.Nell * iv.Nell * sizeof(double));
        iv.BmVectorsp[m] = malloc(iv.Nell * iv.Nell * sizeof(double));
        for(int ij=0; ij< iv.Nell*iv.Nell; ij++){
             iv.BmVectors [m][ij] = 0;
             iv.BmVectorsp[m][ij] = 0;
        }
    }

    return SUCCESS;
}


/*
Bm(ell1, ell2) integration routine:

Each OpenMP worker owns independent (ell1, ell2) cells and performs the
chi integral sequentially for that cell. This preserves trapezoidal order
while allowing the symmetric ell grid to be computed in parallel.
*/
int Bmell(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "Bmell";
    int NumMoments=cmd->mMax+1;
    int Nell = iv.Nell;
    int chiQuadSteps = iv.chiQuadSteps;
    int GLpoints = cmd->GLpoints;
    long total_cells = (long)Nell * (long)Nell;

/*    if(cmd->chatty>0){
        printf("\nComputing Bm(ell1,ell2) for symmetric %d x %d array of ell values  \n",
            iv.Nell, iv.Nell);
        printf("    ellmin = %f, ellmax = %f  \n",
            iv.ellmin, iv.ellmax);
        printf("    Number of moments = %d\n", cmd->mMax+1);
        printf("    Quadrature chi steps= %d\n", iv.chiQuadSteps);
    } */
    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
    "\n%s: Computing Bm(ell1,ell2) for symmetric %d x %d array of ell values\n",
    routineName, iv.Nell, iv.Nell);
    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                        "    ellmin = %f, ellmax = %f  \n",
                        iv.ellmin, iv.ellmax);
    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                        "    Number of moments = %d\n", cmd->mMax+1);
    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                        "    Quadrature chi steps= %d\n", iv.chiQuadSteps);

    double *xGL = malloc(GLpoints * sizeof(double));
    double *wGL = malloc(GLpoints * sizeof(double));
    gaussleg(-m_PI, m_PI, xGL, wGL, GLpoints);

    // Each cell owns its chi integral, preserving trapezoid order under OpenMP.
#ifdef OPENMPCODE
#pragma omp parallel for default(none) \
shared(cmd, gd, iv, NumMoments, Nell, chiQuadSteps, GLpoints, \
       total_cells, xGL, wGL) schedule(dynamic)
#endif
    for (long cell=0; cell<total_cells; cell++){
        int i = cell / Nell;
        int j = cell % Nell;
        if (j < i) continue;

        double ell1 = iv.ellT[i];
        double ell2 = iv.ellT[j];
        double integral[NumMoments];
        double previous[NumMoments];

        for(int m=0; m<NumMoments; m++)
            integral[m] = 0.0;

        for (int ichi=0; ichi<chiQuadSteps; ichi++){
            double chi    = iv.chiT_chiint[ichi];
            double z      = iv.zT_chiint[ichi];
            double Dp     = iv.DpT_chiint[ichi];
            double rsigma = iv.rsigma_chiint[ichi];
            double neff   = iv.neff_chiint[ichi];
            double qv     = iv.q_chiint[ichi];
            double k1 = ell1/chi;
            double k2 = ell2/chi;
            double val[NumMoments];
            double alphaEFT = -3.0;
            double EFTctr = alphaEFT*pow(Dp,2);

            for(int m=0; m<NumMoments; m++)
                val[m] = 0.0;

            for(int igl=0; igl<GLpoints; igl++){
                double varphi = xGL[igl];
                double w = wGL[igl];
                double k3 = sqrt(k1*k1 + k2*k2 - 2.*k1*k2 * cos(varphi));
                double BT;

                if (cmd->tree_level==1){
                    BT = Bispec_tree(cmd, gd, k1, k2, k3, Dp);
                } else if (cmd->tree_level==2){
                    BT = Bispec_P2(cmd, gd, k1, k2, k3, Dp);
                } else if (cmd->tree_level==3){
                    BT  = Bispec_tree_EFT(cmd, gd, k1, k2, k3, Dp, EFTctr);
                } else {
                    BT  = Bispec_Takahashi(cmd, gd, k1, k2, k3, z, Dp,
                                            rsigma, neff);
                }

                for(int m=0; m<NumMoments; m++)
                    val[m] += w*BT*cos(m*varphi);
            }

            double projection_weight = pow(qv,3.)/pow(chi,4.);
            double projected[NumMoments];
            for(int m=0; m<NumMoments; m++)
                projected[m] = projection_weight * val[m]/(2*m_PI);

            if (ichi == 0) {
                for(int m=0; m<NumMoments; m++) {
                    integral[m] += projected[m] * chi;
                    previous[m] = projected[m];
                }
            } else {
                double deltachi = chi - iv.chiT_chiint[ichi-1];
                for(int m=0; m<NumMoments; m++) {
                    integral[m] += 0.5*(previous[m] + projected[m]) * deltachi;
                    previous[m] = projected[m];
                }
            }
        }

        for(int m=0; m<NumMoments; m++){
            iv.BmVectorsp[m][i*Nell + j] = integral[m];
            if(j!=i) iv.BmVectorsp[m][j*Nell + i] = integral[m];
        }
    }

    free(xGL);
    free(wGL);

    // TEST
    char buf[BUFFERSIZE];
    FILE *fp;
    char str[100];
    sprintf(str,"%s/tests",cmd->path_Bells);
    sprintf(buf,"if [ ! -d %s ]; then mkdir %s; fi", str, str);
    system(buf);
    sprintf(str,"%s/tests/i_z_chi_qv_Dp.txt",cmd->path_Bells);
    fp = fopen (str, "w+");
    if (fp != NULL) {
        for(int i=0; i<iv.chiQuadSteps ; i++){
            fprintf(fp, "%d %15e %15e %15e %15e \n",
                    i, iv.zT_chiint[i], iv.chiT_chiint[i],
                    iv.q_chiint[i], iv.DpT_chiint[i]);
        }
        fclose (fp);
    } else {
        verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "\n%s: warning!! file %s can not be opened.\n",
                               routineName, str);
    }

    return SUCCESS;
}

/* 
Single-chi bispectrum projection routine:

*/
int Bm(struct cmdline_data* cmd, struct  global_data* gd,
        double chi, double z, double Dp, double r_sigma, double n_eff)
{
    double k1,k2,varphi,w,k3,BT, ell1,ell2;
    double *xGL, *wGL;
    int m=0;
    int NumMoments=cmd->mMax+1;
    double val[NumMoments];
    double EFTctr;
    double alphaEFT;
    
    xGL = malloc(cmd->GLpoints * sizeof(double));
    wGL = malloc(cmd->GLpoints * sizeof(double));
    gaussleg(-m_PI, m_PI, xGL, wGL, cmd->GLpoints);

    alphaEFT=-3.0;  // =cmd.alphaEFT
//  alphaEFT: EFT parameter
    EFTctr=alphaEFT*pow(Dp,2);
//   EFTctr  : EFT counterterm contribution

    
    for(int i=0; i<iv.Nell; i++){
    for(int j=i; j<iv.Nell; j++){
        ell1=iv.ellT[i];
        ell2=iv.ellT[j];
        k1 = ell1/chi;
        k2 = ell2/chi;
        for(int m=0;m<NumMoments;m++) val[m] = 0.0;
        
        for(int i=0; i<cmd->GLpoints;i++){
            varphi = xGL[i];
            w   = wGL[i];
            k3  = sqrt( k1*k1 + k2*k2 - 2.*k1*k2 * cos(varphi) );
            if (cmd->tree_level==1){
                BT = Bispec_tree(cmd, gd, k1, k2, k3, Dp);
            } else if (cmd->tree_level==2){
                BT = Bispec_P2(cmd, gd, k1, k2, k3, Dp);
            } else if (cmd->tree_level==3){
                BT  = Bispec_tree_EFT(cmd, gd, k1, k2, k3, Dp, EFTctr);
            } else {
                BT  = Bispec_Takahashi(cmd, gd, k1, k2, k3, z, Dp, r_sigma, n_eff);
            }
            for(int m=0;m<NumMoments;m++) val[m] =  val[m] + w*BT*cos(m*varphi);
        }

        for(int m=0; m<NumMoments; m++){
            iv.BmVectors[m][i*iv.Nell + j] = val[m]/(2*m_PI);
            if(j!=i) iv.BmVectors[m][j*iv.Nell + i] = val[m]/(2*m_PI);
        }
    }
    }

    return SUCCESS;
}

/* 
k-space bispectrum table routine:
*/
int BmKspace(struct cmdline_data* cmd, struct  global_data* gd,
              int Maxm, double kmin, double kmax, int Nk,
              int GLpoints, double z, double Dp, double r_sigma, double n_eff)
{
    if(cmd->chatty>0) printf("\nComputing Bm(k1,k2) for symmetric array of %d x %d k1,k2 values  \n", Nk, Nk);
    
    double k1,k2,varphi,w,k3,BT;
    double *kT,*xGL, *wGL;
    double mat[Maxm+1][Nk][Nk];
    double val[Maxm+1];
    int m=0;

    kT  = malloc(Nk       * sizeof(double));
    xGL = malloc(GLpoints * sizeof(double));
    wGL = malloc(GLpoints * sizeof(double));
    
    gaussleg(-m_PI, m_PI, xGL, wGL, GLpoints);
    
    if(cmd->chatty==2)     printf("Maxm=%d, kmin=%f ,kmax=%f, Nk=%d, GLpoints=%d \n",
            Maxm,    kmin,    kmax,    Nk,    GLpoints);
    if(cmd->chatty==2) printf("z=%f, Dp=%f, rsigma=%f, neff=%f \n", z,Dp,r_sigma,n_eff);
    
    for(int i=0; i<Nk ; i++){
        kT[i]= exp(log(kmin) + i*log(kmax/kmin)/(Nk-1.0));
    }

    gv.time=clock();

    for(int i=0; i<Nk ; i++){
    for(int j=i; j<Nk ; j++){
        k1=kT[i];
        k2=kT[j];
        for(int m=0;m<=Maxm;m++) val[m] = 0.0;
        
        for(int i=0; i<GLpoints;i++){
            varphi = xGL[i];
            w   = wGL[i];
            k3  = sqrt( k1*k1 + k2*k2 - 2.*k1*k2 * cos(varphi) );
            BT  = Bispec_Takahashi(cmd, gd, k1, k2, k3, z, Dp, r_sigma, n_eff);
            for(int m=0;m<=Maxm;m++) val[m] =  val[m] + w*BT*cos(m*varphi);
        }

        for(int m=0; m<=Maxm; m++){
            mat[m][i][j] = val[m];
            if(j!=i) mat[m][j][i]=val[m];
        }
        
    }
    }
    
    if(cmd->chatty==2) printf("Bnk time = %f15 \n", (double)(clock()-gv.time) / CLOCKS_PER_SEC  );
    if(cmd->chatty==2) printf("\n");
    
    // Write
    char int_str[20];
    char str[100];
    for (int m=0; m<Maxm+1; m++){
        FILE *fp;
        sprintf(int_str, "%d", m);
        sprintf(str,"%s/%sBnk_%s.txt",cmd->path_Bells,cmd->prefix,int_str);
        fp = fopen (str, "w+");
        
        for(int i=0; i<Nk ; i++){
        for(int j=0; j<Nk ; j++){
            fprintf(fp, "%15e   ", mat[m][i][j]);
            if(j==Nk-1 && i!=Nk-1) fprintf(fp, " \n");
        }
        }
        
        fclose (fp);
    }

    FILE *fp;
    sprintf(str,"%s/%skArray.txt",cmd->path_Bells,cmd->prefix);
    fp = fopen (str, "w+");
    for(int i=0; i<Nk ; i++){
        fprintf(fp,"%15e\n", kT[i]);
    }
    fclose (fp);

    return SUCCESS;
}



//B Routines...

#define EPS 3.0e-11




/* 
Gauss-Legendre quadrature routine:
*/
void gaussleg(double x1, double x2, double xGL[], double wGL[], int n)
{
    int m,j,i;
    double z1,z,xm,xl,pp,p3,p2,p1;
    
    m=(n+1)/2;
    xm=0.5*(x2+x1);
    xl=0.5*(x2-x1);
    for (i=1;i<=m;i++){
        z=cos(m_PI*(i-0.25)/(n+0.5));
        do {
            p1=1.0;
            p2=0.0;
            for (j=1;j<=n;j++) {
                p3=p2;
                p2=p1;
                p1=((2.0*j-1.0)*z*p2-(j-1.0)*p3)/j;
            }
            pp=n*(z*p1-p2)/(z*z-1.0);
            z1=z;
            z=z1-p1/pp;
        } while (fabs(z-z1) > EPS);
        xGL[i-1]=xm-xl*z;
        xGL[n+1-i-1]=xm+xl*z;
        wGL[i-1]=2.0*xl/((1.0-z*z)*pp*pp);
        wGL[n+1-i-1]=wGL[i-1];
    }
}
#undef EPS


/* 
Read and extrapolate input linear power spectrum:
*/
int read_inputpk(struct cmdline_data* cmd, struct  global_data* gd)
//Extrapolation not yet implemented
{
    FILE *fp;
    gd->n_data=0;
    fp=fopen(cmd->fnamePS,"r");   // linear P(k) table
    
    if (NULL == fp) {
        printf("\n\nlinear power spectrum can't be opened \n\n");
    }
    
    if(fp!=NULL){   // input: k[h/Mpc]   P(k)[(Mpc/h)^3]
        while(fscanf(fp, "%lf %lf", &gd->k_data[gd->n_data], &gd->pkz0_data[gd->n_data])!=EOF){
            gd->n_data++;
            if(gd->n_data>n_data_max) printf("n_data_max should be larger than the number of data lines \n");
        }
        fclose(fp);
    }

    return SUCCESS;
}


/* 
Linear interpolation routine:

*/
double interpolation1(double x, double xT[], double yT[], int n_data)   // interpolation order 1
{
  int j,j1,j2,jm;
  double f;

  if(x<xT[0]) return 0.;
  if(x>xT[n_data-1]) return 0.;
  
  j1=0, j2=n_data-1, jm=(j1+j2)/2;
  for(;;){
    if(x>xT[jm]) j1=jm;
    else j2=jm;
    jm=(j1+j2)/2;

    if(j2-j1==1) break;
  }
  j=j1;

  f=(yT[j+1]-yT[j])/(xT[j+1]-xT[j]) * (x -xT[j])+ yT[j];
  
  return f;
}

/* 
Logarithmic interpolation routine:
*/
double interpolationlog(double x, double xT[], double yT[], int n_data)   // interpolation in log space
{
  int j,j1,j2,jm;
  double f;

  if(x<xT[0]) return 0.;
  if(x>xT[n_data-1]) return 0.;
  
  j1=0, j2=n_data-1, jm=(j1+j2)/2;
  for(;;){
    if(x>xT[jm]) j1=jm;
    else j2=jm;
    jm=(j1+j2)/2;

    if(j2-j1==1) break;
  }
  j=j1;

  f=(log10(yT[j+1])-log10(yT[j]))/(log10(xT[j+1])
      -log10(xT[j]))*(log10(x)-log10(xT[j]))+log10(yT[j]);
  
  return pow(10.,f);
}

//E
//B Write

/* 
Output writing routine:
*/
int write(struct cmdline_data* cmd, struct  global_data* gd)
{
    double elli, ellj;
    char int_str[20];
    char str[100];
    
    for (int m=0; m<cmd->mMax+1; m++){
        FILE *fp;
        sprintf(int_str, "%d", m);
        sprintf(str,"%s/%sBmells_%s.txt",cmd->path_Bells,cmd->prefix,int_str);
        sprintf(int_str, "%d", m);
        fp = fopen (str, "w+");
        
        for(int i=0; i<iv.Nell ; i++){
            for(int j=0; j<iv.Nell ; j++){
                fprintf(fp, "%15e ", iv.BmVectorsp[m][i*iv.Nell + j]);
                if(j==iv.Nell-1 && i!=iv.Nell-1) fprintf(fp, " \n");
            }
        }
        fclose (fp);
    }
        
    
    FILE *fp2;
    sprintf(str,"%s/%sellArray.txt",cmd->path_Bells,cmd->prefix);
    fp2 = fopen (str, "w+");
    for(int i=0; i<iv.Nell ; i++){
        fprintf(fp2,"%15e\n", iv.ellT[i]);
    }
    fclose (fp2);
        
    FILE *fp3;
    sprintf(str,"%s/%sinfo.txt",cmd->path_Bells,cmd->prefix);
    fp3 = fopen (str, "w+");
    fprintf(fp3, "Cosmological Parameters: \n");
    fprintf(fp3, "    Omega_m = %f\n",cmd->Omm);
    fprintf(fp3, "         ns = %f\n",cmd->ns);
    fprintf(fp3, "\n");
    fprintf(fp3, "Computing Bm(ell1,ell2) for %d x %d array  \n",
            iv.Nell, iv.Nell);
    fprintf(fp3, "    ellmin = %f, ellmax = %f  \n",
            iv.ellmin, iv.ellmax);
    fprintf(fp3, "    Number of moments = %d\n", cmd->mMax+1);
    fprintf(fp3, "    Quadrature chi steps= %d\n", iv.chiQuadSteps);
    fclose (fp3);

    if (cmd->writevectors ==1){
    for (int m=0; m<cmd->mMax+1; m++){
        FILE *fp;
        sprintf(int_str, "%d", m);
        sprintf(str,"%s/%sBmellsVector_%s.txt",cmd->path_Bells,cmd->prefix,int_str);
        sprintf(int_str, "%d", m);
        fp = fopen (str, "w+");
        
        for(int ij=0; ij<iv.Nell*iv.Nell ; ij++){
            fprintf(fp, "%e \n", iv.BmVectorsp[m][ij]);
        }
        fclose (fp);
    }
    }

    return SUCCESS;
}

//E


//B free variables

//~ void free_variables(void)
//~ {
        //~ for (int m=0; m<cmd.mMax+1; m++){
            //~ free(iv.BmVectorsp[m]);
            //~ free(iv.BmVectors [m]);
        //~ }
        //~ free(iv.BmVectorsp);
        //~ free(iv.BmVectors );
//~ }

//E
