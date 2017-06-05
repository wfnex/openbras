/*
 * Copyright (c) 2014  Nanjing WFN Technology Co., Ltd.  All rights reserved.
*/

#ifndef RANDOM_H
#define RANDOM_H

#include "pppdef.h"


void magic_init();
uint32_t magic();
void random_bytes(unsigned char *buf, int len);
double drand48();

long mrand48();
void srand48(long seedval);


#endif//TIMEPORT_H

