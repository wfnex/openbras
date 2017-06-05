
all: ace acessl makeinstall

.PHONY: ace
ace:
	@echo 'ACE_ROOT=' $(ACE_ROOT)
	@echo 'LD_LIBRARY_PATH=' $(LD_LIBRARY_PATH)
	@chmod a+x $(ACE_ROOT)/bin/add_rel_link.sh
	@chmod a+x $(ACE_ROOT)/bin/ace_install_pkgconfig.pl
	@chmod a+x $(ACE_ROOT)/MPC/prj_install.pl
	$(MAKE) -C ./ACE_wrappers -f GNUmakefile ACE static_libs=0
.PHONY: acessl
acessl:
	@chmod a+x $(ACE_ROOT)/bin/add_rel_link.sh
	$(MAKE) -C ./ACE_wrappers/ace -f GNUmakefile SSL-target static_libs=0
.PHONY: makeinstall
makeinstall:
	@cd ./ACE_wrappers/ace && make -f GNUmakefile install
clean:aceclean
aceclean:
	$(MAKE) -C ./ACE_wrappers/ace realclean
	rm -rf ./install/*
