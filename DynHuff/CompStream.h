#ifndef _COMP_STREAM_H_
#define _COMP_STREAM_H_

typedef struct CompStream {
	char *text;
	int nextByteIndex;
	int nextBitIndex;
	int length;
} CompStream;

#define EMPTY_COMP_STREAM (CompStream){NULL, 0, 0, 0}

#endif
