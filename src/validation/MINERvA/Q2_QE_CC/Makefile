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

# --> CXXFLAGS += -g

GENIE_LIBS   = $(shell $(GENIE)/src/scripts/setup/genie-config --libs)

# include RooMUHisto package
#
INCLUDES += -I$(ROOMU_DIR)
INCLUDES += -I$(ROOMU_DIR)/PlotUtils
#
LIBRARIES   := $(GENIE_LIBS) $(LIBRARIES) $(ROOMU_LIBS) $(CERN_LIBRARIES)

VAL_TEST_SOURCES   := ExpData.cxx IntType.cxx MCAnalysis.cxx
VAL_TEST_SOURCES   += $(wildcard *Analyzer*.cxx)
VAL_TEST_SOURCES   += $(wildcard *DataMC*.cxx)
VAL_TEST_SOURCES   += RunConfig.cxx
VAL_TEST_SOURCES   += testMINERvA.cxx

VAL_TEST_OBJECTS    = $(addsuffix .$(ObjSuf), $(basename $(VAL_TEST_SOURCES)))

all:    testMINERvA 

testMINERvA: FORCE
	$(CXX) $(CXXFLAGS) -c $(VAL_TEST_SOURCES) $(INCLUDES)
	$(LD) $(LDFLAGS) $(VAL_TEST_OBJECTS) $(LIBRARIES) -o  gvld_MINERvA_Q2_QE_CC
	$(COPY) gvld_MINERvA_Q2_QE_CC $(GENIE_BIN_PATH)/gvld_MINERvA_Q2_QE_CC 

purge: FORCE
	$(RM) *.o *~ core

clean: FORCE
	$(RM) *.o *~ core
	$(RM) gvld_MINERvA_Q2_QE_CC
	$(RM) $(GENIE_BIN_PATH)/gvld_MINERvA_Q2_QE_CC

distclean: FORCE
	$(RM) $(GENIE_BIN_INSTALLATION_PATH)/gvld_MINERvA_Q2_QE_CC

FORCE:

# DO NOT DELETE


