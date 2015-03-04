SHELL = /bin/sh
NAME = all
MAKEFILE = Makefile

include $(GENIE)/src/make/Make.include

MODULES := ReWeightIO ReWeightExample Q2_QE_CC

.PHONY: all $(MODULES)

all:     $(MODULES)

$(MODULES): 
	@echo "** Building... $@" 
	$(MAKE) -C $@ 

clean:
	@echo "** Cleaning..."
	$(foreach mod, $(MODULES), make -C $(mod) clean;)

distclean:
	@echo "** Cleaning..."
	$(foreach mod, $(MODULES), make -C $(mod) distclean;)
