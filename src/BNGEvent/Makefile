MAKEFILE = GNUmakefile

ifeq ($(findstring k,$(MAKEFLAGS)),k)
  KEEP_GOING = -
endif

include $(ACE_ROOT)/include/makeinclude/macros.GNU

all: BNGEvent

REMAINING_TARGETS := $(filter-out all,$(TARGETS_NESTED:.nested=)) $(CUSTOM_TARGETS)

$(REMAINING_TARGETS):
	$(KEEP_GOING)@$(MAKE) -f Makefile.event static_libs=1 $(@)

.PHONY: BNGEvent
BNGEvent:
	$(KEEP_GOING)@$(MAKE) -f Makefile.event all static_libs=1

project_name_list:
	@echo BNGEvent
