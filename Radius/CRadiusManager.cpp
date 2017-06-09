#include "CRadiusManager.h"
#include "CRadiusScheme.h"

CRadiusManager::CRadiusManager()
{
}

CRadiusManager::~CRadiusManager()
{
}

int CRadiusManager::Init()
{
    if (m_config.Init() == -1)
    {
        return -1;
    }
    CRadiusScheme *radius = NULL;

    if (CreateRadiusScheme("test",radius) == -1)
    {
        return -1;
    }

    if (radius ==NULL)
    {
        return -1;
    }
    
    radius->SetUpPrimaryAuthConn(m_config.GetAuthServerAdd());
    radius->SetUpPrimaryAcctConn(m_config.GetAcctServerAddr());
    radius->SetUpSecondaryAuthConn(m_config.GetAuthServerAdd());
    radius->SetUpSecondaryAcctConn(m_config.GetAcctServerAddr());


    return -1;
}

int CRadiusManager::TestRadiusAccess()
{
    // Client Data
    CRadiusData User_Name(m_config.User_Name().c_str(),m_config.User_Name().size() );
    CRadiusData User_Password(m_config.User_Password().c_str(),m_config.User_Password().size());
    CRadiusData Called_Station_Id(m_config.Called_Station_Id().c_str(),m_config.Called_Station_Id().size());
    CRadiusData Calling_Station_Id(m_config.Calling_Station_Id().c_str(),m_config.Calling_Station_Id().size() );

    CRadiusAttribute attrUserName(RA_USER_NAME, User_Name );

    CRadiusAttribute attrUserPassword(RA_USER_PASSWORD, User_Password );

    ACE_INET_Addr clientaddr=m_config.GetLocalAddr();

    uint32_t ip(htonl(clientaddr.get_ip_address()));
    CRadiusData valNasIpAddress(&ip ,sizeof(ip));
    CRadiusAttribute attrNasIpAddress(RA_NAS_IP_ADDRESS,valNasIpAddress);

    uint32_t port(htons( clientaddr.get_port_number()));
    CRadiusData valNasPort( &port , sizeof(port) );
    CRadiusAttribute attrNasPort(RA_NAS_PORT, valNasPort );

    u_int32_t serviceType(htonl(RAST_LOGIN) );
    CRadiusData valServiceType( &serviceType , sizeof(serviceType) );
    CRadiusAttribute attrServiceType(RA_SERVICE_TYPE, valServiceType );

    CRadiusAttribute attrCallee(RA_CALLED_STATION_ID, Called_Station_Id );

    CRadiusAttribute attrCaller(RA_CALLING_STATION_ID, Calling_Station_Id );

    CRadiusData valAcctSessionId( "f81d4fae-7dec-11d0-a765-00a0c91e6bf6@wfnex.com", 48);
    CRadiusAttribute attrAcctSessionId( RA_ACCT_SESSION_ID,
                                       valAcctSessionId );

    u_int32_t portType( htonl(RANPT_VIRTUAL) );
    CRadiusData valNasPortType( &portType , sizeof(portType) );
    CRadiusAttribute attrNasPortType( RA_NAS_PORT_TYPE, valNasPortType );
    CRadiusAttribute attrClass( RA_CLASS, CRadiusData( "Nas Ip:1.4.61.70", 16));

    CRadiusMessage accessReqMsg(RP_ACCESS_REQUEST);

    // Warning: assert is used for testing only
    ACE_ASSERT( accessReqMsg.add( attrUserName ) );
    ACE_ASSERT( accessReqMsg.add( attrUserPassword ) );
    ACE_ASSERT( accessReqMsg.add( attrNasIpAddress ) );
    // assert( accessReqMsg.add( attrNasPort ) );
    ACE_ASSERT( accessReqMsg.add( attrServiceType ) );
    // assert( accessReqMsg.add( attrCallee ) );
    // assert( accessReqMsg.add( attrCaller ) );
    ACE_ASSERT( accessReqMsg.add( attrNasPortType ) );

    CRadiusScheme *pscheme = NULL;

    FindRadiusScheme("test",pscheme);
    if (pscheme == NULL)
    {
        return -1;
    }

    TransactionResponse callback = [this](CRadiusMessage *pRaMsg, int type)
    {     
        ACE_DEBUG((LM_INFO, "(%P|%t) TestRadiusAccess,Response\n"));
    };
    ACE_DEBUG((LM_INFO, "(%P|%t) CRadiusManager::TestRadiusAccess\n"));

    pscheme->SendAccessMessageP(accessReqMsg, callback);
}

int CRadiusManager::TestRadiusAcct()
{
    // Client Data
    CRadiusData User_Name(m_config.User_Name().c_str(),m_config.User_Name().size() );
    CRadiusData User_Password(m_config.User_Password().c_str(),m_config.User_Password().size());
    CRadiusData Called_Station_Id(m_config.Called_Station_Id().c_str(),m_config.Called_Station_Id().size());
    CRadiusData Calling_Station_Id(m_config.Calling_Station_Id().c_str(),m_config.Calling_Station_Id().size() );

    CRadiusAttribute attrUserName(RA_USER_NAME, User_Name );

    CRadiusAttribute attrUserPassword(RA_USER_PASSWORD, User_Password );

    ACE_INET_Addr clientaddr=m_config.GetLocalAddr();

    uint32_t ip(htonl(clientaddr.get_ip_address()));
    CRadiusData valNasIpAddress(&ip ,sizeof(ip));
    CRadiusAttribute attrNasIpAddress(RA_NAS_IP_ADDRESS,valNasIpAddress);

    uint32_t port(htons( clientaddr.get_port_number()));
    CRadiusData valNasPort( &port , sizeof(port) );
    CRadiusAttribute attrNasPort(RA_NAS_PORT, valNasPort );

    u_int32_t serviceType(htonl(RAST_LOGIN) );
    CRadiusData valServiceType( &serviceType , sizeof(serviceType) );
    CRadiusAttribute attrServiceType(RA_SERVICE_TYPE, valServiceType );

    CRadiusAttribute attrCallee(RA_CALLED_STATION_ID, Called_Station_Id );

    CRadiusAttribute attrCaller(RA_CALLING_STATION_ID, Calling_Station_Id );

    CRadiusData valAcctSessionId( "f81d4fae-7dec-11d0-a765-00a0c91e6bf6@wfnex.com", 48);
    CRadiusAttribute attrAcctSessionId( RA_ACCT_SESSION_ID,
                                       valAcctSessionId );

    u_int32_t portType( htonl(RANPT_VIRTUAL) );
    CRadiusData valNasPortType( &portType , sizeof(portType) );
    CRadiusAttribute attrNasPortType( RA_NAS_PORT_TYPE, valNasPortType );
    CRadiusAttribute attrClass( RA_CLASS, CRadiusData( "Nas Ip:1.4.61.70", 16));

    CRadiusMessage acctReqMsg( RP_ACCOUNTING_REQUEST );
    
    // Warning: assert is used for testing only
    assert( acctReqMsg.add( attrUserName ) );
    assert( acctReqMsg.add( attrNasIpAddress ) );
    // assert( acctReqMsg.add( attrNasPort ) );
    assert( acctReqMsg.add( attrClass ) );
    assert( acctReqMsg.add( attrCallee ) );
    assert( acctReqMsg.add( attrCaller ) );
    
    u_int32_t acctType( htonl(RAS_START) );
    CRadiusData valAcctStatus( &acctType, sizeof(acctType) );
    CRadiusAttribute attrAcctStatus( RA_ACCT_STATUS_TYPE,
                                    valAcctStatus );
    assert( acctReqMsg.add( attrAcctStatus ) );
    
    assert( acctReqMsg.add( attrAcctSessionId ) );
    assert( acctReqMsg.add( attrNasPortType ) );


    CRadiusScheme *pscheme = NULL;

    FindRadiusScheme("test",pscheme);
    if (pscheme == NULL)
    {
        return -1;
    }

    
    TransactionResponse callback = [this](CRadiusMessage *pRaMsg, int type)
    {     
        ACE_DEBUG((LM_INFO, "(%P|%t) TestRadiusAcct,Response\n"));
    };
    ACE_DEBUG((LM_INFO, "(%P|%t) CRadiusManager::TestRadiusAcct\n"));

    pscheme->SendAcctMessageP(acctReqMsg, callback);
}


int CRadiusManager::CreateRadiusScheme(std::string &name,CRadiusScheme *&radius)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
    std::unordered_map<std::string, CRadiusScheme *>::iterator it = m_schemes.find(name);
    if (it != m_schemes.end())
    {
        return -1£»
    }
    CRadiusScheme *scheme = new CRadiusScheme(*this, name);
    if (scheme == NULL)
    {
        return -1;
    }
    radius = scheme;
    m_schemes[name]=scheme;
    return 0;
}

int CRadiusManager::DestroyRadiusScheme(std::string &name)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
    std::unordered_map<std::string, CRadiusScheme *>::iterator it = m_schemes.find(name);
    if (it == m_schemes.end())
    {
        return -1£»
    }
    delete (it->second);
    m_schemes.erase(it);
    return 0;

}

int CRadiusManager::FindRadiusScheme(std::string &name,CRadiusScheme *&radius)
{
    ACE_GUARD_RETURN (ACE_Thread_Mutex, g, m_mutex, -1);
    std::unordered_map<std::string, CRadiusScheme *>::iterator it = m_schemes.find(name);
    if (it == m_schemes.end())
    {
        return -1£»
    }
    radius = it->second;
    return 0;
}



