// Use:
//#include "wlcf_pxd_05.h"

// included in addons/addons_include/source/cballs_include_05.h

#ifndef _wlcf_pxd_05_h
#define _wlcf_pxd_05_h

//B parameters section

int get_nthreads(struct  cmdline_data* cmd, int *value)
{
    *value = cmd->numthreads;
    return SUCCESS;
}

//E

#endif	// ! _wlcf_pxd_05_h
