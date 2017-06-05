#build_ace=1

export CURRENTPWD:=$(shell pwd)
export PRJDIR=$(CURRENTPWD)
export USER_ROOT=$(CURRENTPWD)/../
export PROJECT_NAME=openbras
build_starfaceserver=1

#######################################################################################
NULL_STDERR = 2>$(/dev/null) || true
MAJOR_VERSION := $(shell awk '/MAJOR_VERSION/ { print $$3}' ${ROOTDIR}/version.h)
MINOR_VERSION := $(shell awk '/MINOR_VERSION/ { print $$3}' ${ROOTDIR}/version.h)
BETA_VERSION := $(shell awk '/BETA_VERSION/ { print $$3}' ${ROOTDIR}/version.h)
VERSION = $(MAJOR_VERSION).$(MINOR_VERSION).$(BETA_VERSION)

#######################################################################################
ifeq (1,$(build_openbras))
buidlall +=openbras
endif

ifeq (1,$(debug))
TARNAME=$(PROJECT_NAME)_debug-$(VERSION)
else
TARNAME=$(PROJECT_NAME)_release-$(VERSION)
endif

all: check_dir $(buidlall)
	cp -Lf $(USER_ROOT)/bin/* $(PRJDIR)/$(PROJECT_NAME)/bin/
	cp -Lf $(USER_ROOT)/libs/* $(PRJDIR)/$(PROJECT_NAME)/libs/
	cp -rf $(USER_ROOT)/scripts/* $(PRJDIR)/$(PROJECT_NAME)/scripts/
	cp -rf $(USER_ROOT)/conf/* $(PRJDIR)/$(PROJECT_NAME)/conf/
	cp -rf $(USER_ROOT)/LICENSE $(PRJDIR)/$(PROJECT_NAME)/
	cp -rf $(USER_ROOT)/README $(PRJDIR)/$(PROJECT_NAME)/
	cp -rf $(USER_ROOT)/AUTHORS $(PRJDIR)/$(PROJECT_NAME)/
	cp -rf $(USER_ROOT)/resources $(PRJDIR)/$(PROJECT_NAME)/
	cp -rf $(USER_ROOT)/docs/* $(PRJDIR)/$(PROJECT_NAME)/docs
	cp -rf $(USER_ROOT)/include/myversion.h $(PRJDIR)/$(PROJECT_NAME)/include/
	echo "Packing " $(TARNAME) "..."
	tar jcvf $(TARNAME).tar.bz2 -C $(PRJDIR) $(PROJECT_NAME)    
	@echo "build " $(PROJECT_NAME) " done!"
	@echo "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!SUCCESS!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!"

.PHONY:openbras
openbras:check_dir
	static=0
	chmod a+x $(USER_ROOT)/make/add_rel_link.sh
	make -C $(USER_ROOT) -f WFNOSMakefile

check_dir: 
	@test -d $(PRJDIR)/$(PROJECT_NAME) || mkdir -p $(PRJDIR)/$(PROJECT_NAME)
	@test -d $(PRJDIR)/$(PROJECT_NAME)/bin || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/bin
	@test -d $(PRJDIR)/$(PROJECT_NAME)/libs || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/libs
	@test -d $(PRJDIR)/$(PROJECT_NAME)/conf || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/conf
	@test -d $(PRJDIR)/$(PROJECT_NAME)/include || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/include
	@test -d $(PRJDIR)/$(PROJECT_NAME)/src || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/src
	@test -d $(PRJDIR)/$(PROJECT_NAME)/scripts || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/scripts
	@test -d $(PRJDIR)/$(PROJECT_NAME)/logs || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/logs
	@test -d $(PRJDIR)/$(PROJECT_NAME)/system || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/system
	@test -d $(PRJDIR)/$(PROJECT_NAME)/tools || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/tools
	@test -d $(PRJDIR)/$(PROJECT_NAME)/changelogs || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/changelogs
	@test -d $(PRJDIR)/$(PROJECT_NAME)/docs || mkdir -p $(PRJDIR)/$(PROJECT_NAME)/docs
	
#################################################################################
clean:
	make -C $(USER_ROOT) -f WFNOSMakefile realclean
	rm -rf $(TARNAME).tar.bz2
	rm -rf $(PRJDIR)/$(PROJECT_NAME)
	