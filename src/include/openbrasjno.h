/***********************************************************************
 Copyright (C) 2016-2017 Nanjing StarOS Technology Co., Ltd 
**********************************************************************/

#ifndef OPENBRASJNO_H
#define OPENBRASJNO_H
#include "corejno.h"
#define OPENBRAS_DIPC_JNO_START                (uint16_t)(CORE_DIPC_JNO_END+1001)
#define OPENBRAS_JNO_STARFACESERVER            (uint16_t)(OPENBRAS_DIPC_JNO_START+1)
#define OPENBRAS_JNO_STAROPENFLOW            (uint16_t)(OPENBRAS_DIPC_JNO_START+2)
#define OPENBRAS_DIPC_JNO_END                  (uint16_t)(OPENBRAS_DIPC_JNO_START+1000)
#endif//DIPCJNO_H

