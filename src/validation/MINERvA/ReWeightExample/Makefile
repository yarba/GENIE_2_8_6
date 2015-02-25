#
# Makefile for the package to re-weight GENIE events (by MaCCQE)
#

SHELL = /bin/sh
NAME = all
MAKEFILE = Makefile

# Include machine specific flags and locations (inc. files & libs)
#
include $(GENIE)/src/make/Make.include

# --> CXXFLAGS += -g

GENIE_LIBS   = $(shell $(GENIE)/src/scripts/setup/genie-config --libs)

INCLUDES += -I../ReWeightIO

LIBRARIES   := $(GENIE_LIBS) $(LIBRARIES) $(CERN_LIBRARIES)
LIBRARIES   += -L../ReWeightIO -l ReWeightIO

RW_SOURCES   := $(wildcard *.cxx)
RW_OBJECTS    = $(addsuffix .$(ObjSuf), $(basename $(RW_SOURCES)))

all:    rw_app 

rw_app: FORCE
	$(CXX) $(CXXFLAGS) -c $(RW_SOURCES) $(INCLUDES)
	$(LD) $(LDFLAGS) $(RW_OBJECTS) $(LIBRARIES) -o  rw_app

purge: FORCE
	$(RM) *.o *~ core

clean: FORCE
	$(RM) *.o *~ core
	$(RM) rw_app

distclean: FORCE
#	$(RM) $(GENIE_BIN_INSTALLATION_PATH)/rw_app

FORCE:

# DO NOT DELETE


