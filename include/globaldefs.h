/*==============================================================================
 HEADER: globaldefs.h        [wlcf]
 Starting date: 15.02.2026
 Purpose: Definitions of global variables and parameters
 Language: C
 Use: '#include "globaldefs.h"
 Major revisions:
 ==============================================================================*/
//        1          2          3          4        ^ 5          6          7
#ifndef _globaldefs_h
#define _globaldefs_h

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#include <string.h>
#include "stdinc.h"
#include "getparam.h"
#include "common_defs.h"

#ifdef OPENMPCODE
#include <omp.h>
#endif

//B socket:
#ifdef ADDONS
#include "globaldefs_include_01.h"
#endif
//E

#if !defined(global)
#  define global extern
#endif

typedef char *string;

//B socket:
#ifdef ADDONS
#include "datastruc_tables.h"
#endif
//E

//B CLASSLIB section
#ifdef CLASSLIB
#include "common.h"
#endif
global ErrorMsg errmsg;
//E

#include "cmdline_data.h"
#include "global_data.h"


//B socket:
#ifdef ADDONS
#include "globaldefs_include_02.h"
#endif
//E

#include "protodefs.h"

#endif  // ! _globaldefs_h
