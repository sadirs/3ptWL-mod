/*==============================================================================
 MODULE: startrun.c				[wlcf]
 Written by: Mario A. Rodriguez-Meza
 Starting date: 01.05.2026
 Purpose: routines to initialize the main code
 Language: C
 Use: 'StartRun();'
 Mayor revisions:
 ==============================================================================*/
//        1          2          3          4        ^ 5          6          7

//
// We must check the order of memory allocation and deallocation!!!
// Here and in EndRun in cballsio.c
//

//
// lines where there is a "//B socket:" string are places to include module files
//  that can be found in addons/addons_include folder
//

#include "globaldefs.h"
#include <limits.h>
#include <errno.h>

#ifdef CLASSLIB
#define WLCOV_FAIL(cmd, ...)                                      \
    do {                                                          \
        snprintf((cmd)->error_message, _ERRORMSGSIZE_, __VA_ARGS__); \
        return FAILURE;                                           \
    } while (0)
#else
#define WLCOV_FAIL(cmd, ...) error(__VA_ARGS__)
#endif

local int ReadParameterFile(struct  cmdline_data*, struct  global_data*,
                             char *);
local int startrun_parameterfile(struct  cmdline_data*, struct  global_data*);
local int startrun_cmdline(struct  cmdline_data*, struct  global_data*);
local void ReadParametersCmdline(struct  cmdline_data*, struct  global_data*);
local void ReadParametersCmdline_short(struct  cmdline_data*, 
                                       struct  global_data*);
local int CheckParameters(struct  cmdline_data*, struct  global_data*);

//B I/O directories:
local int setFilesDirs_log(struct cmdline_data*, struct global_data* gd);
local int setFilesDirs(struct cmdline_data*, struct global_data* gd);
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

 To be called in main:
 StartRun(&cmd, &gd, argv[0], HEAD1, HEAD2, HEAD3);
 
 This routine is in charge of setting all global structures in order to
    the comutation process run smoothly with all parameters given
    by the user, set and checked.

 Arguments:
    * `cmd`: Input: structure cmdline_data pointer
    * `gd`: Input: structure global_data pointer
    * `head0`: Input: string
    * `head1`: Input: string
    * `head2`: Input: string
    * `head3`: Input: string
 Return (the error status):
    int SUCCESS or FAILURE
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

    gd->startrun_cputime = TRUE;
    gd->cmd_allocated = FALSE;
    gd->bytes_tot = 0;

    cmd->paramfile = GetParam("paramfile");
    if (*(cmd->paramfile) == '-') {
        WLCOV_FAIL(cmd, "bad parameter %s\n", cmd->paramfile);
    }
    
    if (!strnull(cmd->paramfile)) {
        if (startrun_parameterfile(cmd, gd) == FAILURE)
            return FAILURE;
    } else {
        if (startrun_cmdline(cmd, gd) == FAILURE)
            return FAILURE;
    }

    gd->bytes_tot += sizeof(struct  global_data);
    gd->bytes_tot += sizeof(struct cmdline_data);
    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                "\n%s: Total allocated %g MByte storage so far.\n",
                        routineName, gd->bytes_tot*INMB);

    class_call_cballs(SetNumberThreads(cmd), errmsg, errmsg);
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

    gd->startrun_cputime = TRUE;
    gd->cmd_allocated = FALSE;
    gd->bytes_tot = 0;

    cmd->paramfile = GetParam("paramfile");
    if (*(cmd->paramfile) == '-') {
        WLCOV_FAIL(cmd, "bad parameter %s\n", cmd->paramfile);
    }

    if (!strnull(cmd->paramfile)) {
        class_call_cballs(input_find_file(cmd, gd, cmd->paramfile, &fc, errmsg),
                          errmsg, errmsg);

        if (input_read_from_file(cmd, gd, &fc, errmsg) == FAILURE) {
            parser_free(&fc);
            class_call_cballs(FAILURE, errmsg, errmsg);
        }

        class_call_cballs(parser_free(&fc), errmsg, errmsg);
        
        if (StartRun_Common(cmd, gd) == FAILURE)
            return FAILURE;

        if (PrintParameterFile(cmd, gd, cmd->paramfile) == FAILURE)
            return FAILURE;
        
        
    } else {
        if (startrun_cmdline(cmd, gd) == FAILURE)
            return FAILURE;
    }

    gd->bytes_tot += sizeof(struct  global_data);
    gd->bytes_tot += sizeof(struct cmdline_data);
    verb_print(cmd->verbose,
               "\n%s: Total allocated %g MByte storage so far.\n",
               routineName, gd->bytes_tot*INMB);

    class_call_cballs(SetNumberThreads(cmd), errmsg, errmsg);

    verb_print_min_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                        "\n%s CPU time: %g %s\n",
                        routineName, CPUTIME - cpustart, PRNUNITOFTIMEUSED);

    return SUCCESS;
}
#endif // ! CLASSLIB

local int startrun_parameterfile(struct cmdline_data* cmd,
                                 struct global_data* gd)
{
    if (ReadParameterFile(cmd, gd, cmd->paramfile) == FAILURE) {
        if (cmd->error_message[0] == '\0') {
            snprintf(cmd->error_message, _ERRORMSGSIZE_,
                     "ReadParameterFile: failed parsing parameter file '%s'\n",
                     cmd->paramfile);
        }
        return FAILURE;
    }

    ReadParametersCmdline_short(cmd, gd);

#ifdef ADDONS
#include "startrun_include_01.h"
#endif

    if (StartRun_Common(cmd, gd) == FAILURE)
        return FAILURE;

    if (PrintParameterFile(cmd, gd, cmd->paramfile) == FAILURE)
        return FAILURE;

    return SUCCESS;
}


#define parameter_null	"parameters_null-wlcf"

//B reading parameters from the command line
local int startrun_cmdline(struct cmdline_data* cmd, struct global_data* gd)
{
    ReadParametersCmdline(cmd, gd);

    if (StartRun_Common(cmd, gd) == FAILURE)
        return FAILURE;

    if (PrintParameterFile(cmd, gd, parameter_null) == FAILURE)
        return FAILURE;

    return SUCCESS;
}

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
    cmd->writevectors = GetbParam("writevectors");
    cmd->verbose = GetiParam("verbose");
    cmd->verbose_log = GetiParam("verbose_log");
#ifdef OPENMPCODE
    cmd->numthreads = GetiParam("numberThreads");
#else
    cmd->numthreads = GetiParam("numberThreads");
    cmd->numthreads = 1;
#endif
    cmd->options = GetParam("options");
    //E

//B socket:
#ifdef ADDONS
#include "startrun_include_02.h"
#endif
//E
}

local void ReadParametersCmdline_short(struct  cmdline_data* cmd, struct  global_data* gd)
{
//B Here add parameters needed to be read after reading parameter file
//    cmd->ellmax = GetdParam("ellmax");
//E
}

//E

#undef parameter_null

//B Section of parameter reading from a file
local int ReadParameterFile(struct  cmdline_data* cmd,
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

    size_t str_size[MAXTAGS];

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
    BPName(cmd->writevectors,"writevectors");
    IPName(cmd->verbose,"verbose");
    IPName(cmd->verbose_log,"verbose_log");
#ifdef OPENMPCODE
    IPName(cmd->numthreads,"numberThreads");
#else
    IPName(cmd->numthreads,"numberThreads");
    cmd->numthreads = 1;
#endif
    SPName(cmd->options,"options",MAXLENGTHOFSTRSCMD);
    //E

//B socket:
#ifdef ADDONS
#include "startrun_include_03.h"
#endif
//E

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
        while (fgets(line, MAXCHARBUF, fd) != NULL) {
//B
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

            while (right >= left && right[0] == ' ') {
              right--;
            }

            if (right >= left && (right[0] == '\'' || right[0] == '\"')) {
              right--;
            }
            

            if (right-left < 0) {
                fprintf(stdout,
        "Error in file %s: there is no variable name before '=' in line: '%s'\n",
                    fname, line);
                errorFlag=1;
                continue;
            }

            if ((size_t)(right - left + 1) >= sizeof(name)) {
                if (fd != NULL) fclose(fd);
                WLCOV_FAIL(cmd, "%s: parameter name too long\n", routineName);
            }

            memcpy(name, left, (size_t)(right - left + 1));
            name[right - left + 1] = '\0';


            left = pequal+1;
            while (left[0]==' ') {
              left++;
            }

            if (phash == NULL)
              right = line+strlen(line)-1;
            else
              right = phash-1;

            while (right >= left && right[0] <= ' ') {
                right--;
            }

            if (right < left)
                continue;

            if ((size_t)(right - left + 1) >= sizeof(value)) {
                if (fd != NULL) fclose(fd);
                WLCOV_FAIL(cmd, "%s: parameter value too long\n", routineName);
            }
            
            memcpy(value, left, (size_t)(right - left + 1));
            value[right - left + 1] = '\0';
//E

            for(i=0,j=-1;i<nt;i++)
                if(strcmp(name,tag[i])==0) {
                    j=i;
                    tag[i][0]=0;
                    break;
                }
            if(j>=0) {
                switch(id[j]) {
                    case DOUBLE: {
                        char *end = NULL;
                        double x;

                        errno = 0;
                        x = strtod(value, &end);

                        if (errno != 0 || end == value || *end != '\0' || !isfinite(x)) {
                            fclose(fd);
                            snprintf(cmd->error_message, _ERRORMSGSIZE_,
                                     "%s: %s=%s not a valid finite double\n",
                                     routineName, name, value);
                            return FAILURE;
                            
                        }

                        *((double*)addr[j]) = x;
                        break;
                    }
                        
                    case STRING:
                        if (copy_checked((char *)addr[j], str_size[j], value, name) != 0) {
                            if (fd != NULL) fclose(fd);
                            WLCOV_FAIL(cmd, "%s: string parameter '%s' too long\n",
                                       routineName, name);
                        }
                        break;

                    case INT: {
                        char *end = NULL;
                        long x;

                        errno = 0;
                        x = strtol(value, &end, 10);

                        if (errno != 0 || end == value || *end != '\0' ||
                            x < INT_MIN || x > INT_MAX) {
                            fclose(fd);
                            snprintf(cmd->error_message, _ERRORMSGSIZE_,
                                     "%s: %s=%s not a valid finite int\n",
                                     routineName, name, value);
                            return FAILURE;

                        }

                        *((int*)addr[j]) = (int)x;
                        break;
                    }
                        
                    case LONG: {
                        char *end = NULL;
                        long x;

                        errno = 0;
                        x = strtol(value, &end, 10);

                        if (errno != 0 || end == value || *end != '\0') {
                            fclose(fd);
                            snprintf(cmd->error_message, _ERRORMSGSIZE_,
                                     "%s: %s=%s not a valid finite long\n",
                                     routineName, name, value);
                            return FAILURE;

                        }

                        *((long*)addr[j]) = x;
                        break;
                    }
                        
                    case BOOLEAN:
                        if (strchr("tTyY1", *value) != NULL) {
                            *((bool*)addr[j])=TRUE;
                        } else
                            if (strchr("fFnN0", *value) != NULL)  {
                                *((bool*)addr[j])=FALSE;
                            } else {
                                if (fd != NULL) fclose(fd);
                                WLCOV_FAIL(cmd, "%s: %s=%s not bool\n", routineName, name, value);
                            }
                        break;
                }
            } else {
                fprintf(stdout, "\n%s: Error in file %s: Tag '%s' %s\n",
                        routineName, fname, name,
                        "not allowed or multiple defined...");
                errorFlag=1;
            }
        } // ! while loop
        fclose(fd);
    } else {
        fprintf(stdout,"Parameter file %s not found.\n", fname);
        errorFlag=2;
        WLCOV_FAIL(cmd, "Parameter file %s not found.\n", fname);
    }

    if (errorFlag == 1)
        WLCOV_FAIL(cmd, "%s: parameter file '%s' contains unknown or duplicated tags\n",
                   routineName, fname);
    
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
                    if (copy_checked((char *)addr[i], str_size[i], GetParam(tag[i]), tag[i]) != 0)
                        WLCOV_FAIL(cmd, "%s: default string parameter '%s' too long\n",
                                   routineName, tag[i]);
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
    
    return SUCCESS;
}
//E

int StartRun_Common(struct  cmdline_data* cmd, struct  global_data* gd)
{
    string routineName = "StartRun_Common";

#ifndef USEGSL
#error `USEGSL` is not defined. Switch it on in Makefile_settings
#endif

    //B
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (gd->cmd_allocated != TRUE) {
        gd->optionsFlag = FALSE;
        gd->rootDirFlagFree = FALSE;
        gd->prefixFlag = FALSE;
        gd->fWgchiFlag = FALSE;
        gd->fnamePSFlag = FALSE;
    }
    
    if (cmd->rootDir == NULL || strlen(cmd->rootDir) == 0) {
        if (cmd->rootDir != NULL && gd->rootDirFlagFree == TRUE) {
            free(cmd->rootDir);
            gd->rootDirFlagFree = FALSE;
        }

        cmd->rootDir = ".";
    }

    if (cmd->options == NULL) {
        cmd->options = "";
        gd->optionsFlag = FALSE;
    }
    //E
    
    if (gd->cmd_allocated != TRUE)
        gd->cmd_allocated = TRUE;

    gd->gd_allocated = FALSE;
    gd->tables_allocated = FALSE;
    gd->outlog = NULL;
    
    if (strnull(cmd->rootDir))
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

    class_call_cballs(StartOutput(cmd, gd), errmsg, errmsg);
    class_call_cballs(setFilesDirs(cmd, gd), errmsg, errmsg);
    class_call_cballs(setFilesDirs_log(cmd, gd), errmsg, errmsg);

    gd->mode[0] = 'w';
    gd->mode[1] = '\0';
    //B gd->outlog is defined
    if (cmd->verbose_log>0 && gd->rootDirFlag == TRUE) {
        if (!(gd->outlog = fopen(gd->logfilePath, gd->mode))) {
            WLCOV_FAIL(cmd, "\n%s: error opening file '%s'\n",
                       routineName, gd->logfilePath);
        }
    }
    //E

    cmd->Omm=cmd->Omb+cmd->Omc+cmd->Omnu;
    cmd->Omw=1.-cmd->Omm;

    class_call_cballs(CheckParameters(cmd, gd), errmsg, errmsg);
    class_call_cballs(startrun_memoryAllocation(cmd, gd), errmsg, errmsg);

    verb_print_normal_info(cmd->verbose, cmd->verbose_log, gd->outlog,
                          "%s: OmegaM = : %f\n",
                           routineName, cmd->Omm);

//B socket:
#ifdef ADDONS
#include "startrun_include_05.h"
#endif
//E
    gd->gd_allocated = TRUE;

    return SUCCESS;
}


//B Section of parameter check
local int CheckParameters(struct  cmdline_data* cmd, struct  global_data* gd)
{
// If it is necessary: an item in cmdline_defs.h must have an item here::
    string routineName = "CheckParameters";

    
//
//  cleanup note: the old range-only checks are still
//  above the new finite checks,
//  so CheckParameters() now has some duplicate validation.
//  It compiles and works,
//  but later may want to merge those blocks into one cleaner validation section.
//

    if (cmd->z < 0.0)
        WLCOV_FAIL(cmd, "\n%s: z (%g) can not be less than 0.\n",
                   routineName, cmd->z);

    if (cmd->h <= 0.0)
        WLCOV_FAIL(cmd, "\n%s: Hubble parameter (%g) can not be less than 0.\n",
                   routineName, cmd->h);

    if (cmd->sigma8 <= 0.0 || !isfinite(cmd->sigma8))
        WLCOV_FAIL(cmd, "\n%s: sigma8 (%g) must be positive and finite.\n",
                   routineName, cmd->sigma8);

    if (cmd->Omb < 0.0)
        WLCOV_FAIL(cmd,
                   "\n%s: Omega baryon parameter (%g) can not be less than 0.\n",
                   routineName, cmd->Omb);

    if (cmd->Omc < 0.0)
        WLCOV_FAIL(cmd,
                   "\n%s: Omega CDM parameter (%g) can not be less than 0.\n",
                   routineName, cmd->Omc);

    if (cmd->Omnu < 0.0)
        WLCOV_FAIL(cmd,
                   "\n%s: Omega nu parameter (%g) can not be less than 0.\n",
                   routineName, cmd->Omnu);

    if (!isfinite(cmd->Omm))
        WLCOV_FAIL(cmd,
                   "\n%s: Omega matter parameter Omm (%g) must be finite.\n",
                   routineName, cmd->Omm);

    if (cmd->Omm <= 0.0 || cmd->Omm > 1.0)
        WLCOV_FAIL(cmd,
        "\n%s: Omega matter parameter Omm (%g = Omb+Omc+Omnu) must be in (0, 1].\n",
        routineName, cmd->Omm);

    if (!isfinite(cmd->Omw))
        WLCOV_FAIL(cmd,
                "\n%s: Omega dark energy parameter Omw (%g) must be finite.\n",
                routineName, cmd->Omw);

    if (cmd->Omw < 0.0)
        WLCOV_FAIL(cmd,
        "\n%s: Omega dark energy parameter Omw (%g = 1-Omm) must be non-negative.\n",
        routineName, cmd->Omw);

    if (!isfinite(cmd->w))
        WLCOV_FAIL(cmd,
                   "\n%s: dark energy equation-of-state w (%g) must be finite.\n",
                   routineName, cmd->w);

    if (cmd->ns < 0.0)
        WLCOV_FAIL(cmd,
            "\n%s: Spectral index parameter ns (%g) can not be less than 0.\n",
            routineName, cmd->ns);

    if (cmd->Nell < 8)
        WLCOV_FAIL(cmd,
        "\n%s: Nell (%d) must be at least 8 for FFTLog c_window_width=0.25.\n",
        routineName, cmd->Nell);

    if (cmd->Nell % 2 != 0)
        WLCOV_FAIL(cmd, "\n%s: Nell (%d) must be even.\n",
                   routineName, cmd->Nell);

    if (cmd->ellmin <= 0.0)
        WLCOV_FAIL(cmd,
                   "\n%s: ellmin (%g) must be positive.\n",
                   routineName, cmd->ellmin);

    if (cmd->ellmax <= cmd->ellmin)
        WLCOV_FAIL(cmd,
                   "\n%s: ellmax (%g) must be greater than ellmin (%g).\n",
                   routineName, cmd->ellmax, cmd->ellmin);
    if (cmd->numthreads < 1)
        WLCOV_FAIL(cmd,
                   "\n%s: numberThreads (%d) must be at least 1.\n",
                   routineName, cmd->numthreads);

    if (cmd->chiQuadSteps < 2)
        WLCOV_FAIL(cmd,
                   "\n%s: chiQuadSteps (%d) must be at least 2.\n",
                   routineName, cmd->chiQuadSteps);

    if (cmd->GLpoints < 2)
        WLCOV_FAIL(cmd,
                   "\n%s: GLpoints (%d) must be at least 2.\n",
                   routineName, cmd->GLpoints);
    
    if (cmd->mMax < 0)
        WLCOV_FAIL(cmd,
                   "\n%s: mMax (%d) must be greater than or equal to 0.\n",
                   routineName, cmd->mMax);

    if (cmd->Wg != 0 && cmd->Wg != 1)
        WLCOV_FAIL(cmd,
                   "\n%s: Wg (%d) must be 0 or 1.\n",
                   routineName, cmd->Wg);

    if (cmd->zbin <= 0.)
        WLCOV_FAIL(cmd,
                   "\n%s: zbin (%g) must be > 0.\n",
                   routineName, cmd->zbin);
    
    if (cmd->mMax == INT_MAX)
        WLCOV_FAIL(cmd,
                   "\n%s: mMax (%d) is too large.\n",
                   routineName, cmd->mMax);

    size_t Nell = (size_t)cmd->Nell;
    size_t NumMoments = (size_t)cmd->mMax + 1;
    size_t size_max = (size_t)-1;

    if (Nell > (size_t)INT_MAX / Nell)
        WLCOV_FAIL(cmd,
                   "\n%s: Nell (%d) is too large: Nell*Nell overflows int loops.\n",
                   routineName, cmd->Nell);

    size_t Nell2 = Nell * Nell;

    if (Nell2 > size_max / sizeof(double))
        WLCOV_FAIL(cmd,
                   "\n%s: Nell (%d) is too large for one Nell*Nell double array.\n",
                   routineName, cmd->Nell);

    if (NumMoments > size_max / sizeof(double *))
        WLCOV_FAIL(cmd,
                   "\n%s: mMax (%d) is too large for moment pointer arrays.\n",
                   routineName, cmd->mMax);

    if (NumMoments > size_max / Nell2 / sizeof(double))
        WLCOV_FAIL(cmd,
                   "\n%s: Nell (%d) and mMax (%d) request too much Bm storage.\n",
                   routineName, cmd->Nell, cmd->mMax);

    //B
    if (!isfinite(cmd->z) || cmd->z < 0.0)
        WLCOV_FAIL(cmd, "\n%s: z (%g) must be finite and >= 0.\n",
                   routineName, cmd->z);

    if (!isfinite(cmd->h) || cmd->h <= 0.0)
        WLCOV_FAIL(cmd, "\n%s: Hubble parameter h (%g) must be finite and > 0.\n",
                   routineName, cmd->h);

    if (!isfinite(cmd->Omb) || cmd->Omb < 0.0)
        WLCOV_FAIL(cmd,
                   "\n%s: Omega baryon parameter (%g) must be finite and >= 0.\n",
                   routineName, cmd->Omb);

    if (!isfinite(cmd->Omc) || cmd->Omc < 0.0)
        WLCOV_FAIL(cmd,
                   "\n%s: Omega CDM parameter (%g) must be finite and >= 0.\n",
                   routineName, cmd->Omc);

    if (!isfinite(cmd->Omnu) || cmd->Omnu < 0.0)
        WLCOV_FAIL(cmd,
                   "\n%s: Omega nu parameter (%g) must be finite and >= 0.\n",
                   routineName, cmd->Omnu);

    if (!isfinite(cmd->ns) || cmd->ns < 0.0)
        WLCOV_FAIL(cmd,
                   "\n%s: Spectral index parameter ns (%g) must be finite and >= 0.\n",
                   routineName, cmd->ns);

    if (!isfinite(cmd->ellmin) || cmd->ellmin <= 0.0)
        WLCOV_FAIL(cmd,
                   "\n%s: ellmin (%g) must be finite and positive.\n",
                   routineName, cmd->ellmin);

    if (!isfinite(cmd->ellmax) || cmd->ellmax <= cmd->ellmin)
        WLCOV_FAIL(cmd,
                   "\n%s: ellmax (%g) must be finite and greater than ellmin (%g).\n",
                   routineName, cmd->ellmax, cmd->ellmin);

    if (!isfinite(cmd->zbin) || cmd->zbin <= 0.0)
        WLCOV_FAIL(cmd,
                   "\n%s: zbin (%g) must be finite and > 0.\n",
                   routineName, cmd->zbin);
    //E
    
//B socket:
#ifdef ADDONS
#include "startrun_include_07.h"
#endif
//E

    return SUCCESS;
}
//E


#define FMTT    "%-35s = %s\n"
#define FMTTS    "%-35s = \"%s\"\n"
#define FMTI    "%-35s = %d\n"
#define FMTIL    "%-35s = %ld\n"
#define FMTR	"%-35s = %g\n"

//B Section of parameter writing to a file
int PrintParameterFile(struct  cmdline_data *cmd,
                       struct  global_data* gd, char *fname)
{
// Every item in cmdline_defs.h must have an item here::
    string routineName = "PrintParameterFile";

    int nwrite;

    FILE *fdout = NULL;
    char buf[BUFFERSIZE];
    char *dp = NULL;
    int errorFlag = 0;
    
    if (cmd == NULL || gd == NULL || fname == NULL)
        return FAILURE;

    if (gd->flagPrint==TRUE && gd->rootDirFlag==TRUE) {
        //B Look for "/" if fname is composed: path and filename
        int ndefault = 0;
        int ipos;
        for (int i=0; i< strlen(fname); i++) {
            if(fname[i] == '/') {
                ipos = i+1;
                ndefault++;
            }
        }
        
        if (ndefault == 0) {
            nwrite = snprintf(buf, sizeof(buf), "%s/%s%s",
                              cmd->rootDir, fname, "-usedvalues");
            if (nwrite < 0 || (size_t)nwrite >= sizeof(buf))
                WLCOV_FAIL(cmd, "\n%s: output path too long\n",routineName);
        } else {
            size_t dplen = strlen(fname) - ipos;
            dp = (char*) malloc((dplen + 1) * sizeof(char));
            if (dp == NULL)
                WLCOV_FAIL(cmd, "\n%s: not enough memory\n",routineName);
            nwrite = snprintf(dp, dplen + 1, "%s", fname + ipos);
            if (nwrite < 0 || (size_t)nwrite >= dplen + 1) {
                free(dp);
                WLCOV_FAIL(cmd, "\n%s: filename too long\n",routineName);
            }

            verb_print_q(3,cmd->verbose,
                         "PrintParameterFile: '/' counts %d pos %d and %s\n",
                         ndefault, ipos, dp);
            nwrite = snprintf(buf, sizeof(buf), "%s/%s%s",
                              cmd->rootDir, dp, "-usedvalues");
            if (nwrite < 0 || (size_t)nwrite >= sizeof(buf)) {
                free(dp);
                WLCOV_FAIL(cmd, "\n%s: output path too long\n",routineName);
            }

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
            //E
            
            //B Parameters to control the I/O file(s)
            // Input catalog parameters
            fprintf(fdout,FMTT,"fnamePS",cmd->fnamePS);
            fprintf(fdout,FMTI,"Wg",cmd->Wg);
            fprintf(fdout,FMTT,"fWgchi",cmd->fWgchi);
            // Output parameters
            fprintf(fdout,FMTT,"rootDir",cmd->rootDir);
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
            fprintf(fdout,FMTT,"writevectors",cmd->writevectors ? "true" : "false");
            fprintf(fdout,FMTI,"verbose",cmd->verbose);
            fprintf(fdout,FMTI,"verbose_log",cmd->verbose_log);
            fprintf(fdout,FMTI,"numberThreads",cmd->numthreads);
            fprintf(fdout,FMTT,"options",cmd->options);
            //E
            
//B socket:
#ifdef ADDONS
#include "startrun_include_08.h"
#endif
//E
            fprintf(fdout, "\n\n");

            if (ferror(fdout)) {
                errorFlag = 1;
                goto cleanup;
            }

            if (fclose(fdout) != 0) {
                fdout = NULL;
                errorFlag = 1;
                goto cleanup;
            }
            fdout = NULL;
            
        }
        
        cleanup:
            if (fdout != NULL) {
                fclose(fdout);
                fdout = NULL;
            }

            if (dp != NULL) {
                free(dp);
                dp = NULL;
            }

            if (errorFlag) {
                WLCOV_FAIL(cmd, "\n%s: error writing parameter file '%s'\n",
                           routineName, buf);
            }
        
        
    } // ! gd->flagPrint==TRUE && gd->rootDirFlag==TRUE

    return SUCCESS;
}
//E

#undef FMTT
#undef FMTTS
#undef FMTI
#undef FMTR

// Free allocated memory in reverse order as were allocated
int startrun_memoryAllocation(struct  cmdline_data *cmd,
                                     struct  global_data* gd)
{
    string routineName = "startrun_memoryAllocation";
    int status = SUCCESS;
    int bm_rows = 0;
    int zv_rows = 0;
    INTEGER bytes_tot_local=0;

    //B
    if (cmd == NULL || gd == NULL)
        return FAILURE;

    if (!isfinite(cmd->zbin) || cmd->zbin <= 0.0)
        WLCOV_FAIL(cmd, "\n%s: zbin (%g) must be finite and > 0.\n",
                   routineName, cmd->zbin);

    if (cmd->chiQuadSteps < 2)
        WLCOV_FAIL(cmd, "\n%s: chiQuadSteps (%d) must be at least 2.\n",
                   routineName, cmd->chiQuadSteps);

    if (cmd->Nell < 8)
        WLCOV_FAIL(cmd, "\n%s: Nell (%d) must be at least 8.\n",
                   routineName, cmd->Nell);

    if (cmd->Nell % 2 != 0)
        WLCOV_FAIL(cmd, "\n%s: Nell (%d) must be even.\n",
                   routineName, cmd->Nell);

    if (!isfinite(cmd->ellmin) || cmd->ellmin <= 0.0)
        WLCOV_FAIL(cmd, "\n%s: ellmin (%g) must be finite and positive.\n",
                   routineName, cmd->ellmin);

    if (!isfinite(cmd->ellmax) || cmd->ellmax <= cmd->ellmin)
        WLCOV_FAIL(cmd, "\n%s: ellmax (%g) must be finite and greater than ellmin (%g).\n",
                   routineName, cmd->ellmax, cmd->ellmin);

    if (cmd->mMax < 0 || cmd->mMax == INT_MAX)
        WLCOV_FAIL(cmd, "\n%s: mMax (%d) is invalid.\n",
                   routineName, cmd->mMax);

    size_t Nell = (size_t)cmd->Nell;
    size_t Nell2 = Nell * Nell;
    size_t NumMoments = (size_t)cmd->mMax + 1;
    size_t size_max = (size_t)-1;

    if (Nell > (size_t)INT_MAX / Nell)
        WLCOV_FAIL(cmd, "\n%s: Nell (%d) is too large for int loops.\n",
                   routineName, cmd->Nell);

    if (Nell2 > size_max / sizeof(double))
        WLCOV_FAIL(cmd, "\n%s: Nell (%d) is too large for Nell*Nell arrays.\n",
                   routineName, cmd->Nell);

    if (NumMoments > size_max / sizeof(double *))
        WLCOV_FAIL(cmd, "\n%s: mMax (%d) is too large for moment pointers.\n",
                   routineName, cmd->mMax);

    if (NumMoments > size_max / Nell2 / sizeof(double))
        WLCOV_FAIL(cmd, "\n%s: Nell (%d) and mMax (%d) request too much Bm storage.\n",
                   routineName, cmd->Nell, cmd->mMax);
    
    int NumMomentsInt = cmd->mMax + 1;
    //E
    
    gv.chiforgLT = NULL;
    gv.gLT = NULL;
    gv.zT = NULL;
    gv.chiOfzT = NULL;
    gv.DpT = NULL;

    iv.zT_chiint = NULL;
    /* ... all iv pointers ... */
    iv.BmVectors = NULL;
    iv.BmVectorsp = NULL;

    zv.r1 = NULL;
    zv.r2 = NULL;
    zv.result = NULL;
    
    //B gv tables
    //B inside compute_gL
    gv.NstepsforgL=1000;
    int Nsteps=gv.NstepsforgL;
    gv.chiforgLT = calloc((size_t)Nsteps, sizeof(*gv.chiforgLT));
    if (gv.chiforgLT == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating gv.chiforgLT\n",
                        routineName);
    }
    
    gv.gLT = calloc((size_t)Nsteps, sizeof(*gv.gLT));
    if (gv.gLT == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating gv.gLT\n",
                        routineName);
    }
    //E

    //B inside background
    gv.zMax = 3.0*cmd->zbin;
    gv.zMin = 0.0;
    gv.Nz   = 200;
    gv.zT = calloc((size_t)gv.Nz, sizeof(*gv.zT));
    if (gv.zT == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating gv.zT\n",
                        routineName);
    }
    gv.chiOfzT = calloc((size_t)gv.Nz, sizeof(*gv.chiOfzT));
    if (gv.chiOfzT == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating gv.chiOfzT\n",
                        routineName);
    }
    gv.DpT = calloc((size_t)gv.Nz, sizeof(*gv.DpT));
    if (gv.DpT == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating gv.DpT\n",
                        routineName);
    }
    
    //E
    //E

    //B iv tables
    //B inside ArraysforChiQuad
    iv.chiQuadSteps=cmd->chiQuadSteps;
    iv.zT_chiint = calloc((size_t)iv.chiQuadSteps, sizeof(*iv.zT_chiint));
    if (iv.zT_chiint == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating iv.zT_chiint\n",
                        routineName);
    }
    iv.chiT_chiint = calloc((size_t)iv.chiQuadSteps, sizeof(*iv.chiT_chiint));
    if (iv.chiT_chiint == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating iv.chiT_chiint\n",
                        routineName);
    }
    iv.DpT_chiint = calloc((size_t)iv.chiQuadSteps, sizeof(*iv.DpT_chiint));
    if (iv.DpT_chiint == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating iv.DpT_chiint\n",
                        routineName);
    }
    iv.rsigma_chiint = calloc((size_t)iv.chiQuadSteps, sizeof(*iv.rsigma_chiint));
    if (iv.rsigma_chiint == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating iv.rsigma_chiint\n",
                        routineName);
    }
    iv.neff_chiint = calloc((size_t)iv.chiQuadSteps, sizeof(*iv.neff_chiint));
    if (iv.neff_chiint == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating iv.neff_chiint\n",
                        routineName);
    }
    iv.q_chiint = calloc((size_t)iv.chiQuadSteps, sizeof(*iv.q_chiint));
    if (iv.q_chiint == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating iv.q_chiint\n",
                        routineName);
    }
    //E

    //B allocate_iv
    iv.Nell   = cmd->Nell;
    iv.ellmax = cmd->ellmax;
    iv.ellmin = cmd->ellmin;
    iv.ellT = calloc((size_t)iv.Nell, sizeof(*iv.ellT));
    if (iv.ellT == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating iv.ellT\n",
                        routineName);
    }

    for(int i=0; i<iv.Nell ; i++){
        iv.ellT[i]= exp(log(iv.ellmin)
        + i*log(iv.ellmax/iv.ellmin)/(iv.Nell-1.0));
    }

    //B
    iv.BmVectors = calloc(NumMoments, sizeof(*iv.BmVectors));
    iv.BmVectorsp = calloc(NumMoments, sizeof(*iv.BmVectorsp));
    if (iv.BmVectors == NULL || iv.BmVectorsp == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating Bm vector row pointers\n",
                        routineName);
    }

    for (int m = 0; m < NumMomentsInt; m++) {
        iv.BmVectors[m] = calloc((size_t)iv.Nell * (size_t)iv.Nell,
                                 sizeof(**iv.BmVectors));
        iv.BmVectorsp[m] = calloc((size_t)iv.Nell * (size_t)iv.Nell,
                                  sizeof(**iv.BmVectorsp));
        if (iv.BmVectors[m] == NULL || iv.BmVectorsp[m] == NULL) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup,
                            "%s: not enough memory allocating Bm vector row %d\n",
                            routineName, m);
        }
        bm_rows++;
    }
    //E
    
    //B zv tables
    
    zv.r1 = calloc((size_t)iv.Nell, sizeof(*zv.r1));
    if (zv.r1 == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating zv.r1\n",
                        routineName);
    }

    zv.r2 = calloc((size_t)iv.Nell, sizeof(*zv.r2));
    if (zv.r2 == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating zv.r2\n",
                        routineName);
    }

    zv.result = calloc((size_t)iv.Nell, sizeof(*zv.result));
    if (zv.result == NULL) {
        status = FAILURE;
        COSMO_FAIL_GOTO(cmd, cleanup,
                        "%s: not enough memory allocating zv.result row pointers\n",
                        routineName);
    }

    for (int i = 0; i < iv.Nell; i++) {
        zv.result[i] = calloc((size_t)iv.Nell, sizeof(**zv.result));
        if (zv.result[i] == NULL) {
            status = FAILURE;
            COSMO_FAIL_GOTO(cmd, cleanup,
                            "%s: not enough memory allocating zv.result row %d\n",
                            routineName, i);
        }
        zv_rows++;
    }
    
    //E

    gd->tables_allocated = TRUE;
    gd->bytes_tot += bytes_tot_local;
    return SUCCESS;

    cleanup:
        for (int i = 0; i < zv_rows; i++)
            free(zv.result[i]);
        free(zv.result);
        free(zv.r2);
        free(zv.r1);

        for (int m = 0; m < bm_rows; m++) {
            free(iv.BmVectorsp[m]);
            free(iv.BmVectors[m]);
        }
        free(iv.BmVectorsp);
        free(iv.BmVectors);

        free(iv.ellT);
        free(iv.q_chiint);
        free(iv.neff_chiint);
        free(iv.rsigma_chiint);
        free(iv.DpT_chiint);
        free(iv.chiT_chiint);
        free(iv.zT_chiint);

        free(gv.DpT);
        free(gv.chiOfzT);
        free(gv.zT);
        free(gv.gLT);
        free(gv.chiforgLT);

        gd->tables_allocated = FALSE;
        return status;
}

#ifdef OPENMPCODE
int SetNumberThreads(struct  cmdline_data *cmd)
{
    if (cmd == NULL || cmd->numthreads < 1) return FAILURE;
    
    omp_set_num_threads(cmd->numthreads);

    return SUCCESS;
}
#else // dummy for no OPENMCODE
int SetNumberThreads(struct  cmdline_data *cmd)
{
    return SUCCESS;
}
#endif

local int print_make_info(struct cmdline_data* cmd,
                          struct  global_data* gd)
{
    verb_print(cmd->verbose,
               "\nprint_make_info:\n");

#ifdef OPENMPCODE
    verb_print(cmd->verbose, "using OpenMP\n");
#endif

#ifdef DEBUG
    verb_print(cmd->verbose, "DEBUG\n");
#endif

#ifdef USEGSL
    verb_print(cmd->verbose, "using GSL\n");
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

//B I/O directories:
local int setFilesDirs_log(struct cmdline_data* cmd,
                             struct  global_data* gd)
{
    string routineName = "setFilesDirs_log";

    int nwrite;

    // gd->logfilePath is defined
    if (cmd->verbose_log>0 && gd->rootDirFlag==TRUE) {
        nwrite = snprintf(gd->tmpDir, sizeof(gd->tmpDir),
                          "%s/%s", cmd->rootDir, "tmp");
        if (nwrite < 0 || (size_t)nwrite >= sizeof(gd->tmpDir)) {
            WLCOV_FAIL(cmd, "\n%s: tmpDir too long\n\n",routineName);
        }
        
        if (mkdir_p(gd->tmpDir, 0755) == 0) {
        } else {
            WLCOV_FAIL(cmd, "\n%s: Error creating directory: %s\n\n",
                       routineName, gd->tmpDir);
        }
        
        nwrite = snprintf(gd->logfilePath, sizeof(gd->logfilePath),
                          "%s/wlcf.log", gd->tmpDir);
        if (nwrite < 0 || (size_t)nwrite >= sizeof(gd->logfilePath)) {
            WLCOV_FAIL(cmd, "\n%s: logfilePath too long\n",routineName);
        }

    }

    return SUCCESS;
}

local int setFilesDirs(struct cmdline_data* cmd, struct global_data* gd)
{
    string routineName = "setFilesDirs";

    if (gd->rootDirFlag == TRUE) {
        if (strlen(cmd->rootDir) >= MAXLENGTHOFFILES) {
            WLCOV_FAIL(cmd, "%s: rootDir too long\n", routineName);
        }

        if (mkdir_p(cmd->rootDir, 0755) != 0) {
            WLCOV_FAIL(cmd, "%s: Error creating directory: %s",
                       routineName, cmd->rootDir);
        }
    }

    return SUCCESS;
}

int StartOutput(struct cmdline_data *cmd, struct  global_data* gd)
{
    if (cmd == NULL || gd == NULL)
        return FAILURE;
    
    if (cmd->options == NULL) cmd->options = "";

    if (cmd->verbose>=VERBOSEMININFO)
        if (! strnull(cmd->options))
            verb_print(cmd->verbose, "\n\toptions: %s\n", cmd->options);

    return SUCCESS;
}
//E
