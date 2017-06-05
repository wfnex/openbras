# -*- Makefile -*-

#----------------------------------------------------------------------------
#      Platform-dependent macro definitions
#      (link to the appropriate platform-specific config file).
#----------------------------------------------------------------------------

#### Default to building shared libraries only.  Set it here to give the
#### platform a chance to override.
ifndef shared_libs
  shared_libs = 1
endif # shared_libs
ifndef static_libs
  static_libs = 0
endif # static_libs

#### Set up the build version in case the platform_macros.GNU wants it.
ifeq (cmd,$(findstring cmd,$(SHELL)))
  USER_MAJOR_VERSION := $(shell awk "/USER_MAJOR_VERSION/ { print $$3}" ${USER_ROOT}/include/myversion.h)
  USER_MINOR_VERSION := $(shell awk "/USER_MINOR_VERSION/ { print $$3}" ${USER_ROOT}/include/myversion.h)
  USER_BETA_VERSION := $(shell awk "/USER_BETA_VERSION/ { print $$3}" ${USER_ROOT}/include/myversion.h)
else
  USER_MAJOR_VERSION := $(shell awk '/USER_MAJOR_VERSION/ { print $$3}' ${USER_ROOT}/include/myversion.h)
  USER_MINOR_VERSION := $(shell awk '/USER_MINOR_VERSION/ { print $$3}' ${USER_ROOT}/include/myversion.h)
  USER_BETA_VERSION := $(shell awk '/USER_BETA_VERSION/ { print $$3}' ${USER_ROOT}/include/myversion.h)
endif

USER_VERSION = $(USER_MAJOR_VERSION).$(USER_MINOR_VERSION).$(USER_BETA_VERSION)

# Define some variables to silence warnings
SHR_FILTER ?=

# Include this before the below variables to make it possible for
# platform_macros.GNU to set default values for them.
ifneq ($(alternate_platform_macros),)
  include $(alternate_platform_macros)
else
  include $(USER_ROOT)/make/makeinclude/platform_macros.GNU
endif # alternate_platform_macros

# Define some variables to silence warnings
GHS ?=
AIX_TEMPLATE_HACK ?=


####
#### Defaults are fast off (0), threads on (1)
#### and versioned_so on (1).
####
fast ?= 0
versioned_so ?= 1
use_dep_libs ?= 1
threads ?= 1
symlinks ?= relative
link_groups ?= 0
mcpp ?= 0
root_is_symlink ?= 0

SOVERSION ?=
SONAME ?=

ifeq ($(root_is_symlink),1)
  ifneq ($(symlinks),relative)
    warn_output:=$(warning WARNING: You should really use relative library symlinks with symlinked root)
  endif
  use_pwd_call = 1
  ### use 'real' pwd command from shell; internal make command resolves symlinks
  ifeq ($(PWD),)
    PWD=$(shell $(SHELL) -c pwd)
  endif
else
  ifeq ($(PWD),)
    PWD=$(shell pwd)
  endif
endif

MKDIR = mkdir -p

#----------------------------------------------------------------------------
#      Platform-independent macro definitions
#----------------------------------------------------------------------------

INCLDIRS ?=
DEFFLAGS ?=

ifeq ($(debug),0)
  DEFFLAGS        += -DNDEBUG
endif

ifeq (,$(findstring -I$(USER_ROOT),$(INCLDIRS)))
  INCLDIRS        += -I$(USER_ROOT)/include
endif

CPPFLAGS          += $(DEFFLAGS) $(INCLDIRS)

# Define default extensions for IDL-generated files
IDL_CLIENT_HDR_EXT ?= C.h
IDL_CLIENT_INL_EXT ?= C.inl
IDL_CLIENT_SRC_EXT ?= C.cpp
IDL_SERVER_HDR_EXT ?= S.h
IDL_SERVER_SRC_EXT ?= S.cpp
IDL_SERVER_THDR_EXT ?= S_T.h
IDL_SERVER_TINL_EXT ?= S_T.inl
IDL_SERVER_TSRC_EXT ?= S_T.cpp
IDL_GEN_FILES_DIR ?= .

ifeq (default,$(origin LEX))
  LEX = flex
endif

EXEEXT ?=

ifndef COMSPEC
  ifdef ComSpec
    COMSPEC = $(ComSpec)
  endif # ComSpec
endif # ! COMPSPEC

ifdef COMSPEC
  #### Assume we're on a WIN32 host.
  ACE_NUL = nul
else  # ! WIN32
  ACE_NUL = /dev/null
endif # ! WIN32

#### Redirect stderr to nul device, and mask exit status.
ACE_NUL_STDERR = 2>$(ACE_NUL) || true
PCFLAGS         += #### set in platform_macros.GNU file, e.g., -p
PCCFLAGS        += #### set in platform_macros.GNU file, e.g., -p
VAR             ?=

# User-written makefiles that are not MPC-generated often use LIB to specify
# what to build. This is in contrast to MPC-generated files that use
# LIB_CHECKED for this purpose. LIB causes conflicts on Windows with gmake
# and Visual Studio which uses the LIB environment variable as a search path
# for libraries to link. So, to allow non-MPC makefiles to keep working, set
# LIB_CHECKED based on LIB if the former is not set and the latter is.
ifndef COMSPEC
  LIB_CHECKED ?= $(LIB)
endif # ! COMPSPEC (and, therefore, assumed !Windows

# default to not using an architecture-specific output directory
ARCH ?=

ifdef ARCH
  INSLIB := $(INSLIB:%=%/$(ARCH))

  # ADD_ARCH is a function used with $(call) to "ARCHIFY" a variable which
  # contains the name and path of an executable or library.  If the file is not
  # located in the ARCH subdir, the non-ARCH location is used.  First we need
  # some helper functions to deal with quoting/protecting spaces.
  # ADD_ARCH can return a path containing a space, the caller must quote it or
  # escape the space, depending on the context.
  PROT_SPC = $(subst $(SPACE),?,$(1))
  UNPROT_SPC = $(subst ?,$(SPACE),$(1))
  FILE_EXISTS = $(wildcard $(subst $(SPACE),\$(SPACE),$(1)))
  ADD_ARCH_HLP = $(call UNPROT_SPC,$(dir $(1))$(ARCH)/$(notdir $(1)))
  ARCH_FILE = $(call FILE_EXISTS,$(call ADD_ARCH_HLP,$(call PROT_SPC,$(1))))
  ADD_ARCH = $(if $(call ARCH_FILE,$(1)),$(call ARCH_FILE,$(1)),$(call FILE_EXISTS,$(1)))

  ARCH_BIN     = $(if $(BIN),$(ARCH)/$(BIN))
  ARCH_BIN2    = $(if $(BIN2),$(ARCH)/$(BIN2))
  ARCH_LIB     = $(if $(LIB_CHECKED),$(ARCH)/$(LIB_CHECKED))
  ARCH_LIB2    = $(if $(LIB2),$(ARCH)/$(LIB2))
  ARCH_SHLIB   = $(if $(SHLIB),$(ARCH)/$(SHLIB))
  ARCH_SHLIB2  = $(if $(SHLIB2),$(ARCH)/$(SHLIB2))
  ARCH_SHLIBA  = $(if $(SHLIBA),$(ARCH)/$(SHLIBA))
  ARCH_SHLIBA2 = $(if $(SHLIBA2),$(ARCH)/$(SHLIBA2))

else #ARCH is not defined

  ADD_ARCH = $(1)

  ARCH_BIN     = $(BIN)
  ARCH_BIN2    = $(BIN2)
  ARCH_LIB     = $(LIB_CHECKED)
  ARCH_LIB2    = $(LIB2)
  ARCH_SHLIB   = $(SHLIB)
  ARCH_SHLIB2  = $(SHLIB2)
  ARCH_SHLIBA  = $(SHLIBA)
  ARCH_SHLIBA2 = $(SHLIBA2)

endif #ARCH

# for use with the result of ADD_ARCH
ADD_QUOTE = $(if $(findstring $(SPACE),$(1)),"$(1)",$(1))


INSINC      ?= $(USER_ROOT)/include
INSLIB      ?= $(USER_ROOT)/libs
INSMAN      ?= $(USER_ROOT)/man

ifeq (,$(findstring -L$(INSLIB),$(LDFLAGS)))
  LDFLAGS         += -L$(INSLIB)
endif

#ifdef ARCH
#  LDFLAGS_ARCH := $(LDFLAGS:-L%=-L%/$(ARCH))
#  LDFLAGS += $(LDFLAGS_ARCH)
#endif

BIN_UNCHECKED ?=
CLEANUP_BIN ?=
ifdef BIN
  CLEANUP_BIN = $(ARCH_BIN)
endif # !BIN
ifdef BIN2
  CLEANUP_BIN += $(ARCH_BIN2)
endif # !BIN2
ifneq ($(BIN_UNCHECKED),)
  CLEANUP_BIN += $(BIN_UNCHECKED)
endif # !BIN_UNCHECKED

LIB_UNCHECKED ?=
CLEANUP_LIB ?=
ifdef LIB_CHECKED
  CLEANUP_LIB = $(ARCH_LIB)
endif # !LIB_CHECKED
ifdef LIB2
  CLEANUP_LIB += $(ARCH_LIB2)
endif # !LIB2
ifneq ($(LIB_UNCHECKED),)
  CLEANUP_LIB = $(LIB_UNCHECKED)
endif # !LIB_UNCHECKED

SHLIB_UNCHECKED ?=
CLEANUP_SHLIB ?=
ifdef SHLIB
  CLEANUP_SHLIB = $(ARCH_SHLIB)
endif # !SHLIB
ifdef SHLIB2
  CLEANUP_SHLIB += $(ARCH_SHLIB2)
endif # SHLIB2
ifneq ($(SHLIB_UNCHECKED),)
  CLEANUP_SHLIB += $(SHLIB_UNCHECKED)
endif # ! SHLIB_UNCHECKED

SHLIBA_UNCHECKED ?=
CLEANUP_SHLIBA ?=
ifdef SHLIBA
  CLEANUP_SHLIBA = $(ARCH_SHLIBA)
endif # SHLIBA
ifdef SHLIBA2
  CLEANUP_SHLIBA = $(ARCH_SHLIBA2)
endif # SHLIB2A
ifneq ($(SHLIBA_UNCHECKED),)
  CLEANUP_SHLIBA = $(SHLIBA_UNCHECKED)
endif # SHLIBA_UNCHECKED

# Let users override the default VDIR directories
VDIR_PRJ ?=
ifdef ARCH
VDIR          ?= $(ARCH)/.obj/$(VDIR_PRJ)
VSHDIR        ?= $(ARCH)/.shobj/$(VDIR_PRJ)
else
VDIR          ?= .obj/$(VDIR_PRJ)
VSHDIR        ?= .shobj/$(VDIR_PRJ)
endif

# NOTE: VLDLIBS is defined below, in case it is overridden for static-
#       or shared-only builds.
ifeq (default,$(origin YACC))
  YACC = bison
endif

#----------------------------------------------------------------------------
#      Platform-dependent macros that require platform_macros.GNU.
#----------------------------------------------------------------------------

OBJEXT ?= o
SOEXT ?= so
LIBEXT ?= a
LIB_PREFIX ?= lib

CC_OUTPUT_FLAG ?= -o
SO_OUTPUT_FLAG ?= -o
LINK_OUTPUT_FLAG ?= -o

#----------------------------------------------------------------------------
#      make flags
#----------------------------------------------------------------------------

ifeq ($(optimize),0)
  override optimize =
endif # optimize

ifneq ($(optimize),)
  CFLAGS  += $(OCFLAGS)
  CCFLAGS += $(OCCFLAGS)
endif # optimize

ifeq ($(debug),0)
  override debug =
endif # debug

ifeq ($(debug),)
  CPPFLAGS += -DACE_NDEBUG
else
  CFLAGS  += $(DCFLAGS)
  CCFLAGS += $(DCCFLAGS)
endif # debug

dmalloc ?=
ifeq ($(dmalloc),0)
  override dmalloc =
endif # dmalloc

ifdef dmalloc
  CPPFLAGS += $(PLATFORM_DMALLOC_CPPFLAGS)
  LDFLAGS += $(PLATFORM_DMALLOC_LDFLAGS)
  LIBS += $(PLATFORM_DMALLOC_LIBS)
endif # dmalloc

mtrace ?=
ifeq ($(mtrace),0)
  override mtrace =
endif # mtrace

ifdef mtrace
  CPPFLAGS += $(PLATFORM_MTRACE_CPPFLAGS)
endif # mtrace

ifeq ($(threads),0)
  override threads =
endif # threads

trio ?=
ifeq ($(trio),0)
  override trio =
endif # trio

# ace_for_tao subsetting is disabled by default.
ace_for_tao ?=
ifeq ($(ace_for_tao),0)
  override ace_for_tao =
endif # ace_for_tao



ifeq ($(xt),0)
  override xt =
endif # xt


fl ?=
ifeq ($(fl),0)
  override fl =
endif # fl


tk ?=
ifeq ($(tk),0)
  override tk =
endif # tk


qt ?=
ifeq ($(qt),0)
  override qt =
endif # qt

ifneq ($(qt),)
  override qt=1
  override ace_qtreactor=1
  override tao_qtresource=1
  ACE_MAKE_OPTIONS += qt
endif # qt
gtk_reactor ?=


fox ?=
ifeq ($(fox),0)
  override fox =
endif # fox


QTDIR ?=
ifneq ($(QTDIR),)
  MOC = ${QTDIR}/bin/moc
else
  MOC = moc
endif

rapi ?=
ifeq ($(rapi),0)
   override rapi =
endif #rapi


sctp ?=


gl ?=
ifeq ($(gl),0)
  override gl =
endif # gl

ssl ?= 0
ifeq ($(ssl),0)
  override ssl =
else
  ifneq ($(SSL_ROOT), /usr)
    ifneq ($(SSL_ROOT),)
      PLATFORM_SSL_CPPFLAGS += -I$(SSL_ROOT)/include
      PLATFORM_SSL_LDFLAGS  += -L$(SSL_ROOT)/lib
    endif
  endif
  PLATFORM_SSL_LIBS ?= -lssl -lcrypto
endif

## These are related to MPC.  When 'requires' is used in a project
## it controls whether or not the project is generated.  There is also
## a second level of control in platform macros and thus requires
## some features to be set here as well.
ec_typed_events ?= 1
zlib ?= 1


profile ?=
ifeq ($(profile),0)
  override profile =
endif # profile

ifneq ($(profile),)
  CFLAGS  += $(PCFLAGS)
  CCFLAGS += $(PCCFLAGS)
endif # profile

inline ?= 1

ifeq ($(shared_libs),0)
  override shared_libs =
endif # shared_libs
ifeq ($(static_libs),0)
  override static_libs =
endif # static_libs

shared_libs_only ?=
ifeq ($(shared_libs_only),0)
  override shared_libs_only =
endif # shared_libs_only

static_libs_only ?=
ifeq ($(static_libs_only),0)
  override static_libs_only =
endif # static_libs_only

ifdef shared_libs_only
  ifdef static_libs_only
    $(error Both static_libs_only and shared_libs_only defined.)
  else
    override shared_libs = 1
    override static_libs =
  endif
endif # shared_libs_only
ifdef static_libs_only
  override shared_libs =
  override static_libs = 1
endif # static_libs_only

ifdef shared_libs
  ifndef static_libs
    override shared_libs_only = 1
  endif
endif

ifdef static_libs
  ifndef shared_libs
    override static_libs_only = 1
  endif
endif

ACE_SHLIBS ?=

ifdef shared_libs
  ifdef SHLIBA
    LDLIBS := $(LDLIBS:-l%=-l%shr)
    ACE_SHLIBS := $(ACE_SHLIBS:-l%=-l%shr)
  endif # SHLIBA
else # ! shared_libs
  ifdef static_libs
    #### Don't build shared libs.
    PIC             =
    SHLIB           =
    SOEXT           = o
    VSHDIR          = $(VDIR)
    VLDLIBS         = $(LDLIBS:%.so=%.o)
  endif # static_libs
endif # shared_libs

VLDLIBS = $(LDLIBS) $(LIBS)

probe ?=

purify ?=

quantify ?=

repo ?=


# If pipes is not set, we default to 0
pipes ?= 0

# If pipes is set to 1 then add the compiler flag -pipe. This uses pipe instead
# of intermediate files. This can be disabled by setting pipes to 0 in the
# platform_macros.GNU file.
ifeq ($(pipes),1)
   FLAGS_C_CC += -pipe
endif # pipes

# RMCast is turned on by default
rmcast ?= 1


# If not explicitly is set that we don't have rwho, assume we have it.
rwho ?= 1

# If not explicitly is set that we don't have stlport, assume we
# don't have it.
stlport ?= 0


# If not explicitly is set that we have wfmo, assume we don't have it.
wfmo ?= 0

# If not explicitly is set that we have winregistry, assume we don't have it.
winregistry ?= 0

# If not explicitly is set that we have winnt, assume we don't have it.
winnt ?= 0

# In MPC, these are now features that are enabled by default.
ace_other     ?= 1
ace_codecs    ?= 1
ace_token     ?= 1
ace_svcconf   ?= 1
ace_uuid      ?= 1
ace_filecache ?= 1
acexml        ?= 1

# Icecream distributed compilation support
ifeq ($(icecream),1)
  # check availability of icecc
  ifneq ($(ICECC_ROOT),)
    ICECC_PROG := $(ICECC_ROOT)/icecc
  else
    ICECC_PROG := $(shell which icecc 2>/dev/null)
  endif
  ICECC_VERTEST := $(shell test -x "$(ICECC_PROG)" && $(ICECC_PROG) --version)
  ifneq (ICECC,$(findstring ICECC,$(ICECC_VERTEST)))
    # not good, try alternative install location
    ICECC_PROG := /opt/icecream/bin/icecc
    ICECC_VERTEST := $(shell test -x "$(ICECC_PROG)" && $(ICECC_PROG) --version)
    ifneq (ICECC,$(findstring ICECC,$(ICECC_VERTEST)))
      override icecream := 0
      $(error Cannot locate valid Icecream compiler.)
    endif
  endif
  ifeq ($(icecream),1)
    # force LD and DLD to be 'normal' tools
    override LD := $(LD)
    override DLD := $(DLD)
    # define tools that icecc should call for local jobs
    ICECC_CC := $(CC)
    ICECC_CXX := $(CXX)
    export ICECC_CC ICECC_CXX
    # let all compilation go through icecc
    CC := $(ICECC_PROG)
    CXX := $(ICECC_PROG)
  endif
endif

#----------------------------------------------------------------------------
#      Conditional macro definitions
#----------------------------------------------------------------------------

PTDIRS ?=
PRELINK ?=
POSTLINK ?=
PURELINK ?=
MVCMD ?=
PREPROCESS_OPT ?= -E
PREPROCESS_SUFFIX ?= i

COMPILE.c       = $(CC) $(CFLAGS) $(FLAGS_C_CC) $(CPPFLAGS) -c
COMPILE.cc      = $(CXX) $(CCFLAGS) $(FLAGS_C_CC) $(CPPFLAGS) $(PTDIRS) -c
PREPROCESS.c   ?= $(CC) $(PREPROCESS_OPT) $(CFLAGS) $(CPPFLAGS)
PREPROCESS.cc  ?= $(CXX) $(PREPROCESS_OPT) $(CCFLAGS) $(CPPFLAGS)
RC ?=

COMPILE.rc      = $(RC) $(RCFLAGS)
COMPILE-NO_DASH_G.cc = `echo $(COMPILE.cc) | sed 's/-g //'`
# 960905 Marius Kjeldahl <marius@funcom.com>
# Added the line below to be used for compiling executable shared objects
COMPILEESO.cc   = $(CXX) $(CCFLAGS) $(FLAGS_C_CC) $(CPPFLAGS) $(PTDIRS)

ifeq ($(LINK.c),override)
  LINK.c  = $(LINK.c.override)
else
  LINK.c  = $(PURELINK) $(PRELINK) $(LD) $(CFLAGS) $(FLAGS_C_CC) $(CPPFLAGS) $(LDFLAGS) \
            $(LDLIBS) $(LIBS)
endif # LINK.c.override

ifeq ($(LINK.cc),override)
  LINK.cc = $(LINK.cc.override)
else
  LINK.cc = $(PURELINK) $(PRELINK) $(LD) $(CCFLAGS) $(FLAGS_C_CC) $(CPPFLAGS) $(PTDIRS)
endif # LINK.cc.override

ifeq ($(SOLINK.cc),override)
  SOLINK.cc = $(SOLINK.cc.override)
else
  SOLINK.cc = $(PRELINK) $(DLD) $(SOFLAGS)
endif # SOLINK.cc.override
