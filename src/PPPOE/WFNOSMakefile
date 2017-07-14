#you can use static_libs=1/0 ,to build static lib or dynamiclib
include $(USER_ROOT)/make/makeinclude/macros.GNU
MYLIB=PPPOE
MYFILES = \
CClientDiscovery.cpp\
CEtherIntf.cpp\
CPPPOEPacketHandler.cpp\
CPPPOEDiscoveryHandler.cpp\
CPPPOESessionHandler.cpp\
CPPPOEEventHandler.cpp\
CPPPOE.cpp\
CSession.cpp\
CAddSessionId.cpp

MYCPPFLAGS= -I../include -I../PPP -std=gnu++11
MYLDFLAGS= 
MYDLLLIB= 
include $(USER_ROOT)/make/makeinclude/allinone.GNU

