# ----- MAKE FILE FOR wlcf CODE -----
# Mario A. Rodriguez-Meza, Ciudad de Mexico, 15.02.2026
#
#
# Nothing to do in this file. Make your settings in Makefile_settings file only
#
MACHINES_DIR = ./
# Machine definitions and code settings. Edit this file according to your needs
include $(MACHINES_DIR)Makefile_settings

#
# Nothing to do below
#

EXECPREFIX = wlcf
EXEC = $(EXECPREFIX)

$(info )
$(info =====================================================)
$(info EXEC = [${EXEC}])
$(info External options = [${OPT2}])
$(info =====================================================)
$(info )

MAIN = main.o

OBJS = main.o background.o functions.o libs.o \
		procedures.o tests.o twobessel.o utils.o \
		zetam.o

#ifeq ($(GETPARAM_ON),1)
OBJS += startrun.o
#endif

PYTHON_FILES = python/fkpt2.pyx python/setup.py python/cfkpt2.pxd

all: $(EXEC) lib$(EXEC).a wlcfpy
#all: $(EXEC) lib$(EXEC).a

lib$(EXEC).a: $(OBJS) $(EXTERNAL)
	$(AR)  $@ $(addprefix build/, $(OBJS) $(TOOLS) $(SOURCE) $(EXTERNAL) $(EXTERNALCXX))

$(EXEC): $(OBJS) $(EXTERNAL) $(MAIN)
	$(CC) $(OPTFLAG) $(OMPFLAG) $(LDFLAG) -o $(EXEC) $(addprefix build/,$(notdir $^)) $(MLIBS)
#	 $(FITSIOLIBS)

wlcfpy: lib$(EXEC).a python/wlcfpy.pyx python/cwlcfpy.pxd
	export CC=$(CC); output=$$($(PYTHON) -m pip install . 2>&1); \
    echo "$$output"; \
    if echo "$$output" | grep -q "ERROR: Cannot uninstall"; then \
        site_packages=$$($(PYTHON) -c "import distutils.sysconfig; print(distutils.sysconfig.get_python_lib())" || $(PYTHON) -c "import site; print(site.getsitepackages()[0])") && \
        echo "Cleaning up previous installation in: $$site_packages" && \
        rm -rf $$site_packages/wlcfpy* && \
        $(PYTHON) -m pip install .; \
    fi

.PHONY : clean
clean: .base
	rm -rf $(WRKDIR);
	rm -f $(EXEC)
	rm -f lib$(EXEC).a
	rm -f $(MDIR)/python/wlcfpy.c
	rm -rf $(MDIR)/python/build
	rm -rf wlcfpy.egg-info
