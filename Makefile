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

ifeq ($(ADDONSON),0)
OBJS = main.o startrun.o wlcf.o wlcfio.o background.o functions.o libs.o \
		procedures.o tests.o twobessel.o utils.o \
		zetam.o
else
OBJS = main.o startrun.o wlcf.o wlcfio.o background.o functions.o libs.o \
		procedures.o tests.o twobessel.o utils.o \
		zetam.o abi_check.o
endif

PYTHON_FILES = python/wlcfpy.pyx setup.py python/cwlcfpy.pxd.in

all: $(EXEC) lib$(EXEC).a wlcfpy

lib$(EXEC).a: $(OBJS) $(EXTERNAL)
	$(AR)  $@ $(addprefix $(WRKDIR)/, $(OBJS) $(TOOLS) $(SOURCE) $(EXTERNAL) $(EXTERNALCXX))

$(EXEC): $(OBJS) $(EXTERNAL) $(MAIN)
	$(CC) $(OPTFLAG) $(OMPFLAG) $(LDFLAG) -o $(EXEC) $(addprefix $(WRKDIR)/,$(notdir $^)) $(MLIBS)

ifeq ($(CLASSLIBON),1)
wlcfpy: python/wlcfpy.pyx python/cwlcfpy.pxd.in setup.py Makefile_settings addons/Makefile_addons_settings
	export CC=$(CC); \
	output=$$($(PYTHON) -m pip install . 2>&1); \
	status=$$?; \
	echo "$$output"; \
	if [ $$status -ne 0 ]; then \
	    if echo "$$output" | grep -q "ERROR: Cannot uninstall"; then \
	        site_packages=$$($(PYTHON) -c "import sysconfig; print(sysconfig.get_paths()['purelib'])" || $(PYTHON) -c "import site; print(site.getsitepackages()[0])"); \
	        echo "Cleaning up previous installation in: $$site_packages"; \
	        rm -rf "$$site_packages"/wlcfpy*.egg-info; \
	        rm -rf "$$site_packages"/wlcfpy*.dist-info; \
	        $(PYTHON) -m pip install .; \
	    else \
	        exit $$status; \
	    fi; \
	fi
else
wlcfpy:
endif

.PHONY : clean
clean: .base
	rm -rf $(WRKDIR);
	rm -f $(EXEC)
	rm -f lib$(EXEC).a
	rm -f $(MDIR)/python/wlcfpy.c
	rm -rf $(MDIR)/python/build
	rm -rf wlcfpy.egg-info
	rm -f $(MDIR)/python/cwlcfpy.pxd

