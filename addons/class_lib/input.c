/*==============================================================================
 * input.c module.
 *
 * Julien Lesgourgues, 27.08.2010
 *
 * Adapted to be used in wlcov by Mario A. Rodriguez-Meza
==============================================================================*/
//        1          2          3          4        ^ 5          6          7

//
// lines with a "//B socket:" string are places to include module files
//  that can be found in addons/addons_include folder
//

#include "globaldefs.h"
#include "input.h"
#include <limits.h>
#include <errno.h>


local int testParameterFile(struct cmdline_data*,
                            struct global_data*,
                            char *,
                            ErrorMsg);

int input_find_file(struct  cmdline_data* cmd, struct  global_data* gd,
                    char *fname,
                    struct file_content * fc,
                    ErrorMsg errmsg){

  struct file_content fc_input;
  struct file_content fc_precision;
  struct file_content * pfc_input;
  struct file_content fc_setroot;

  char input_file[_ARGUMENT_LENGTH_MAX_];
  char precision_file[_ARGUMENT_LENGTH_MAX_];

    //B test if cmd->paramfile exist...
    if (!strnull(fname)) {
        class_call(testParameterFile(cmd, gd, cmd->paramfile, errmsg),
                   errmsg,
                   errmsg);
    }
    //E

  pfc_input = &fc_input;

    fc->size = 0;
    fc->filename = NULL;
    fc->name = NULL;
    fc->value = NULL;
    fc->read = NULL;

    fc_input.size = 0;
    fc_input.filename = NULL;
    fc_input.name = NULL;
    fc_input.value = NULL;
    fc_input.read = NULL;

    fc_precision.size = 0;
    fc_precision.filename = NULL;
    fc_precision.name = NULL;
    fc_precision.value = NULL;
    fc_precision.read = NULL;

    fc_setroot.size = 0;
    fc_setroot.filename = NULL;
    fc_setroot.name = NULL;
    fc_setroot.value = NULL;
    fc_setroot.read = NULL;


  input_file[0]='\0';
  precision_file[0]='\0';

    stream outstr;
    if (strnull(fname)) {
        verb_print(1,
    "If you intend to use a parameter file call with <ParameterFileName>\n");
        return FAILURE;
    } else {
        outstr = stropen(fname, "r");
        fclose(outstr);
    }

    if (copy_checked(input_file, sizeof(input_file), fname, "input file") != 0)
        return FAILURE;

    if (!strnull(input_file)) {
        
        class_call_except(parser_read_file(input_file, &fc_input, errmsg),
                          errmsg,
                          errmsg,
                          parser_free(&fc_input););

        class_call_except(input_set_root(input_file, &pfc_input,
                                         &fc_setroot, errmsg),
                          errmsg,
                          errmsg,
                          parser_free(pfc_input);
                          parser_free(&fc_setroot););

  }

  if ((input_file[0]!='\0') || (precision_file[0]!='\0')){
      
      class_call_except(parser_cat(pfc_input, &fc_precision, fc, errmsg),
                        errmsg,
                        errmsg,
                        parser_free(pfc_input);
                        parser_free(&fc_precision);
                        parser_free(fc););

  }

  class_call(parser_free(pfc_input), errmsg, errmsg);
  class_call(parser_free(&fc_precision), errmsg, errmsg);

  return SUCCESS;
}

int input_set_root(char* input_file,
                   struct file_content** ppfc_input,
                   struct file_content * pfc_setroot,
                   ErrorMsg errmsg) {

    int flag1;
  int index_root_in_fc_input = -1;
  int overwrite_root;

  FileArg outfname;

  struct file_content fc_root;                      // Temporary structure with
                                                    //  only the root name
  FileArg string1;                                  // Is ignored
  struct file_content * pfc = *ppfc_input;

  class_call(parser_read_string(pfc,"rootDir",&string1,&flag1,errmsg),
             errmsg, errmsg);

//B To behave as not class_lib parameter file
    overwrite_root = TRUE;
    class_read_flag("overwrite_root",overwrite_root);
    overwrite_root = TRUE;
//E

    if (flag1 == FALSE) {
        if (copy_checked(outfname, sizeof(outfname), "Output", "rootDir") != 0)
            return FAILURE;
    } else {
        for (index_root_in_fc_input=0; index_root_in_fc_input<pfc->size;
             ++index_root_in_fc_input) {
            if (strcmp(pfc->name[index_root_in_fc_input], "rootDir") == 0) {
                if (copy_checked(outfname, sizeof(outfname),
                                 pfc->value[index_root_in_fc_input],
                                 "rootDir") != 0)
                    return FAILURE;
                break;
            }
        }
    }

    if(flag1 == FALSE) {
        class_call(parser_init(&fc_root, 1, pfc->filename, errmsg),
                   errmsg,errmsg);
        if (copy_checked(fc_root.name[0], sizeof(FileArg), "rootDir", "rootDir name") != 0) {
            parser_free(&fc_root);
            return FAILURE;
        }

        if (copy_checked(fc_root.value[0], sizeof(FileArg),
                         outfname, "rootDir value") != 0) {
            parser_free(&fc_root);
            return FAILURE;
        }

        fc_root.read[0] = FALSE;
        
        class_call_except(parser_cat(pfc, &fc_root, pfc_setroot, errmsg),
                          errmsg,
                          errmsg,
                          parser_free(pfc);
                          parser_free(&fc_root););

        class_call(parser_free(pfc), errmsg, errmsg);
        class_call(parser_free(&fc_root), errmsg, errmsg);
        (*ppfc_input) = pfc_setroot;

    } else {
        if (copy_checked(pfc->value[index_root_in_fc_input], sizeof(FileArg),
                         outfname, "rootDir value") != 0)
            return FAILURE;

        (*ppfc_input) = pfc;
    }

  return SUCCESS;
}

int input_read_from_file(struct cmdline_data *cmd, struct  global_data* gd,
                         struct file_content * pfc,
                         ErrorMsg errmsg)
{
    int input_verbose = 0;

    if (gd->startrun_cputime==FALSE) {
        gd->cpuinit = CPUTIME;                       // init of cpu time
        gd->cpurealinit = rcpu_time();               // init of real time
    }

    gd->cmd_allocated = FALSE;

    class_read_int("verbose",input_verbose);
    verb_print(input_verbose, "\nReading input parameters...\n");

    class_call(input_read_parameters(cmd, gd, pfc, errmsg),errmsg,errmsg);

    return SUCCESS;
}

int input_read_parameters(struct cmdline_data *cmd,
                          struct  global_data* gd,
                          struct file_content * pfc,
                          ErrorMsg errmsg)
{
    int input_verbose=0;

    class_call(input_default_params(cmd),errmsg,errmsg);
    class_read_int("input_verbose",input_verbose);
    class_call(input_read_parameters_general(cmd, gd, pfc, errmsg),
               errmsg, errmsg);

    gd->cmd_allocated = TRUE;

    return SUCCESS;
}

int input_read_parameters_general(struct cmdline_data *cmd,
                                  struct  global_data* gd,
                                  struct file_content * pfc, ErrorMsg errmsg)
{

#define BASE_FREE_STRINGS_ON_FAILURE()          \
    do {                                        \
        if (gd->optionsFlag == TRUE) {          \
            free(cmd->options);                 \
            cmd->options = NULL;                \
            gd->optionsFlag = FALSE;            \
        }                                       \
        if (gd->rootDirFlagFree == TRUE) {      \
            free(cmd->rootDir);                 \
            cmd->rootDir = NULL;                \
            gd->rootDirFlagFree = FALSE;        \
        }                                       \
        if (gd->prefixFlag == TRUE) {           \
            free(cmd->prefix);                  \
            cmd->prefix = NULL;                 \
            gd->prefixFlag = FALSE;             \
        }                                       \
        if (gd->fWgchiFlag == TRUE) {           \
            free(cmd->fWgchi);                  \
            cmd->fWgchi = NULL;                 \
            gd->fWgchiFlag = FALSE;             \
        }                                       \
        if (gd->fnamePSFlag == TRUE) {          \
            free(cmd->fnamePS);                 \
            cmd->fnamePS = NULL;                \
            gd->fnamePSFlag = FALSE;            \
        }                                       \
    } while (0)


    int flag;
    int flag1;
    int param;
    int index;
    double param1;
    char string1[_ARGUMENT_LENGTH_MAX_];

    // All malloc have to be freed at the end of the run (EndRun)

    //B Parameters about the I/O file(s)
    class_call(parser_read_string(pfc,"fnamePS",&string1,&flag1,errmsg),
               errmsg,errmsg);
    gd->fnamePSFlag=FALSE;
    if (flag1 == TRUE) {
        for (index=0;index<pfc->size;++index){
          if (strcmp(pfc->name[index],"fnamePS") == 0){
              
              cmd->fnamePS = (char*) malloc(MAXLENGTHOFSTRSCMD * sizeof(char));
  
              if (cmd->fnamePS == NULL) {
                  BASE_FREE_STRINGS_ON_FAILURE();
                  return FAILURE;
              }

              if (copy_checked(cmd->fnamePS, MAXLENGTHOFSTRSCMD,
                               pfc->value[index], "fnamePS") != 0) {
                  free(cmd->fnamePS);
                  cmd->fnamePS = NULL;
                  BASE_FREE_STRINGS_ON_FAILURE();
                  return FAILURE;
              }
              gd->fnamePSFlag=TRUE;
            break;
          }
        }
    }

    // Output parameters
    class_call_except(parser_read_string(pfc,"fWgchi",&string1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    gd->fWgchiFlag=FALSE;
    if (flag1 == TRUE) {
        for (index=0;index<pfc->size;++index){
          if (strcmp(pfc->name[index],"fWgchi") == 0){

              cmd->fWgchi = (char*) malloc(MAXLENGTHOFSTRSCMD * sizeof(char));
              if (cmd->fWgchi == NULL) {
                  BASE_FREE_STRINGS_ON_FAILURE();
                  return FAILURE;
              }

              if (copy_checked(cmd->fWgchi, MAXLENGTHOFSTRSCMD,
                               pfc->value[index], "fWgchi") != 0) {
                  free(cmd->fWgchi);
                  cmd->fWgchi = NULL;
                  BASE_FREE_STRINGS_ON_FAILURE();
                  return FAILURE;
                  
              }
              gd->fWgchiFlag=TRUE;
            break;
          }
        }
    }
    //E


    class_call(parser_read_string(pfc,"prefix",&string1,&flag1,errmsg),
               errmsg,errmsg);
    gd->prefixFlag=FALSE;
    if (flag1 == TRUE) {
        for (index=0;index<pfc->size;++index){
          if (strcmp(pfc->name[index],"prefix") == 0){
              
              cmd->prefix = (char*) malloc(MAXLENGTHOFSTRSCMD * sizeof(char));
  
              if (cmd->prefix == NULL) {
                  BASE_FREE_STRINGS_ON_FAILURE();
                  return FAILURE;
              }

              if (copy_checked(cmd->prefix, MAXLENGTHOFSTRSCMD,
                               pfc->value[index], "prefix") != 0) {
                  free(cmd->prefix);
                  cmd->prefix = NULL;
                  BASE_FREE_STRINGS_ON_FAILURE();
                  return FAILURE;
              }
              gd->prefixFlag=TRUE;
            break;
          }
        }
    }

    // Output parameters
    class_call_except(parser_read_string(pfc,"rootDir",&string1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    gd->rootDirFlagFree=FALSE;
    if (flag1 == TRUE) {
        for (index=0;index<pfc->size;++index){
          if (strcmp(pfc->name[index],"rootDir") == 0){

              cmd->rootDir = (char*) malloc(MAXLENGTHOFSTRSCMD * sizeof(char));
              if (cmd->rootDir == NULL) {
                  BASE_FREE_STRINGS_ON_FAILURE();
                  return FAILURE;
              }

              if (copy_checked(cmd->rootDir, MAXLENGTHOFSTRSCMD,
                               pfc->value[index], "rootDir") != 0) {
                  free(cmd->rootDir);
                  cmd->rootDir = NULL;
                  BASE_FREE_STRINGS_ON_FAILURE();
                  return FAILURE;
                  
              }
              gd->rootDirFlagFree=TRUE;
            break;
          }
        }
    }
    //E

    class_call_except(parser_read_double(pfc,"z",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->z = param1;
    }
    class_call_except(parser_read_double(pfc,"h",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->h = param1;
    }
    class_call_except(parser_read_double(pfc,"sigma8",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->sigma8 = param1;
    }
    class_call_except(parser_read_double(pfc,"Omb",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->Omb = param1;
    }
    class_call_except(parser_read_double(pfc,"Omc",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->Omc = param1;
    }
    class_call_except(parser_read_double(pfc,"ns",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->ns = param1;
    }
    class_call_except(parser_read_double(pfc,"w",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->w = param1;
    }
    class_call_except(parser_read_double(pfc,"Omnu",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->Omnu = param1;
    }

    //B
    class_call(parser_read_string(pfc, "writevectors", &string1, &flag1, errmsg),
               errmsg, errmsg);

    if (flag1 == TRUE) {
        if ((strcmp(string1, "1") == 0) ||
            (strcasecmp(string1, "true") == 0) ||
            (strcasecmp(string1, "yes") == 0) ||
            (strcasecmp(string1, "y") == 0)) {
            cmd->writevectors = TRUE;
        }
        else if ((strcmp(string1, "0") == 0) ||
                 (strcasecmp(string1, "false") == 0) ||
                 (strcasecmp(string1, "no") == 0) ||
                 (strcasecmp(string1, "n") == 0)) {
            cmd->writevectors = FALSE;
        }
        else {
            class_stop(errmsg,
                "incomprehensible input '%s' for the field 'writevectors'. "
                "Use true/false, yes/no, or 1/0.",
                string1);
        }
    }
    //E

    class_call_except(parser_read_int(pfc,"Wg",&param,&flag,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag == TRUE) cmd->Wg = param;

    //B Parameters to set a region in the sky, for example for Takahashi data
    class_call_except(parser_read_double(pfc,"zbin",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->zbin = param1;
    }
    
    class_call_except(parser_read_int(pfc,"mMax",&param,&flag,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag == TRUE) cmd->mMax = param;

    class_call_except(parser_read_int(pfc,"tree_level",&param,&flag,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag == TRUE) cmd->tree_level = param;

    class_call_except(parser_read_int(pfc,"chiQuadSteps",&param,&flag,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag == TRUE) cmd->chiQuadSteps = param;

    class_call_except(parser_read_int(pfc,"GLpoints",&param,&flag,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag == TRUE) cmd->GLpoints = param;

    class_call_except(parser_read_int(pfc,"Nell",&param,&flag,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag == TRUE) cmd->Nell = param;

    class_call_except(parser_read_double(pfc,"ellmax",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->ellmax = param1;
    }
    class_call_except(parser_read_double(pfc,"ellmin",&param1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE){
      cmd->ellmin = param1;
    }
    //E

    class_call_except(parser_read_int(pfc,"verbose",&param,&flag,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag == TRUE)
      cmd->verbose = param;
    class_call_except(parser_read_int(pfc,"verbose_log",&param,&flag,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag == TRUE)
      cmd->verbose_log = param;

#ifdef OPENMPCODE
    class_call_except(parser_read_int(pfc,"numberThreads",&param,&flag,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag == TRUE)
      cmd->numthreads = param;
#else
    class_call_except(parser_read_int(pfc,"numberThreads",&param,&flag,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag == TRUE)
      cmd->numthreads = 1;
#endif

    class_call_except(parser_read_string(pfc,"options",&string1,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    gd->optionsFlag=FALSE;
    if (flag1 == TRUE) {
        for (index=0;index<pfc->size;++index){
          if (strcmp(pfc->name[index],"options") == 0){
              
              cmd->options = (char*) malloc(MAXLENGTHOFSTRSCMD * sizeof(char));
              if (cmd->options == NULL) {
                  BASE_FREE_STRINGS_ON_FAILURE();
                  return FAILURE;
              }

              if (copy_checked(cmd->options, MAXLENGTHOFSTRSCMD,
                               pfc->value[index], "options") != 0) {
                  
                  free(cmd->options);
                  cmd->options = NULL;
                  BASE_FREE_STRINGS_ON_FAILURE();
                  return FAILURE;

              }
              gd->optionsFlag=TRUE;
            break;
          }
        }
    }

    //B base_path is needed in wlcovpy.px... but not needed in C wlcov code
    FileArg base_path_tmp;
    class_call_except(parser_read_string(pfc,"base_path",&base_path_tmp,&flag1,errmsg),
                      errmsg,
                      errmsg,
                      BASE_FREE_STRINGS_ON_FAILURE(););
    if (flag1 == TRUE) {
        
        if (copy_checked(cmd->base_path, sizeof(cmd->base_path),
                         base_path_tmp, "base_path") != 0) {
            BASE_FREE_STRINGS_ON_FAILURE();
            return FAILURE;
        }
        
    }
    //E

//B socket:
#ifdef ADDONS
#include "class_lib_include_01.h"
#endif
//

  return SUCCESS;
}

//B wlcf default values
int input_default_params(struct cmdline_data *cmd)
{
// Every item in cmdline_defs.h must have an item here::
    cmd->z = 1.0334;
    cmd->h = 0.7;
    cmd->sigma8 = 0.82;
    cmd->Omb = 0.046;
    cmd->Omc = 0.233;
    cmd->ns = 0.97;
    cmd->w = -1;
    cmd->Omnu = 0.0;

    // Parameters to set a region in the sky, for example for Takahashi data
    cmd->zbin = 0.5078;
    cmd->mMax = 5;
    cmd->chiQuadSteps = 300;
    cmd->GLpoints = 64;
    //E

    //B Parameters about the I/O file(s)
    cmd->fnamePS = "./input/linear_pk_Takahashi_z0.txt";
    cmd->prefix = "run1_";
    // Output parameters
    cmd->rootDir = "Output";
    cmd->tree_level = 3;
    //E

    //B Parameters to control histograms and their output files
    cmd->Nell = 128;
    cmd->ellmax = 10000.0;
    //
    cmd->ellmin = 0.001;
    cmd->Wg = 0;
    cmd->fWgchi = "./input/Wg_Takahashi_z05078.txt";
    //
    //E

    //B Miscellaneous parameters
    cmd->writevectors=1;
    cmd->verbose = 0;
    cmd->verbose_log = 0;
#ifdef OPENMPCODE
    cmd->numthreads = 16;
#else
    cmd->numthreads = 1;
#endif
    cmd->options = "\0";
    //E

    //B base_path is needed in wlcovpy.px... but not neede in C wlcov code
    if (copy_checked(cmd->base_path, sizeof(cmd->base_path),
                     __WLCFDIR__, "base_path") != 0)
        return FAILURE;
    //E

//B socket:
#ifdef ADDONS
#include "class_lib_include_02.h"
#endif
//

  return SUCCESS;
}
//E


//B parameter reading/testing from a file
local int testParameterFile(struct cmdline_data* cmd,
                            struct global_data* gd,
                            char *fname,
                            ErrorMsg errmsg)
{
// Every item in cmdline_defs.h must have an item here::
#define DOUBLE 1
#define STRING 2
#define INT 3
#define LONG 6
#define BOOLEAN 4
#define MAXTAGS 300
#define MAXCHARBUF 1024

    string routineName = "testParameterFile";
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
                snprintf(errmsg, _ERRORMSGSIZE_,
                         "%s: parameter name too long\n", routineName);
                if (fd != NULL) fclose(fd);
                return FAILURE;
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
                snprintf(errmsg, _ERRORMSGSIZE_,
                         "%s: parameter value too long\n", routineName);
                if (fd != NULL) fclose(fd);
                return FAILURE;
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
                            snprintf(errmsg, _ERRORMSGSIZE_,
                                     "%s: string parameter '%s' too long\n", routineName, name);
                            if (fd != NULL) fclose(fd);
                            return FAILURE;
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
                                snprintf(errmsg, _ERRORMSGSIZE_,
                                         "getbparam: %s=%s not bool\n", name, value);
                                if (fd != NULL) fclose(fd);
                                return FAILURE;
                            }
                        break;
                }
            } else {
                fprintf(stdout, "Error in file %s: Tag '%s' %s\n",
                        fname, name,
                        "not allowed or multiple defined...\n");
                errorFlag=1;
            }
        } // ! while loop
        fclose(fd);
    } else {
        snprintf(errmsg, _ERRORMSGSIZE_,
                 "Parameter file %s not found.\n", fname);
        return FAILURE;
    }

    if (errorFlag == 1) {
        snprintf(errmsg, _ERRORMSGSIZE_,
                 "%s: parameter file '%s' contains unknown or duplicated tags\n",
                 routineName, fname);
        return FAILURE;
    }

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
                    if (copy_checked((char *)addr[i], str_size[i], GetParam(tag[i]), tag[i]) != 0) {
                        snprintf(errmsg, _ERRORMSGSIZE_,
                                 "%s: default string parameter '%s' too long\n", routineName, tag[i]);
                        return FAILURE;
                    }
                    
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
            errorFlag=3;
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
