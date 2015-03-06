#
# Makefile for the package to re-weight GENIE events (by MaCCQE)
#

SHELL = /bin/sh
NAME = all
MAKEFILE = Makefile

# Include machine specific flags and locations (inc. files & libs)
#
include $(GENIE)/src/make/Make.include

LIBNAME = libGVldMINERvAReWeightIO

# --> CXXFLAGS += -g

GENIE_LIBS   = $(shell $(GENIE)/src/scripts/setup/genie-config --libs)

LIBRARIES   := $(GENIE_LIBS) $(LIBRARIES) $(CERN_LIBRARIES)

RW_SOURCES   := $(wildcard *RW*.cxx)

RW_OBJECTS    = $(addsuffix .$(ObjSuf), $(basename $(RW_SOURCES)))

DICTIONARY = _ROOT_DICT_ReWeightIO
DICTGEN_SOURCES = $(wildcard *RW*.cxx)
DICTGEN_HEADERS     := $(addsuffix .h, $(basename $(DICTGEN_SOURCES)))
DICTGEN_OBJECTS     := $(addsuffix .$(ObjSuf), $(basename $(DICTGEN_HEADERS)))
RW_SOURCES          := $(RW_SOURCES) $(DICTIONARY).cc $(wildcard *.cpp)

all:    rootcint lib

rootcint: $(DICTIONARY).o

$(DICTIONARY).cc: LinkDef.h $(DICTGEN_OBJECTS)
	$(RM) $(DICTIONARY).*
	@echo "[package: MINERvA/ReWeightIO] Generating ROOT dictionary ..."
	$(ROOTSYS)/bin/rootcint -f $(DICTIONARY).cc -c -I$(ROOTSYS)/include $(DICTGEN_HEADERS) LinkDef.h

lib: $(RW_SOURCES)
	@echo "[package: MINERvA/ReWeightIO] Generating shared library ..."
	$(SOCMD) $(SOFLAGS) $(SOMINF) $(RW_OBJECTS) $(EXTRA_EXT_LIBS) $(OutPutOpt) $(LIBNAME).$(DllSuf)
	$(COPY) $(LIBNAME).$(DllSuf) $(GENIE_LIB_PATH)/$(LIBNAME)$(GLIBVERSION).$(DllSuf)
	$(SYMLINK) $(LIBNAME)$(GLIBVERSION).$(DllSuf) $(GENIE_LIB_PATH)/$(LIBNAME).$(DllSuf) 

clean: FORCE
	$(RM) *.o *~ core
	$(RM) *.d
	$(RM) $(DICTIONARY).*
	$(RM) *.so

distclean: FORCE
	$(RM) $(GENIE_LIB_PATH)/$(LIBNAME).$(DllSuf)
	$(RM) $(GENIE_LIB_PATH)/$(LIBNAME)$(GLIBVERSION).$(DllSuf)

FORCE:

# DO NOT DELETE


