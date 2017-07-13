#you can use static_libs=1/0 ,to build static lib or dynamiclib
include $(USER_ROOT)/make/makeinclude/macros.GNU
MYLIB=Portal
MYFILES = \
CRadiusAttribute.cpp\
CRadiusConfig.cpp\
CRadiusConnector.cpp\
CRadiusManager.cpp\
CRadiusMessage.cpp\
CRadiusScheme.cpp\
CRadiusTransaction.cpp\
radius.cpp

MYCPPFLAGS=-I../include -I../libconfig/src -I../Utils -std=gnu++11 -I../Portal -I../VPDN
MYLDFLAGS= 
MYDLLLIB= 
include $(USER_ROOT)/make/makeinclude/allinone.GNU

