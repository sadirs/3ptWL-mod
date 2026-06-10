# Set of declarations from C to python
# Only define quantities you will use,
# for input, output or intermediate manipulations
# by the python wrapper. The rest is internal in cBalls

cdef extern from "wlcf.h":

    ctypedef char FileArg[40]
    ctypedef char* ErrorMsg

    cdef struct cmdline_data:
        int numberThreads
        int verbose
        int verbose_log

    cdef struct global_data:
        short cmd_allocated
#B PXD functions
#E
# In common_defs.h #define MAXITEMS                100

    cdef struct file_content:
        char * filename
        int size
        FileArg * name
        FileArg * value
        short * read

    cdef int FAILURE
    cdef int FALSE
    cdef int TRUE

    int input_read_from_file(void*, void*, void*, char*)
    int StartRun_Common(void*, void*);
    int PrintParameterFile(void*, void*, char*)
    int StartOutput(void*)
    int SetNumberThreads(void*)
    int Initial(void*, void*);
    int MainLoop(void*, void*);

#B flags
#E

#B parameters
    int get_nthreads(void*, int*)
#B parameters

#B free memory
#E

#B histograms
#E histograms


