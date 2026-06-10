
AddOns include folder

Files of the wl3pcf folders tree are:

1. include folder
	- cmdline_defs.h
	- datastruc_defs.h
	- globaldefs.h
	- protdefs.h
2. main
	- main.c
3. source
	- startrun.c
	- testdata.c
	- wl3pcf_io.c
	- wl3pcf.c
	- wl3pcfutils.c
4. addons folder

Any possible AddOn item will be inserted into corresponding files above that are located in the addons/addons_include folder.

Each of the files above has lines like:

#ifdef ADDONS
#include "xxx_include_yyy.h"
#endif

Look for them and think where your AddOn items will be inserted on. Then go to the corresponding  file ("xxx_include.h") and edit it.


Other files and folders are auxiliaries:

1. docs
2. general_lib
3. getparam
4. gsl
5. Makefiles_xxx
6. tests




