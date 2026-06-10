#include "global.h"
#include "utils.h"
#include "twobessel.h"


#define m_2PI2 39.4784176043574  // (2 pi)^2


//void get_zetam(void);
int get_zetam(struct cmdline_data* cmd,
              struct  global_data* gd);
//void free_variables(void);
int free_variables(struct cmdline_data* cmd,
                   struct  global_data* gd);
void allocate_zv(void);

