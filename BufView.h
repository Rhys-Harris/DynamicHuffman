#ifndef _BUF_VIEW_H_
#define _BUF_VIEW_H_

#include "./def.h"

void writeInt16ToBuff(uint16 i, int index, byte *buff);
void writeInt32ToBuff(uint32 i, int index, byte *buff);

uint16 readInt16FromBuff(const int index, const byte *buff);
uint32 readInt32FromBuff(const int index, const byte *buff);

#endif
