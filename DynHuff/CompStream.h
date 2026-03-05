#ifndef _COMP_STREAM_H_
#define _COMP_STREAM_H_

#include "../def.h"

typedef struct CompStream {
	byte *text;
	int nextByteIndex;
	int nextBitIndex;
	int length;
} CompStream;

#define EMPTY_COMP_STREAM (CompStream){NULL, 0, 0, 0}

#endif
