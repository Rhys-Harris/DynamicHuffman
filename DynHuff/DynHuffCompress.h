#ifndef _DYN_HUFF_COMPRESS_H_
#define _DYN_HUFF_COMPRESS_H_

#include <corecrt.h>

// Compresses using huffman and writes directly to the file
errno_t dynHuffCompress(const char *text, const char *outfilename, const int dataLength);

errno_t dynHuffCompressFile(const char *infilename, const char *outfilename);

#endif
