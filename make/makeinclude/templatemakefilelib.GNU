# -*- Makefile -*-

MAKEFILE         = GNUmakefile.$(MYLIB)
DEPENDENCY_FILE  = .depend.$(MYLIB)

LIB_PREFIX      ?= lib

## LIB_CHECKED may be set to empty later on in this file
LIB_UNCHECKED    = $(LIB_PREFIX)$(MYLIB).$(LIBEXT)
LIB_CHECKED      = $(LIB_UNCHECKED)
LIB_NAME         = $(LIB_PREFIX)$(MYLIB)

## SHLIB may be set to empty later on in this file
SHLIB_UNCHECKED  = $(LIB_PREFIX)$(MYLIB).$(SOEXT)
SHLIB            = $(SHLIB_UNCHECKED)

FILES = $(MYFILES)

#----------------------------------------------------------------------------
#       Include macros and targets
#----------------------------------------------------------------------------
ACE_SHLIBS = -lACE

PRJ_TYPE = library

COLLAPSE_SLASHES = $(if $(findstring //,$(1)),$(call COLLAPSE_SLASHES,$(subst //,/,$(1))),$(1))
INSLIB ?= $(call COLLAPSE_SLASHES,$(USER_ROOT)/libs)
ifeq ($(INSLIB),.)
  ifeq ($(CURDIR),)
    ifeq ($(PWD),)
      PWD=$(shell pwd)
    endif
  else
    PWD=$(CURDIR)
  endif
  INSLIB = $(PWD)
endif
OUTPUT_DIRECTORY = $(INSLIB)

include $(USER_ROOT)/make/makeinclude/wrapper_macros.GNU


GNUACE_PROJECT_VERSION ?= $(USER_VERSION)

ifdef TEMPINCDIR
TEMPINCDIR := $(TEMPINCDIR)/$(MYLIB)
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
    LIBCHECK ?= $(filter-out $(foreach lib,ACE,$(findstring $(lib),$(foreach libpath,. $(if $(ARCH),./$(ARCH)) ../../lib $(if $(ARCH),../../lib/$(ARCH)) $(USER_ROOT)/libs /usr/lib /usr/lib64 $(INSLIB),$(wildcard $(libpath)/$(LIB_PREFIX)$(lib).* $(libpath)/$(lib).lib)))),ACE)
    ifeq ($(LIBCHECK),)
      LIBCHECK = 1
    endif
  endif
endif

LSRC   = $(FILES)

include $(USER_ROOT)/make/makeinclude/macros.GNU
include $(USER_ROOT)/make/makeinclude/rules.common.GNU
include $(USER_ROOT)/make/makeinclude/rules.nonested.GNU
include $(USER_ROOT)/make/makeinclude/rules.lib.GNU
include $(USER_ROOT)/make/makeinclude/rules.local.GNU

LDFLAGS      += -L. $(if $(ARCH),-L./$(ARCH)) -L$(USER_ROOT)/libs\
-L$(ACE_LIB_PATH)


CPPFLAGS     += -I$(USER_ROOT)/include\
-I$(ACE_INCLUDE)

ifeq ($(shared_libs),1)
  ifneq ($(SHLIB),)
    CPPFLAGS += -DUSER_BUILD_DLL -DCAW_OS_BUILD_DLL
  endif
endif
ifeq ($(static_libs),1)
  CPPFLAGS += -DUSER_AS_STATIC_LIBS
endif

CPPFLAGS     += -DHAVE_LINUX_TYPES_H -DHAVE_CONFIG_H $(MYCPPFLAGS)
LDFLAGS		 += $(MYLDFLAGS)
ifeq (1,$(debug))
	CPPFLAGS += -DDEBUG_VERSION
	CPPFLAGS += -DCAW_DEBUG
else
	CPPFLAGS += -DRELEASE_VERSION
	CPPFLAGS += -DCAW_DISABLE_TRACE
endif
#----------------------------------------------------------------------------
#       Local targets
#----------------------------------------------------------------------------
lib_warning:
	@echo will not be built due to the following missing library: $(LIBCHECK) >&2
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



realclean: clean
ifneq ($(GENERATED_DIRTY),)
	-$(RM) -r $(GENERATED_DIRTY)
endif
