# -*- Makefile -*-

# If the client makefile is not called "Makefile", the MAKEFILE
# variable must be set to its actual name before including this
# file to allow the recursive MAKE to work properly.

MAKEFILE ?= WFNOSMakefile
SUBDIR_MAKEFILE ?= $(MAKEFILE)

# Make sure that we build directories with MYDIRS= in sequence instead of in
# parallel, unless DIRS_PARALLEL is set, in which case they may be built
# in parallel if enabled by the proper flags and not disabled elsewhere.
DIRS_PARALLEL ?=

ifeq ($(DIRS_PARALLEL),)
.NOTPARALLEL:
endif

# A rule to build DIR.subdir from DIR by calling MAKE again, with
# DIR as the new directory, SUBDIR_TARGET as the target to build, and
# "GNUmakefile" as the makefile to use in the new directory.

# Triggered by the $(TARGETS_NESTED) target, below.

%.subdir: %
	cd $< && $(MAKE) -f $(SUBDIR_MAKEFILE) $(SUBDIR_TARGET)

# Build FOO.nested by calling MAKE again, this time with MYDIRS (with
# ".subdir" appended to each) as the targets, and passing FOO as the
# real target to build in the SUBDIR_TARGET variable.

all.local:

realclean.local:
clean.local:

$(TARGETS_NESTED):
ifdef MYDIRS
  ifneq ($(MYDIRS),)
	$(MAKE) -f $(MAKEFILE) SUBDIR_TARGET=$(@:.nested=) $(addsuffix .subdir, $(MYDIRS))
  endif # MYDIRS
endif # MYDIRS
