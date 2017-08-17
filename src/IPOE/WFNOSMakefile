#you can use static_libs=1/0 ,to build static lib or dynamiclib
include $(USER_ROOT)/make/makeinclude/macros.GNU
MYLIB=IPOE
MYFILES = CDHCPServer.cpp\
CDHCPServerBCR.cpp\
CDHCPSession.cpp\
CIPOEModule.cpp\
DHCPPacket.cpp\
CDHCPAuthRespEventHndl.cpp\
CDHCPKickUserEventHndl.cpp\
CAuthMgerTest.cpp\
CSessionManagerTest.cpp

MYCPPFLAGS=-I../include -I../Utils -std=gnu++11 
MYLDFLAGS= 
MYDLLLIB=
include $(USER_ROOT)/make/makeinclude/allinone.GNU

