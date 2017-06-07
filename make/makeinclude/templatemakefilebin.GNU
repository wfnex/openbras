# -*- Makefile -*-

MAKEFILE         = GNUmakefile.$(MYBIN)
DEPENDENCY_FILE  = .depend.$(MYBIN)
BIN_UNCHECKED    = $(MYBIN)

LIB_PREFIX      ?= lib

FILES = $(MYFILES)

#----------------------------------------------------------------------------
#       Include macros and targets
#----------------------------------------------------------------------------
LDLIBS = -lACE

PRJ_TYPE = rtp

ifeq ($(INSBIN),.)
  ifeq ($(CURDIR),)
    ifeq ($(PWD),)
      PWD=$(shell pwd)
    endif
  else
    PWD=$(CURDIR)
  endif
  INSBIN = $(PWD)
endif

INSBIN=$(USER_ROOT)/bin
OUTPUT_DIRECTORY = $(if $(ARCH),./$(ARCH))

include $(USER_ROOT)/make/makeinclude/wrapper_macros.GNU


GNUACE_PROJECT_VERSION ?= $(USER_VERSION)

# To build multiple targets in the same directory on AIX, it works
# best to have a template directory per project.
# The compiler/linker isn't too smart about instantiating templates...
ifdef TEMPINCDIR
TEMPINCDIR := $(TEMPINCDIR)/$(MYFILES)
all: $(TEMPINCDIR)
endif

ifneq ($(OUTPUT_DIRECTORY),)
.PHONY: create_output_directory.local
all: create_output_directory.local
create_output_directory.local:
	-@$(MKDIR) "$(OUTPUT_DIRECTORY)" $(ACE_NUL_STDERR)
endif

# turn off libcheck if doing a dry run
ifeq ($(findstring n, $(MAKEFLAGS)),n)
  LIBCHECK = 1
else
  # turn off libcheck if keep going was passed too
  ifeq ($(findstring k, $(MAKEFLAGS)),k)
    LIBCHECK = 1
  else
    LIBCHECK ?= $(filter-out $(foreach lib,ACE,$(findstring $(lib),$(foreach libpath,. $(if $(ARCH),./$(ARCH)) ../../../lib $(if $(ARCH),../../../lib/$(ARCH)) $(USER_ROOT)/libs /opt/staros.xyz/stardlls/libs /usr/lib /usr/lib64 $(INSLIB),$(wildcard $(libpath)/$(LIB_PREFIX)$(lib).* $(libpath)/$(lib).lib)))),ACE)
    ifeq ($(LIBCHECK),)
      LIBCHECK = 1
    endif
  endif
endif
ifeq ($(LIBCHECK), 1)
BIN    = $(BIN_UNCHECKED)$(EXEEXT)
ifdef ARCH
  ARCH_BIN = $(ARCH)/$(BIN)
endif #ARCH
else
  all: lib_warning
endif

# If it contains ../ at all use notdir.
OBJS   = $(foreach var, $(addsuffix .$(OBJEXT), $(basename $(FILES)) $(RESOURCES)), $(if $(findstring ../,$(var)),$(notdir $(var)),$(var)))
SRC    = $(FILES)

include $(USER_ROOT)/make/makeinclude/macros.GNU
include $(USER_ROOT)/make/makeinclude/rules.common.GNU
include $(USER_ROOT)/make/makeinclude/rules.nonested.GNU

ifneq ($(INSBIN),)
ifneq ($(INSBIN),.)
  INSTALL = $(VBIN:%=$(INSBIN)/%)
  CLEANUP_INSTALL += $(CLEANUP_BIN:%=$(INSBIN)/$(if $(ARCH),$(ARCH)/)%$(VAR)$(EXEEXT))
endif
endif

include $(USER_ROOT)/make/makeinclude/rules.local.GNU


LDFLAGS      += -L. $(if $(ARCH),-L./$(ARCH)) -L$(USER_ROOT)/libs\
-L$(ACE_LIB_PATH)

CPPFLAGS     += -I$(USER_ROOT)/include\
-I$(ACE_INCLUDE)

LDFLAGS		 += $(MYLDFLAGS)
CPPFLAGS     += -DHAVE_LINUX_TYPES_H -DHAVE_CONFIG_H $(MYCPPFLAGS)

ifeq (1,$(debug))
	CPPFLAGS += -DDEBUG_VERSION
	CPPFLAGS += -DCAW_DEBUG
else
	CPPFLAGS += -DRELEASE_VERSION
	CPPFLAGS += -DCAW_DISABLE_TRACE
endif

ifeq ($(static_libs),1)
  CPPFLAGS += -DUSER_AS_STATIC_LIBS
  ifeq ($(link_groups), 1)
    LDLIBS := -Wl,--start-group $(LDLIBS) -Wl,--end-group
  endif
endif

#----------------------------------------------------------------------------
#       Local targets
#----------------------------------------------------------------------------
lib_warning:
	@echo WFNOS will not be built due to the following missing library: $(LIBCHECK) >&2

## Some OS's have /bin/test others only have /usr/bin/test
ifeq ($(wildcard /bin/test), /bin/test)
  TEST_EXE = /bin/test
else
ifeq ($(wildcard /usr/bin/test), /usr/bin/test)
  TEST_EXE = /usr/bin/test
endif
endif

SPACE = $(should_be_unset) $(should_be_unset)
ifneq ($(GENERATED_DIRTY),)
.PRECIOUS: $(GENERATED_DIRTY)
## If the generated files are anything but source files, we need to
## ensure that those files are generated before we attempt to build anything
## else.
ifeq ($(OBJS_DEPEND_ON_GENERATED),1)
$(VDIR)$(ACE_PCH_FILE) $(addprefix $(VDIR), $(OBJS)): $(GENERATED_DIRTY)
$(VSHDIR)$(ACE_PCH_FILE) $(VSHOBJS): $(GENERATED_DIRTY)
endif
endif


ifneq ($(VXWORKSLINK),1)
ifeq ($(static_libs_only), 1)
  ifeq ($(use_dep_libs), 1)
    DEPLIBS = $(foreach lib, ACE , $(foreach libpath, . $(USER_ROOT)/libs, $(wildcard $(libpath)/$(LIB_PREFIX)$(lib).$(LIBEXT))))
  endif
endif

VLDLIBS +=-lm -lrt -lpthread -lutil -ldl -lcrypt \
	-lACE \
	$(MYDLLLIB)
$(VBIN): $(addprefix $(VDIR), $(OBJS)) $(DEPLIBS)
  ifneq (,$(LINK.cc.override))
	$(LINK.cc.override)
  else
	$(LINK.cc) $(LDFLAGS) $(CC_OUTPUT_FLAG) $@ $(filter-out %.a,$^) $(VLDLIBS) $(POSTLINK)
  endif
endif


realclean: clean
ifneq ($(GENERATED_DIRTY),)
	-$(RM) -r $(GENERATED_DIRTY)
endif

__prebuild__:
	@-:


.PHONY: install
install: all
INSTALL_LIB ?= lib
ifneq ($(INSTALL_PREFIX),)
ifneq ($(install_rpath),0)
LDFLAGS += -Wl,-R$(INSTALL_PREFIX)/$(INSTALL_LIB) $(LD_RPATH_FLAGS)
endif
endif
