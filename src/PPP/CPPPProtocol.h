
/*
 * Copyright (c) 2017 The OpenBras project authors. All Rights Reserved.
 */
 
#ifndef CPPPPROTOCOL_H
#define CPPPPROTOCOL_H

#include "pppdef.h"

class VPN_PUBLIC CPPPProtocol
{
public:
    CPPPProtocol();
    virtual ~CPPPProtocol();
    
    virtual void Init() = 0;                                     /* Initialization procedure */
    virtual void Input(unsigned char *packet ,size_t size) = 0;  /* Process a received packet */
    virtual void Protrej() = 0;                                  /* Process a received protocol-reject */
    virtual void LowerUp() = 0;                                  /* Lower layer has come up */
    virtual void LowerDown() = 0;                                /* Lower layer has gone down */
    virtual void Open() = 0;                                     /* Open the protocol */
    virtual void Close(char *reason) = 0;                        /* Close the protocol */
};

#endif//CPPPPROTOCOL_H

