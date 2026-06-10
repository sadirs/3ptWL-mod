//==========================================================================
//        1          2          3          4        ^ 5          6          7

/*
============================================================================
NAME: startrun.c                             [wlcf]
Written by: Mario A. Rodriguez-Meza
Starting date: February 2026
Purpose: Routines to initialize the main code
Language: C
*/
//==========================================================================

//
// We must check the order of memory allocation and deallocation!!!
// Here and in EndRun in cballsio.c
//

//
// lines where there is a "//B socket:" string are places to include module files
//  that can be found in addons/addons_include folder
//
#include "global.h"

local void ReadParameterFile(struct  cmdline_data*, struct  global_data*, char *);
local int startrun_parameterfile(struct  cmdline_data*, struct  global_data*);
local int startrun_cmdline(struct  cmdline_data*, struct  global_data*);
local void ReadParametersCmdline(struct  cmdline_data*, struct  global_data*);
local void ReadParametersCmdline_short(struct  cmdline_data*, 
                                       struct  global_data*);
local int CheckParameters(struct  cmdline_data*, struct  global_data*);

//B Special routines to scan command line
local int startrun_getParamsSpecial(struct  cmdline_data*, struct  global_data*);
local int scaniOption(struct  cmdline_data*, struct  global_data*,
                      string, int *, int *, int, int, string);
local int scanrOption(struct  cmdline_data*, struct  global_data*,
                      string, double *, int *, int, int, string);
//E
//B I/O directories:
local void setFilesDirs_log(struct cmdline_data*, struct  global_data* gd);
local void setFilesDirs(struct cmdline_data*, struct  global_data* gd);
//E
local int print_make_info(struct cmdline_data* cmd,
                     struct  global_data* gd);

//B socket:
#ifdef ADDONS
#include "startrun_include_00.h"
#endif
//E

/*
 StartRun routine:
 */
#ifndef CLASSLIB
int StartRun(struct  cmdline_data* cmd, struct  global_data* gd, 
             string head0, string head1, string head2, string head3)
{
    string routineName = "StartRun";
    double cpustart = CPUTIME;

    gd->headline0 = head0; gd->headline1 = head1;
    gd->headline2 = head2; gd->headline3 = head3;

    printf("\n%s\n%s: %s\n\t %s\n",
           gd->headline0, gd->headline1, gd->headline2, gd->headline3);
    printf("Version = %s\n", getversion());

    //B move all these to Startrun_Common... or make an appropriate change
    gd->cmd_allocated = FALSE;
    gd->cputotalinout = 0.;
    gd->cputotal = 0.;
    gd->bytes_tot = 0;
    //E

    cmd->paramfile = GetParam("paramfile");
    if (*(cmd->paramfile)=='-')
        error("bad parameter %s\n", cmd->paramfile);
    if (!strnull(cmd->paramfile))
		startrun_parameterfile(cmd, gd);
    else
		startrun_cmdline(cmd, gd);

    gd->bytes_tot += sizeof(struct  global_data);
    gd->bytes_tot += sizeof(struct cmdline_data);
    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                "\n%s: Total allocated %g MByte storage so far.\n",
                        routineName, gd->bytes_tot*INMB);

//B If uncommented there will be a warning in the setup.py process
//#ifdef OPENMPCODE
    class_call_cballs(SetNumberThreads(cmd), errmsg, errmsg);
//#endif
//E
    gd->cputotalinout += CPUTIME - cpustart;
    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                        "\n%s CPU time: %g %s\n",
                        routineName, CPUTIME - cpustart, PRNUNITOFTIMEUSED);

    return SUCCESS;
}

#else // ! CLASSLIB

#include "input.h"

int StartRun(struct  cmdline_data* cmd, struct  global_data* gd,
             string head0, string head1, string head2, string head3)
{
    string routineName = "StartRun";
    struct file_content fc;

    double cpustart = CPUTIME;
    
    gd->headline0 = head0; gd->headline1 = head1;
    gd->headline2 = head2; gd->headline3 = head3;
    printf("\n%s\n%s: %s\n\t %s\n",
           gd->headline0, gd->headline1, gd->headline2, gd->headline3);
    printf("Version = %s\n", getversion());

    //B move all these to Startrun_Common... or make an appropriate change
    gd->cmd_allocated = FALSE;
//    gd->stopflag = 0;
    gd->cputotalinout = 0.;
    gd->cputotal = 0.;
    gd->bytes_tot = 0;
//    gd->sameposcount = 0;
    //E

    cmd->paramfile = GetParam("paramfile");
    if (*(cmd->paramfile)=='-')
        error("bad parameter %s\n", cmd->paramfile);
    if (!strnull(cmd->paramfile)) {
        class_call_cballs(input_find_file(cmd, gd, cmd->paramfile, &fc, errmsg),
                          errmsg, errmsg);
        class_call_cballs(input_read_from_file(cmd, gd, &fc, errmsg),
                          errmsg, errmsg);
        class_call_cballs(parser_free(&fc), errmsg, errmsg);
    } else {
        startrun_cmdline(cmd, gd);
    }

    if (!strnull(cmd->paramfile))
        class_call_cballs(StartRun_Common(cmd, gd), errmsg, errmsg);

        if (!strnull(cmd->paramfile))
            PrintParameterFile(cmd, gd, cmd->paramfile);

    gd->bytes_tot += sizeof(struct  global_data);
    gd->bytes_tot += sizeof(struct cmdline_data);
    verb_print(cmd->verbose,
               "\n%s: Total allocated %g MByte storage so far.\n",
               routineName, gd->bytes_tot*INMB);

//#ifdef OPENMPCODE
    class_call_cballs(SetNumberThreads(cmd), errmsg, errmsg);
//#endif

    gd->cputotalinout += CPUTIME - cpustart;
    verb_print(cmd->verbose, "\n%s CPU time: %g %s\n",
               routineName, CPUTIME - cpustart, PRNUNITOFTIMEUSED);

    return SUCCESS;
}
#endif // ! CLASSLIB

/*
Parameter-file startup routine:
*/
local int startrun_parameterfile(struct  cmdline_data* cmd,
                                 struct  global_data* gd)
{
	ReadParameterFile(cmd, gd, cmd->paramfile);
    ReadParametersCmdline_short(cmd, gd);

//B socket:
#ifdef ADDONS
#include "startrun_include_01.h"
#endif
//E

	StartRun_Common(cmd, gd);
    PrintParameterFile(cmd, gd, cmd->paramfile);

    return SUCCESS;
}


#define parameter_null	"parameters_null-cballs"

//B Section for reading parameters from the command line

/*
Command-line startup routine:
*/
local int startrun_cmdline(struct  cmdline_data* cmd, struct  global_data* gd)
{
	ReadParametersCmdline(cmd, gd);
	StartRun_Common(cmd, gd);
//    if (gd->flagPrint==TRUE && gd->rootDirFlag==TRUE) {
        PrintParameterFile(cmd, gd, parameter_null);
//    }

    return SUCCESS;
}
/*
Read full command-line parameters routine:
*/
local void ReadParametersCmdline(struct  cmdline_data* cmd, 
                                 struct  global_data* gd)
{
// Every item in cmdline_defs.h must have an item here::

    //B Parameters related to background cosmology
    cmd->z = GetdParam("z");
    cmd->h = GetdParam("h");
    cmd->sigma8 = GetdParam("sigma8");
    cmd->Omb = GetdParam("Omb");
    cmd->Omc = GetdParam("Omc");
    cmd->ns = GetdParam("ns");
    cmd->w = GetdParam("w");
    cmd->Omnu = GetdParam("Omnu");
    //E

    //B Parameters for I/O
    cmd->rootDir = GetParam("rootDir");
    cmd->fnamePS = GetParam("fnamePS");
    cmd->prefix = GetParam("prefix");
    cmd->path_Bells = GetParam("path_Bells");
    cmd->tree_level = GetiParam("tree_level");
    cmd->zbin = GetdParam("zbin");
    cmd->mMax = GetiParam("mMax");
    cmd->chiQuadSteps = GetiParam("chiQuadSteps");
    cmd->GLpoints = GetiParam("GLpoints");
    cmd->Nell = GetiParam("Nell");
    cmd->ellmax = GetdParam("ellmax");
    cmd->ellmin = GetdParam("ellmin");
    cmd->Wg = GetiParam("Wg");
    cmd->fWgchi = GetParam("fWgchi");
    //E

    //B Parameters to control histograms and their output files
    //E

    //B Miscellaneous parameters
    cmd->writevectors = GetiParam("writevectors");
//    cmd->writevectors = GetbParam("writevectors");
    cmd->chatty = GetiParam("chatty");
    cmd->verbose = GetiParam("verbose");
    cmd->verbose_log = GetiParam("verbose_log");
#ifdef OPENMPCODE
    cmd->numthreads = GetiParam("numberThreads");
#endif
    cmd->options = GetParam("options");
    //E

//B socket:
#ifdef ADDONS
#include "startrun_include_02.h"
#endif
//E
}
/*
Read short command-line parameters routine:
*/

local void ReadParametersCmdline_short(struct  cmdline_data* cmd, struct  global_data* gd)
{
//B Here add parameters needed to be read after reading parameter file
    //B Miscellaneous parameters
//    cmd->preScript = GetParam("preScript");
//    cmd->posScript = GetParam("posScript");
    //E
//E
}

//E

#undef parameter_null

//B Section of parameter reading from a file

/*
Parameter-file reading routine:
*/
local void ReadParameterFile(struct  cmdline_data* cmd, 
                             struct  global_data* gd, char *fname)
{
// Every item in cmdline_defs.h must have an item here::
#define DOUBLE 1
#define STRING 2
#define INT 3
#define LONG 6
#define BOOLEAN 4
#define MAXTAGS 300
#define MAXCHARBUF 1024

    string routineName = "ReadParameterFile";
    FILE *fd;

  int  i,j,nt;
  int  id[MAXTAGS];
  void *addr[MAXTAGS];
  char tag[MAXTAGS][50];
  int  errorFlag=0;

    int input_verbose = 2;
    verb_print(input_verbose, "\nparsing input parameters...\n");

  nt=0;

    //B Parameters related to the cosmological background
    RPName(cmd->z,"z");
    RPName(cmd->h,"h");
    RPName(cmd->sigma8,"sigma8");
    RPName(cmd->Omb,"Omb");
    RPName(cmd->Omc,"Omc");
    RPName(cmd->ns,"ns");
    RPName(cmd->w,"w");
    RPName(cmd->Omnu,"Omnu");
    //E

    //B Parameters to control the I/O file(s)
    // Input catalog parameters
    SPName(cmd->fnamePS,"fnamePS",MAXLENGTHOFSTRSCMD);
    // Output parameters
    SPName(cmd->prefix,"prefix",MAXLENGTHOFSTRSCMD);
    SPName(cmd->rootDir,"rootDir",MAXLENGTHOFSTRSCMD);
    SPName(cmd->path_Bells,"path_Bells",MAXLENGTHOFSTRSCMD);
    IPName(cmd->tree_level,"tree_level");
    RPName(cmd->zbin,"zbin");
    IPName(cmd->mMax,"mMax");
    //E

    //B Set of parameters needed to integrate
    IPName(cmd->chiQuadSteps,"chiQuadSteps");
    IPName(cmd->GLpoints,"GLpoints");
    IPName(cmd->Nell,"Nell");
    RPName(cmd->ellmax,"ellmax");
    RPName(cmd->ellmin,"ellmin");

    IPName(cmd->Wg,"Wg");
    SPName(cmd->fWgchi,"fWgchi",MAXLENGTHOFSTRSCMD);
    //E

    //B Miscellaneous parameters
    IPName(cmd->writevectors,"writevectors");
//    SPName(cmd->preScript,"preScript",MAXLENGTHOFSTRSCMD);
//    SPName(cmd->posScript,"posScript",MAXLENGTHOFSTRSCMD);
    IPName(cmd->chatty,"chatty");
    IPName(cmd->verbose,"verbose");
    IPName(cmd->verbose_log,"verbose_log");
#ifdef OPENMPCODE
    IPName(cmd->numthreads,"numberThreads");
#endif
    SPName(cmd->options,"options",MAXLENGTHOFSTRSCMD);
    //E

//B socket:
#ifdef ADDONS
#include "startrun_include_03.h"
#endif
//E

    size_t slen;
    char *script1;
    char *script2;
    char *script3;
    char *script4;
    char buf4[MAXCHARBUF];
    char buf5[MAXCHARBUF];

//B
#ifndef _LINE_LENGTH_MAX_
#define _LINE_LENGTH_MAX_ 1024
#endif
#define _ARGUMENT_LENGTH_MAX_ 1024
        char line[_LINE_LENGTH_MAX_];
        char name[_ARGUMENT_LENGTH_MAX_];
        char value[_ARGUMENT_LENGTH_MAX_];
        char * phash;
        char * pequal;
        char * left;
        char * right;
//E

    if((fd=fopen(fname,"r"))) {
        while(!feof(fd)) {
//B
            fgets(line,MAXCHARBUF,fd);

            pequal=strchr(line,'=');
            if (pequal == NULL)
                continue;
            phash=strchr(line,'#');
            if ((phash != NULL) && (phash-pequal<2))
                continue;
            phash=strchr(line,'%');
            if ((phash != NULL) && (phash-pequal<2))
                continue;

            left=line;
            while (left[0]==' ') {
              left++;
            }
            if(left[0]=='\'' || left[0]=='\"'){
              left++;
            }
            right=pequal-1;
            while (right[0]==' ') {
              right--;
            }
            if(right[0]=='\'' || right[0]=='\"'){
              right--;
            }

            if (right-left < 0) {
                fprintf(stdout,
        "Error in file %s: there is no variable name before '=' in line: '%s'\n",
                    fname, line);
                errorFlag=1;
                continue;
            }

            strncpy(name,left,right-left+1);
            name[right-left+1]='\0';

            left = pequal+1;
            while (left[0]==' ') {
              left++;
            }

            if (phash == NULL)
              right = line+strlen(line)-1;
            else
              right = phash-1;

            while (right[0]<=' ') {
              right--;
            }

            if (right-left < 0)
                continue;

            strncpy(value,left,right-left+1);
            value[right-left+1]='\0';
//E

            for(i=0,j=-1;i<nt;i++)
                if(strcmp(name,tag[i])==0) {
                    j=i;
                    tag[i][0]=0;
                    break;
                }
            if(j>=0) {
                switch(id[j]) {
                    case DOUBLE:
                        *((double*)addr[j])=atof(value);
                        break;
                    case STRING:
                        if (strcmp(name,"preScript") == 0){ // To remove both '"'
                            int index;
                            size_t slen;
                            slen = strlen(value);
                            //B
                            script1 = (char*) malloc(1*sizeof(char));
                            memcpy(script1,value,1);
                            script2 = strchr(script1, '"');
                            if (script2 == NULL)
                                error("preScript parameter needs enclosing script with \"\"!! (%s)\n\n",
                                      value);
                            free(script1);
                            //E
                            //B
                            script1 = (char*) malloc((slen-1)*sizeof(char));
                            memcpy(script1,value+1,slen-1);
                            script2 = strchr(script1, '"');
                            if (script2 == NULL)
                                error("preScript parameter needs enclosing script with \"\"!! (%s)\n\n",
                                      value);
                            free(script1);
                            //E
                            cmd->preScript = (char*) malloc((slen-2)*sizeof(char));
                            script1 = (char*) malloc(slen*sizeof(char));
                            memcpy(script1,value,slen);
                            script2 = strchr(script1, '"');
                            if (script2 == NULL)
                                error("preScript parameter needs enclosing script with \"\"!! (%s)\n\n",
                                      script1);
                            memcpy(cmd->preScript,script2+1,slen-2);
                            free(script1);
                        } else {
                            if (strcmp(name,"posScript")==0){// To remove both '"'
                                int index;
                                size_t slen;
                                slen = strlen(value);
                                //B
                                script1 = (char*) malloc(1*sizeof(char));
                                memcpy(script1,value,1);
                                script2 = strchr(script1, '"');
                                if (script2 == NULL)
                                    error("posScript parameter needs enclosing script with \"\"!! (%s)\n\n",
                                          value);
                                free(script1);
                                //E
                                //B
                                script1 = (char*) malloc((slen-1)*sizeof(char));
                                memcpy(script1,value+1,slen-1);
                                script2 = strchr(script1, '"');
                                if (script2 == NULL)
                                    error("posScript parameter needs enclosing script with \"\"!! (%s)\n\n",
                                          value);
                                free(script1);
                                //E
                                cmd->posScript=(char*) malloc((slen-2)*sizeof(char));
                                script1 = (char*) malloc(slen*sizeof(char));
                                memcpy(script1,value,slen);
                                script2 = strchr(script1, '"');
                                if (script2 == NULL)
                                    error("posScript parameter needs enclosing script with \"\"!! (%s)\n\n",
                                          script1);
                                memcpy(cmd->posScript,script2+1,slen-2);
                                free(script1);
                            } else
                                strcpy(addr[j],value);
                        }
                        break;
                    case INT:
                        *((int*)addr[j])=atoi(value);
                        break;
                    case LONG:
                        *((long*)addr[j])=atol(value);
                        break;
                    case BOOLEAN:
                        if (strchr("tTyY1", *value) != NULL) {
                            *((bool*)addr[j])=TRUE;
                        } else
                            if (strchr("fFnN0", *value) != NULL)  {
                                *((bool*)addr[j])=FALSE;
                            } else {
                                error("getbparam: %s=%s not bool\n",name,value);
                            }
                        break;
                }
            } else {
                fprintf(stdout, "\n%s: Error in file %s: Tag '%s' %s\n",
                        routineName, fname, name,
                        "not allowed or multiple defined...");
//                        "look at saved parameter file which value was used");
                errorFlag=1;
            }
        } // ! while loop
        fclose(fd);
    } else {
        fprintf(stdout,"Parameter file %s not found.\n", fname);
        errorFlag=2;
        exit(0);
    }

    if (errorFlag==1)
        error("%s: going out\n", routineName);

    for(i=0;i<nt;i++) {
        if(*tag[i]) {
            if (cmd->verbose>2)
                fprintf(stdout,
                    "Warning! I miss a value for tag '%s' in parameter file '%s'.\n",
                    tag[i],fname);
            switch(id[i]) {
                case DOUBLE:
                    *((double*)addr[i])=GetdParam(tag[i]);
                    break;
                case STRING:
                    strcpy(addr[i],GetParam(tag[i]));
                    break;
                case INT:
                    *((int*)addr[i])=GetiParam(tag[i]);
                    break;
                case LONG:
                    *((long*)addr[i])=GetlParam(tag[i]);
                    break;
                case BOOLEAN:
                    *((bool*)addr[i])=GetbParam(tag[i]);
                    break;
            }
            errorFlag=1;
        }
    }

#undef DOUBLE
#undef STRING
#undef INT
#undef BOOLEAN
#undef MAXTAGS
#undef MAXCHARBUF
}
//E
/*
Common startup routine:
*/
int StartRun_Common(struct  cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "StartRun_Common";
    double cpustart;
    double cpustartMiddle;

    debug_tracking_s("001", routineName);

    if (strlen(cmd->rootDir)==0 || strnull(cmd->rootDir))
        gd->rootDirFlag = FALSE;
    else
        gd->rootDirFlag = TRUE;

    gd->flagPrint = TRUE;

    if (scanopt(cmd->options, "make-info"))
        print_make_info(cmd, gd);

//B socket:
#ifdef ADDONS
#include "startrun_include_04.h"
#endif
//E

    debug_tracking("002");
    debug_tracking("003");
    class_call_cballs(StartOutput(cmd, gd), errmsg, errmsg);
    debug_tracking("004");
    setFilesDirs(cmd, gd);
    setFilesDirs_log(cmd, gd);
    strcpy(gd->mode,"w");
    if (cmd->verbose_log>0) {               // gd->outlog is defined
        if(!(gd->outlog=fopen(gd->logfilePath, gd->mode)))
            error("\n%s: error opening file '%s' \n",
                  routineName, gd->logfilePath);
    }

    class_call_cballs(startrun_getParamsSpecial(cmd, gd), errmsg, errmsg);
    class_call_cballs(CheckParameters(cmd, gd), errmsg, errmsg);
    class_call_cballs(startrun_memoryAllocation(cmd, gd), errmsg, errmsg);

    cmd->Omm=cmd->Omb+cmd->Omc+cmd->Omnu;
    cmd->Omw=1.-cmd->Omm;
    verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                          "%s: OmegaM = : %f\n",
                           routineName, cmd->Omm);

//B socket:
#ifdef ADDONS
#include "startrun_include_05.h"
#endif
//E

    debug_tracking_s("005... final", routineName);

    return SUCCESS;
}

//B Section of parameter check

/*
Parameter checking routine:
*/
local int CheckParameters(struct  cmdline_data* cmd, struct  global_data* gd)
{
// If it is necessary: an item in cmdline_defs.h must have an item here::
    string routineName = "CheckParameters";

    debug_tracking_s("001", routineName);

    //B Parameters related to the cosmological background
    if (cmd->z < 0.0)
        error("\n%s: Redshift (%g) can not be less than 0.\n",
              routineName, cmd->z);
    if (cmd->h < 0.0)
        error("\n%s: Hubble parameter (%g) can not be less than 0.\n",
              routineName, cmd->h);
    if (cmd->sigma8 < 0.0)
        error("\n%s: sigma8 (%g) can not be less than 0.\n",
              routineName, cmd->sigma8);
    if (cmd->Omb < 0.0)
        error("\n%s: Omega baryon parameter (%g) can not be less than 0.\n",
              routineName, cmd->Omb);
    if (cmd->Omc < 0.0)
        error("\n%s: Omega CDM parameter (%g) can not be less than 0.\n",
              routineName, cmd->Omc);
    if (cmd->Omnu < 0.0)
        error("\n%s: Omega nu parameter (%g) can not be less than 0.\n",
              routineName, cmd->Omnu);
    if (cmd->ns < 0.0)
        error("\n%s: Spectral index parameter ns (%g) can not be less than 0.\n",
              routineName, cmd->ns);
//    if (cmd->w < 0.0)
//        error("\n%s: Equation of state parameter w (%g) can not be less than 0.\n",
//             routineName, cmd->w);
    //E

//B socket:
#ifdef ADDONS
#include "startrun_include_07.h"
#endif
//E
    debug_tracking_s("002... final", routineName);

    return SUCCESS;
}
//E


#define FMTT    "%-35s = %s\n"
#define FMTTS    "%-35s = \"%s\"\n"
#define FMTI    "%-35s = %d\n"
#define FMTIL    "%-35s = %ld\n"
#define FMTR	"%-35s = %g\n"

//B Section of parameter writing to a file

/*
Parameter-file writing routine:
*/
int PrintParameterFile(struct  cmdline_data *cmd,
                       struct  global_data* gd, char *fname)
{
// Every item in cmdline_defs.h must have an item here::
    string routineName = "PrintParameterFile";

    FILE *fdout;
    char buf[200];
    int  errorFlag=0;

    debug_tracking_s("001", routineName);

    if (gd->flagPrint==TRUE && gd->rootDirFlag==TRUE) {
        //B Look for "/" if fname is composed: path and filename
        int ndefault = 0;
        int ipos;
        char *dp;
        for (int i; i< strlen(fname); i++) {
            if(fname[i] == '/') {
                ipos = i+1;
                ndefault++;
            }
        }
        
        if (ndefault == 0) {
            sprintf(buf,"%s/%s%s",cmd->rootDir,fname,"-usedvalues");
        } else {
            dp = (char*) malloc((strlen(fname)-ipos)*sizeof(char));
            strncpy(dp, fname + ipos, strlen(fname)-ipos);
            verb_print_q(3,cmd->verbose,
                         "PrintParameterFile: '/' counts %d pos %d and %s\n",
                         ndefault, ipos, dp);
            sprintf(buf,"%s/%s%s",cmd->rootDir,dp,"-usedvalues");
        }
        //E
        
        if(!(fdout=fopen(buf,"w"))) {
            fprintf(stdout,"error opening file '%s' \n",buf);
            errorFlag=1;
        } else {
            //B Parameters related to the cosmological background
            fprintf(fdout,FMTR,"z",cmd->z);
            fprintf(fdout,FMTR,"h",cmd->h);
            fprintf(fdout,FMTR,"sigma8",cmd->sigma8);
            fprintf(fdout,FMTR,"Omb",cmd->Omb);
            fprintf(fdout,FMTR,"Omc",cmd->Omc);
            fprintf(fdout,FMTR,"Omnu",cmd->Omnu);
            fprintf(fdout,FMTR,"ns",cmd->ns);
            fprintf(fdout,FMTR,"w",cmd->w);


//            fprintf(fdout,FMTI,"mChebyshev",cmd->mChebyshev);
//            fprintf(fdout,FMTI,"nsmooth",cmd->nsmooth);
//            fprintf(fdout,FMTT,"rsmooth",cmd->rsmooth);
//            fprintf(fdout,FMTR,"theta",cmd->theta);
//            fprintf(fdout,FMTT,"usePeriodic",
//                    cmd->usePeriodic ? "true" : "false");
            //E
            
            //B Parameters to control the I/O file(s)
            // Input catalog parameters
            fprintf(fdout,FMTT,"fnamePS",cmd->fnamePS);
            fprintf(fdout,FMTI,"Wg",cmd->Wg);
            fprintf(fdout,FMTT,"fWgchi",cmd->fWgchi);
            // Output parameters
            fprintf(fdout,FMTT,"rootDir",cmd->rootDir);
            fprintf(fdout,FMTT,"path_Bells",cmd->path_Bells);
            fprintf(fdout,FMTT,"prefix",cmd->prefix);
            fprintf(fdout,FMTI,"tree_level",cmd->tree_level);
            fprintf(fdout,FMTR,"zbin",cmd->zbin);
            fprintf(fdout,FMTI,"mMax",cmd->mMax);
            //E
            
            //B Set of parameters needed to integrate
            fprintf(fdout,FMTI,"chiQuadSteps",cmd->chiQuadSteps);
            fprintf(fdout,FMTI,"GLpoints",cmd->GLpoints);
            fprintf(fdout,FMTI,"Nell",cmd->Nell);
            fprintf(fdout,FMTR,"ellmax",cmd->ellmax);
            fprintf(fdout,FMTR,"ellmin",cmd->ellmin);
            //E

            //B Miscellaneous parameters
            fprintf(fdout,FMTI,"writevectors",cmd->writevectors);
//            fprintf(fdout,FMTTS,"preScript",cmd->preScript);
//            fprintf(fdout,FMTTS,"posScript",cmd->posScript);
            fprintf(fdout,FMTI,"chatty",cmd->chatty);
            fprintf(fdout,FMTI,"verbose",cmd->verbose);
            fprintf(fdout,FMTI,"verbose_log",cmd->verbose_log);
#ifdef OPENMPCODE
            fprintf(fdout,FMTI,"numberThreads",cmd->numthreads);
#endif
            if (cmd->verbose>=VERBOSEDEBUGINFO) {
                verb_print(cmd->verbose, "\n%s: PrintParamterFile: preScript: %s\n",
                           routineName, cmd->preScript);
                verb_print(cmd->verbose, "\n%s: PrintParamterFile: posScript: %s\n",
                           routineName, cmd->posScript);
            }
            fprintf(fdout,FMTT,"options",cmd->options);
            //E
            
            //B socket:
#ifdef ADDONS
#include "startrun_include_08.h"
#endif
            //E
            
            fprintf(fdout,"\n\n");
        }
        fclose(fdout);
        
        if(errorFlag) {
            exit(0);
        }
        
        if (ndefault != 0)
            free(dp);
        
    } // ! gd->flagPrint==TRUE && gd->rootDirFlag==TRUE

    debug_tracking_s("002... final", routineName);


    return SUCCESS;
}
//E

#undef FMTT
#undef FMTTS
#undef FMTI
#undef FMTR
/*
Memory-allocation startup routine:
*/
int startrun_memoryAllocation(struct  cmdline_data *cmd,
                                     struct  global_data* gd)
{
    string routineName = "startrun_memoryAllocation";
    // Free allocated memory in reverse order as were allocated

    INTEGER bytes_tot_local=0;
    //B PXD functions
#ifdef PXD
#endif
    bytes_tot_local += 0.0*sizeof(real);
    //E PXD functions

//B socket:
#ifdef ADDONS
    // this is empty and can be remove these 3 lines
#include "startrun_include_10.h"                    // should be sync with
                                                //  "cballsio_include_10.h"
#endif
//E

    gd->bytes_tot += bytes_tot_local;
    verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                           "\n%s: Allocated %g MByte for histograms storage.\n",
                           routineName, bytes_tot_local*INMB);

//    gd->histograms_allocated = TRUE;

    return SUCCESS;
}

/*
Memory-allocation startup routine:

*/
int SetNumberThreads(struct  cmdline_data *cmd)
{
    omp_set_num_threads(cmd->numthreads);

    return SUCCESS;
}
//#endif


//B Special routines to scan command line

/*
Special-parameter scanning routine:

*/

local int startrun_getParamsSpecial(struct  cmdline_data* cmd,
                                    struct  global_data* gd)
{
    string routineName = "startrun_getParamsSpecial";
    char *pch;
    int nitems, ndummy=1;
    char inputnametmp[MAXLENGTHOFSTRSCMD];
    int i;
    size_t slen;

    debug_tracking_s("001", routineName);


//B socket:
#ifdef ADDONS
#include "startrun_include_12.h"
#endif
//E
    debug_tracking_s("002... final", routineName);

    return SUCCESS;
}
/*
Integer-option scanning routine:
*/
local int scaniOption(struct  cmdline_data* cmd, struct  global_data* gd,
                      string optionstr, int *option, int *noption,
                      int nfiles, int flag, string message)
{
    string routineName = "scaniOption";
    char *pch;
    char *poptionstr[30],  optiontmp[100];
    int i;

    verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                           "\n%s: Processing '%s' option:\n",
                        routineName, message);

    verb_log_print(cmd->verbose_log, gd->outlog,
                           "%s: Splitting string \"%s=%s\" in tokens:\n",
                        routineName, message, optionstr);

    if (!strnull(optionstr)) {
        strcpy(optiontmp,optionstr);
        verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "%s: Splitting string \"%s\" in tokens:\n",
                            routineName, optiontmp);
        *noption=0;
        pch = strtok(optiontmp," ,");
        while (pch != NULL) {
            poptionstr[*noption] = (string) malloc(10);
            strcpy(poptionstr[*noption],pch);
            ++(*noption);
            verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                                  "%s: %s\n",
                                routineName, poptionstr[*noption-1]);
            pch = strtok (NULL, " ,");
        }

        verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "%s: num. of tokens in option %s =%d\n",
                            routineName, optionstr, *noption);

        if (flag == 0)
            if (*noption != nfiles)
                error("\nscanOption: noption = %d %s",
                      *noption,
                      "must be equal to number of infiles\n\n");
        if (*noption > MAXITEMS)
            error("\nscaniOption: noption = %d %s",
                  *noption,
                  "must be less than the maximum num. of lines\n\n");

        for (i=0; i<*noption; i++) {
            option[i]=atoi(poptionstr[i]);
            verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                                   "%s: option: %d\n",
                                routineName, option[i]);
        }
        verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "%s: noptions, nfiles: %d %d\n\n",
                            routineName, *noption,nfiles);
        if (flag == 1) {
            if (*noption > nfiles)
                error("\nscanOption: noption = %d %s",
                      *noption,
                      "must be less or equal to number of files\n\n");
            else {
                for (i=*noption; i<nfiles; i++) {
                    option[i]=option[i-1]+1;
                    verb_print_debug_info(cmd->verbose,
                                          cmd->verbose_log, gd->outlog,
                                          "%s: option: %d\n",
                                          routineName, option[i]);
                }
            }
        }
    }

    return SUCCESS;
}
/*
Real-option scanning routine:
*/
local int scanrOption(struct  cmdline_data* cmd, struct  global_data* gd,
                      string optionstr, double *option, int *noption,
    int nfiles, int flag, string message)
{
    string routineName = "scanrOption";

    char *pch;
    char *poptionstr[30],  optiontmp[100];
    int i;

    verb_log_print(cmd->verbose_log, gd->outlog,
                           "\n%s: Processing '%s' option:\n",
                        routineName, message);

    verb_log_print(cmd->verbose_log, gd->outlog,
                           "%s: Splitting string \"%s\" in tokens:\n",
                        routineName, message);

    if (!strnull(optionstr)) {
        strcpy(optiontmp,optionstr);
        verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                               "\n%s: Splitting string \"%s\" in tokens:\n",
                            routineName, optiontmp);
        *noption=0;
        pch = strtok(optiontmp," ,");
        while (pch != NULL) {
            poptionstr[*noption] = (string) malloc(MAXLENGTHOFREAL);
            strcpy(poptionstr[*noption],pch);
            ++(*noption);
            verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                                  "%s: %s\n",
                                routineName, poptionstr[*noption-1]);
            pch = strtok (NULL, " ,");
        }
        verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                              "%s: num. of tokens in option %s =%d\n",
                            routineName, optionstr, *noption);

        if (flag == 0)
            if (*noption != nfiles)
            error("\nscanOption: noption = %d must be equal to number of files\n\n",
                      *noption);
        if (*noption > MAXITEMS)
    error("\nscanOption: noption = %d must be less than the maximum num. of lines\n\n",
                  *noption);

        for (i=0; i<*noption; i++) {
            option[i]=atof(poptionstr[i]);
            if (cmd->verbose_log>=3)
            verb_log_print(cmd->verbose_log, gd->outlog, "option: %g\n",option[i]);
        }

        verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                              "\n%s: noptions, nfiles: %d %d\n",
                            routineName, *noption, nfiles);
        if (flag == 1) {
            if (*noption > nfiles)
    error("\nscanOption: noption = %d must be less or equal to number of files\n\n",
                  *noption);
            else {
                for (i=*noption; i<nfiles; i++) {
                    option[i]=option[i-1]+1;
                    if (cmd->verbose_log>=3)
                    verb_log_print(cmd->verbose_log, gd->outlog, 
                                   "option: %g\n",option[i]);
                }
            }
        }

        for (i=0; i<*noption; i++) {
            free(poptionstr[*noption]);
        }

    } else {
        for (i=0; i<nfiles; i++) {
            option[i]=0.0;                          // Be aware of this values
            verb_print_debug_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                                   "%s: option: %d\n",
                                routineName, option[i]);
        }
    }

    return SUCCESS;
}

/*
Real-option scanning routine:
*/

local int print_make_info(struct cmdline_data* cmd,
                          struct  global_data* gd)
{
    verb_print(cmd->verbose,
               "\nprint_make_info:\n");

#ifdef OPENMPCODE
    verb_print(cmd->verbose, "using OpenMP\n");
#endif

#ifdef SINGLEP
    verb_print(cmd->verbose, "SINGLEP\n");
#endif

#ifdef LONGINT
    verb_print(cmd->verbose, "LONGINT\n");
#endif

#ifdef DEBUG
    verb_print(cmd->verbose, "DEBUG\n");
#endif

#ifdef USEGSL
#ifdef GSLINTERNAL
    verb_print(cmd->verbose, "using internal GSL\n");
#else
    verb_print(cmd->verbose, "using GSL\n");
#endif
#endif

#ifdef ADDONS
    verb_print(cmd->verbose, "with ADDONS\n");
#endif

#ifdef CLASSLIB
    verb_print(cmd->verbose, "with CLASSLIB\n");
#endif

#ifdef PXD
    verb_print(cmd->verbose, "with PXD\n");
#endif

    return SUCCESS;
}

// I/O directories:

/*
Log-directory setup routine:
*/
local void setFilesDirs_log(struct cmdline_data* cmd,
                             struct  global_data* gd)
{
    string routineName = "setFilesDirs_log";
    char buf[BUFFERSIZE];

    debug_tracking_s("001", routineName);
    if (cmd->verbose_log>0) {           // gd->logfilePath is defined
        debug_tracking_s("002", cmd->rootDir);
        sprintf(gd->tmpDir,"%s/%s",cmd->rootDir,"tmp");
        double cpustart = CPUTIME;
        debug_tracking_s("003", gd->tmpDir);
        sprintf(buf,"if [ ! -d %s ]; then mkdir %s; fi",
                gd->tmpDir,gd->tmpDir);
        system(buf);
        debug_tracking("004");
        gd->cputotalinout += CPUTIME - cpustart;
        sprintf(gd->logfilePath,"%s/wlcf.log",
                gd->tmpDir);
//        gd->tmpDir,cmd->suffixOutFiles);
    }
    debug_tracking("005... final");
}
/*
Output-directory setup routine:
*/
local void setFilesDirs(struct cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "setFilesDirs";
    char buf[BUFFERSIZE];

    char outputDir[MAXLENGTHOFFILES];

    double cpustart = CPUTIME;

    int ndefault = 0;
    int *ipos;
    char *dp1, *dp2;
    int lenDir = strlen(cmd->rootDir);
    int i;

    debug_tracking_s("001", routineName);
    debug_tracking_s("002: init", cmd->rootDir);

    if (gd->rootDirFlag==TRUE) {
        
        int nslashs = MAXNSLASHS;
        ipos = (int*) malloc((nslashs)*sizeof(int));
//        dp1 = (char*) malloc((lenDir)*sizeof(char));
        dp1 = (char*) malloc((MAXLENGTHOFSTRSCMD)*sizeof(char));

        for (i=0; i< lenDir; i++) {
            if(cmd->rootDir[i] == '/') {
                ipos[ndefault] = i+1;
                ndefault++;
            }
        }
        if (ndefault>nslashs)
            error("%s: more '/' than %d in 'rootDir=%s'. Use only %d or none\n",
                  routineName, nslashs, cmd->rootDir, nslashs);
        
        if (ndefault == 0) {
            sprintf(outputDir,cmd->rootDir);
            sprintf(buf,"if [ ! -d %s ]; then mkdir %s; fi",
                    outputDir,outputDir);
            if (cmd->verbose >= 3)
                verb_print_q(3, cmd->verbose_log,"\nsystem: %s\n",buf);
            system(buf);
        } else {
            for (i=0; i<ndefault; i++) {
                debug_tracking("003");
//                strncpy(dp1, cmd->rootDir, ipos[i]-1);
                snprintf(dp1, ipos[i]+1, "%s", cmd->rootDir);
                sprintf(buf,"if [ ! -d %s ]; then mkdir -p %s; fi",dp1,dp1);
                verb_print_q(3,cmd->verbose_log,"\nsystem: %d: %s\n",i,buf);
                system(buf);
                debug_tracking("004");
            }
//            strncpy(dp1, cmd->rootDir, lenDir);
            snprintf(dp1, lenDir+1, "%s", cmd->rootDir);
            sprintf(buf,"if [ ! -d %s ]; then mkdir -p %s; fi",dp1,dp1);
            verb_print_q(3,cmd->verbose_log,"\nsystem: %d: %s\n",i,buf);
            system(buf);
        }
        gd->cputotalinout += CPUTIME - cpustart;
        
        debug_tracking_s("005", cmd->rootDir);
/*
        sprintf(gd->fpfnameOutputFileName,"%s/%s%s%s",
                cmd->rootDir,cmd->outfile,cmd->suffixOutFiles,EXTFILES);
        sprintf(gd->fpfnamehistNNFileName,"%s/%s%s%s",
                cmd->rootDir,cmd->histNNFileName,cmd->suffixOutFiles,EXTFILES);
        sprintf(gd->fpfnamehistCFFileName,"%s/%s%s%s",
                cmd->rootDir,"histCF",cmd->suffixOutFiles,EXTFILES);
        sprintf(gd->fpfnamehistrBinsFileName,"%s/%s%s%s",
                cmd->rootDir,"rbins",cmd->suffixOutFiles,EXTFILES);
        sprintf(gd->fpfnamehistXi2pcfFileName,"%s/%s",
                cmd->rootDir,cmd->histXi2pcfFileName);
        sprintf(gd->fpfnamehistZetaGFileName,"%s/%s%s%s",
                cmd->rootDir,cmd->histZetaFileName,"G",cmd->suffixOutFiles);
        sprintf(gd->fpfnamehistZetaGmFileName,"%s/%s%s%s",
                cmd->rootDir,cmd->histZetaFileName,"G",cmd->suffixOutFiles);
        sprintf(gd->fpfnamehistZetaMFileName,"%s/%s%s%s",
                cmd->rootDir,cmd->histZetaFileName,"M",cmd->suffixOutFiles);
        sprintf(gd->fpfnamemhistZetaMFileName,"%s/%s%s%s%s",
                cmd->rootDir,"m",cmd->histZetaFileName,"M",cmd->suffixOutFiles);
        sprintf(gd->fpfnameCPUFileName,"%s/cputime%s%s",
                cmd->rootDir,cmd->suffixOutFiles,EXTFILES);
*/
        free(ipos);
        

        //B socket:
#ifdef ADDONS
#include "wlcfio_include_09b.h"
#endif
        //E
        free(dp1);
    } // ! rootDirFlag
    debug_tracking_s("006: final", cmd->rootDir);
}
/*
Output initialization routine:
*/
int StartOutput(struct cmdline_data *cmd, struct  global_data* gd)
{
    //B clear some char arrays
//    gd->logfilePath[0] = '\0';
//    gd->fpfnameOutputFileName[0] = '\0';
//    gd->fnameData_kd[0] = '\0';
//    gd->fnameOut_kd[0] = '\0';
    //E

//    outfilefmt_string_to_int(cmd->outfilefmt, &outfilefmt_int);

    if (cmd->verbose>=VERBOSEMININFO)
        if (! strnull(cmd->options))
            verb_print(cmd->verbose, "\n\toptions: %s\n", cmd->options);

    return SUCCESS;
}
