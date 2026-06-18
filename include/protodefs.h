/*==============================================================================
 HEADER: protodefs.h				[cTreeBalls]
 Written by: Mario A. Rodriguez-Meza
 Starting date: april 2023
 Purpose: Definitions of global prototypes
 Language: C
 Use: '#include "protodefs.h"
 Major revisions:
 ==============================================================================*/
//        1          2          3          4        ^ 5          6          7

//
// lines where there is a "//B socket:" string are places to include module files
//  that can be found in addons/addons_include folder
//

#ifndef _protodefs_h
#define _protodefs_h

#ifdef __cplusplus
extern "C" {
#endif

int MainLoop(struct cmdline_data* cmd, struct  global_data* gd);

int Initial(struct cmdline_data* cmd, struct  global_data* gd);

int StartRun(struct cmdline_data* cmd, struct  global_data* gd,
             string, string, string, string);

int StartRun_Common(struct cmdline_data*, struct  global_data*);
int PrintParameterFile(struct cmdline_data *, struct  global_data*, char *);
int SetNumberThreads(struct cmdline_data *cmd);
int StartOutput(struct cmdline_data *, struct  global_data*);
int EndRun(struct cmdline_data* cmd, struct  global_data* gd);
int startrun_memoryAllocation(struct cmdline_data* cmd, struct  global_data* gd);
int EndRun_FreeMemory(struct cmdline_data* cmd,
                             struct  global_data* gd);
int EndRun_FreeMemory_tables(struct cmdline_data* cmd,
                             struct  global_data* gd);
int EndRun_FreeMemory_cmd(struct cmdline_data* cmd,
                             struct  global_data* gd);
int EndRun_FreeMemory_gd(struct cmdline_data* cmd,
                             struct  global_data* gd);

//B socket:
#ifdef ADDONS
// If you have an addon that need global proto definitions
//  go to this file and add the addon item.
#include "protodefs_include.h"
#endif
//E

// Application Binary Interface (ABI) definitions
size_t sizeof_cmdline_data(void);
size_t sizeof_global_data(void);


#ifdef __cplusplus
}
#endif

#endif // ! _protodefs_h
