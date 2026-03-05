#ifndef _BUF_VIEW_H_
#define _BUF_VIEW_H_

void writeInt16ToBuff(short i, int index, unsigned char *buff);
void writeInt32ToBuff(int i, int index, unsigned char *buff);

short readInt16FromBuff(const int index, const unsigned char *buff);
int readInt32FromBuff(const int index, const unsigned char *buff);

#endif
