#
# Makefile for the package to validate GENIE vs MINERvA data
#

SHELL = /bin/sh
NAME = all
MAKEFILE = Makefile

# Include machine specific flags and locations (inc. files & libs)
#
include $(GENIE)/src/make/Make.include
# --> debug-dev purposes only !!!   include misc-make-config/Make.include

CXXFLAGS += -g

GENIE_LIBS   = $(shell $(GENIE)/src/scripts/setup/genie-config --libs)

# include RooMUHisto pacakge
#
# tmp pointer to local setup
# needs to be more general in the future 
#
# ROOMU_SYS = /home/yarba_j/work/genie-work-area/RooMUHistos

INCLUDES += -I$(ROOMU_SYS) 
INCLUDES += -I$(ROOMU_SYS)/PlotUtils

LIBRARIES   := $(GENIE_LIBS) $(LIBRARIES) -L$(ROOMU_SYS)/lib -lplotutils $(CERN_LIBRARIES)

VAL_TEST_SOURCES   := ExpData.cxx IntType.cxx MCAnalysis.cxx
VAL_TEST_SOURCES   += $(wildcard *Analyzer*.cxx)
VAL_TEST_SOURCES   += $(wildcard *DataMC*.cxx)
# VAL_TEST_SOURCES   += MUH1D.cxx 
# VAL_TEST_SOURCES   += MUVertErrorBand.cxx 
# MUPlotter.cxx
VAL_TEST_SOURCES   += testMINERvA.cxx

VAL_TEST_OBJECTS    = $(addsuffix .$(ObjSuf), $(basename $(VAL_TEST_SOURCES)))

all:    testMINERvA 

testMINERvA: FORCE
	$(CXX) $(CXXFLAGS) -c $(VAL_TEST_SOURCES) $(INCLUDES)
	$(LD) $(LDFLAGS) $(VAL_TEST_OBJECTS) $(LIBRARIES) -o tstMINERvA

purge: FORCE
	$(RM) *.o *~ core

clean: FORCE
	$(RM) *.o *~ core
	$(RM) ./tstMINERvA

distclean: FORCE
	$(RM) $(GENIE_BIN_INSTALLATION_PATH)/tstMINERvA

FORCE:

# DO NOT DELETE


