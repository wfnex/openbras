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
