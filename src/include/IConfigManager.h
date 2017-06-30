/***********************************************************************
	Copyright (c) 2017, The OpenBRAS project authors. All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions are
	met:

	  * Redistributions of source code must retain the above copyright
		notice, this list of conditions and the following disclaimer.

	  * Redistributions in binary form must reproduce the above copyright
		notice, this list of conditions and the following disclaimer in
		the documentation and/or other materials provided with the
		distribution.

	  * Neither the name of OpenBRAS nor the names of its contributors may
		be used to endorse or promote products derived from this software
		without specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
	A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
	HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
	SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
	LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
	DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
	THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
	(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
	OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**********************************************************************/
// Interface for configuration manager

#ifndef __ICONFIGMANAGER_H__
#define __ICONFIGMANAGER_H__

#define CONFIGMGR_PPPOE_MAX_ACNAME_LENGTH (200)
#define CONFIGMGR_PPPOE_MAX_SERVICE_NAME_LENGTH (200)
#define CONFIGMGR_ETHER_INTF_NAME_SIZE (16)

#include <string>

// 回调
class IConfigManagerSink
{
public:    
    virtual ~IConfigManagerSink() {}

    // PPPoE related configurations
    virtual void SetACName(const char acName[CONFIGMGR_PPPOE_MAX_ACNAME_LENGTH]) = 0;
    virtual void SetSvcName(const char svcName[CONFIGMGR_PPPOE_MAX_SERVICE_NAME_LENGTH]) = 0;
    
    // VBUI related configurations
    virtual void SetVBUIIntfName(const char acIntfName[CONFIGMGR_ETHER_INTF_NAME_SIZE+1]) = 0;
    virtual void SetVBUIIntfMtu(unsigned short mtu) = 0;
    virtual void SetVBUIIntfIP(unsigned int ipAddr) = 0;  // ip为网络序
    virtual void SetVBUIIntfIP(std::string &ipAddr) = 0;  // 如std::string("10.1.1.1")

    // Authetication related configurations
    virtual void SetAuthType(uint16_t authType) = 0;  // The value of authType is the enumeraton of AIM_AUTH_TYPE in aim_ex.h
    virtual void SetHostName(std::string &hostName) = 0;
};

class IConfigManager
{
public:
    static IConfigManager & instance();
    virtual ~IConfigManager() {};
    
    virtual int openWithSink(IConfigManagerSink *pSink) = 0; // 注册IConfigManagerSink回调
    virtual int Close() = 0;                                 // 注销IConfigManagerSink回调
};

#endif // __ICONFIGMANAGER_H__
