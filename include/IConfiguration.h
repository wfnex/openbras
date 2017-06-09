#ifndef ICONFIGURATION_H
#define ICONFIGURATION_H

using namespace std;

class IConfiguration
{
public:
    virtual ~IConfiguration(){}
    static  IConfiguration* Instance();

    // [General]
    virtual void   GetRadiusServerIP(std::string &aRadiusSeverIp)                   = 0;
};

#endif

