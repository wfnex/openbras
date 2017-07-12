#you can use static_libs=1/0 ,to build static lib or dynamiclib
include $(USER_ROOT)/make/makeinclude/macros.GNU
MYLIB=utils
MYFILES = \
md4.cpp\
md5.cpp\
sha1.cpp\
iniparser.cpp\
dictionary.cpp

MYCPPFLAGS= -I../include -std=gnu++11
MYLDFLAGS= 
MYDLLLIB= 
include $(USER_ROOT)/make/makeinclude/allinone.GNU

