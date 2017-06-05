#ifndef CPPPOESESSION_HANDLER_H
#define CPPPOESESSION_HANDLER_H

#include "CPPPOEPacketHandler.h"

class CPPPOE;

class CPPPOESessionHandler : public CPPPOEPacketHandler
{
public:
    CPPPOESessionHandler(CPPPOE &pppoe);
    virtual ~CPPPOESessionHandler();
    
    int Init();

    // For class CPPPOEPacketHandler
    virtual SWORD32 ProcessPacket(const CHAR *packet, ssize_t size);

    CPPPOE &GetPppoe();
    
private:
    CPPPOE &m_pppoe;
};

#endif //  CPPPOESESSION_HANDLER_H