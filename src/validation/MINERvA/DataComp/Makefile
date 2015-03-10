#
# Makefile for the package to validate GENIE vs MINERvA data
#

SHELL = /bin/sh
NAME = all
MAKEFILE = Makefile

# Include machine specific flags and locations (inc. files & libs)
#
include $(GENIE)/src/make/Make.include

# --> CXXFLAGS += -g

PACKAGE           = VldMINERvA
DICTIONARY        = _ROOT_DICT_VldMINERvA
LIBNAME           = libGVldMINERvA
EXTRA_EXT_LIBS    = 

all     : rootcint lib lib-link
install : install-inc install-lib

# Include standard package makefile targets
#
include $(GENIE)/src/make/Make.std-package-targets

FORCE:
