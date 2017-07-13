
include $(USER_DEPTH)/make/makeinclude/wf_pre.mak

USER_LIB = l2tp

USER_SRCS_CPP = \
CL2TPLAC.cpp\
CL2TPManager.cpp\
CL2TPMessage.cpp\
CL2TPSession.cpp\
CL2TPTunnel.cpp
	
USER_INCLDIRS = -I./ \
	-I$(WFACE_INCLUDE) \
	-I$(WOCAL_INCLUDE) \
	-I$(ACE_INCLUDE) \
	-I$(USER_DEPTH)/include

USER_DEFFLAGS = -Wno-deprecated -std=gnu++0x
USER_LINK_LIBS =
USER_LINK_DLLS = 
USER_LINK_FLAGS=
USER_OBJ_DIR=$(TARGET_PATH)/src

include $(USER_DEPTH)/make/makeinclude/wf_post.mak



