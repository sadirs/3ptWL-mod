/*==============================================================================
 NAME: main.c                [wlcf]
 Written by: A. Aviles et al.
 Starting date: february 2026
 Purpose: Main routine
 Language: C
 Major revision:
 ===============================================================================
 Use: wlcf --help (or -h)
 Input:     Command line parameters, Parameters file, data catalogs
 Output: several histograms containing 2pcf, 3pcf,...
 Units:
 History:
 Acknowledgements: ...
 Comments and notes:
 References: Zeno project, NR, GSL, FFTLog, FFTW3,...
 github: https://github.com/rodriguezmeza/wlcf.git
 Publication: cite: JCAP12(2024)049 (ArXiv ePrint: 2408.16847)
 ==============================================================================*/
// ============================================================================
//        1          2          3          4        ^ 5          6          7

#define global

#include <stdio.h>
#include <stdlib.h>

#include "globaldefs.h"
#include "procedures.h"
#include "functions.h"
#include "background.h"
#include "zetam.h"

#include "cmdline_defs.h"

// to make validation erros shout loud
static void print_failure(struct cmdline_data *cmd, const char *stage)
{
    if (cmd->error_message[0] != '\0')
        fprintf(stderr, "\n%s failed:\n%s\n", stage, cmd->error_message);
    else
        fprintf(stderr, "\n%s failed with no error message.\n", stage);
}

/*
 Main routine:
 
 This routine is in charge of main computational flow
    as it is explained below in the comments.

 Arguments:
    * `argc`: Input: int
    * `argv`: Input: string array
 Return (the error status):
    int SUCCESS or FAILURE
 */
int main(int argc, string argv[])
{
    struct cmdline_data cmd = {0};                  // share command parameters
    struct global_data gd = {0};                    // share global parameters
    int status = SUCCESS;

    gd.cpuinit = CPUTIME;                           // init register of cpu time
    gd.cpurealinit = rcpu_time();                   // init register of real time

    InitParam(argv, defv);

    if (StartRun(&cmd, &gd, argv[0],                // get parameters and
                 HEAD1, HEAD2, HEAD3) == FAILURE) { //  init global structure
        print_failure(&cmd, "StartRun");            //  and do other useful
        status = FAILURE;                           //  process, like param
        goto cleanup;                               //  check, ...
    }
    
    if (Initial(&cmd, &gd) == FAILURE) {
        print_failure(&cmd, "Initial");
        status = FAILURE;
        goto cleanup;
    }

    if (MainLoop(&cmd, &gd) == FAILURE) {           // do the job
        print_failure(&cmd, "MainLoop");
        status = FAILURE;
        goto cleanup;
    }

cleanup:
    if (EndRun(&cmd, &gd) == FAILURE) {             // close streams and free mem
        print_failure(&cmd, "EndRun");
        status = FAILURE;
    }

    return status == SUCCESS ? EXIT_SUCCESS : EXIT_FAILURE;
}


