#include "./BufView.h"

void writeInt16ToBuff(uint16 i, int index, byte *buff) {
	buff[index] = (byte)(i>>8);
	buff[index+1] = (byte)(i);
}

void writeInt32ToBuff(uint32 i, int index, byte *buff) {
	buff[index] = (byte)(i>>24);
	buff[index+1] = (byte)(i>>16);
	buff[index+2] = (byte)(i>>8);
	buff[index+3] = (byte)(i);
}

uint16 readInt16FromBuff(const int index, const byte *buff) {
	uint16 i = 0;

	i |= ((uint16)(buff[index])<<8);
	i |= ((uint16)buff[index+1]);

	return i;
}

uint32 readInt32FromBuff(const int index, const byte *buff) {
	uint32 i = 0;

	i |= ((uint32)(buff[index])<<24);
	i |= ((uint32)(buff[index+1])<<16);
	i |= ((uint32)(buff[index+2])<<8);
	i |= ((uint32)buff[index+3]);

	return i;
}
