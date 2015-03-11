#
# Makefile for the package to validate GENIE vs MINERvA data
#

SHELL = /bin/sh
NAME = all
MAKEFILE = Makefile

# Include machine specific flags and locations (inc. files & libs)
#
include $(GENIE)/src/make/Make.include
# include misc-make-config/Make.include

LIBNAME = libGVldMINERvACommon

# --> CXXFLAGS += -g

GENIE_LIBS   = $(shell $(GENIE)/src/scripts/setup/genie-config --libs)
GENIE_LIBS   += -lGVldMINERvA -lGVldTuning -lGVldMINERvAReWeightIO

# include RooMUHisto package
#
# tmp stuff
# ROOMU_DIR = $(ROOMU_SYS)
# ROOMU_LIB_DIR = $(ROOMU_SYS)/lib
# ROOMU_LIBS = -L$(ROOMU_LIB_DIR) -l plotutils
#
INCLUDES += -I$(ROOMU_DIR)
INCLUDES += -I$(ROOMU_DIR)/PlotUtils

LIBRARIES   := $(GENIE_LIBS) $(LIBRARIES) $(ROOMU_LIBS) $(CERN_LIBRARIES)

MINERvA_COMMON_SOURCES   += $(wildcard *.cxx)

MINERvA_COMMON_OBJECTS    = $(addsuffix .$(ObjSuf), $(basename $(MINERvA_COMMON_SOURCES)))

all:    lib 

lib: FORCE
	@echo "[package: MINERvA/Common] Generating shared library ..."
	$(CXX) $(CXXFLAGS) -c $(MINERvA_COMMON_SOURCES) $(INCLUDES)
	$(SOCMD) $(SOFLAGS) $(SOMINF) $(MINERvA_COMMON_OBJECTS) $(EXTRA_EXT_LIBS) $(OutPutOpt) $(LIBNAME).$(DllSuf)
	$(COPY) $(LIBNAME).$(DllSuf) $(GENIE_LIB_PATH)/$(LIBNAME)$(GLIBVERSION).$(DllSuf)
	$(SYMLINK) $(LIBNAME)$(GLIBVERSION).$(DllSuf) $(GENIE_LIB_PATH)/$(LIBNAME).$(DllSuf)
	
clean: FORCE
	$(RM) *.o *~ core
	$(RM) *.d
	$(RM) *.so

distclean: FORCE

FORCE:



