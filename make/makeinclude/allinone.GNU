# -*- Makefile -*-

ifdef MYDIRS
	include $(USER_ROOT)/make/makeinclude/wrapper_macros.GNU
	include $(USER_ROOT)/make/makeinclude/macros.GNU
	include $(USER_ROOT)/make/makeinclude/rules.common.GNU
	include $(USER_ROOT)/make/makeinclude/rules.nested.GNU
else
	ifdef MYLIB
		include $(USER_ROOT)/make/makeinclude/templatemakefilelib.GNU
	else
		ifdef MYBIN
			include $(USER_ROOT)/make/makeinclude/templatemakefilebin.GNU
		endif
	endif
endif
