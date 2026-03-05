#ifndef _DYN_HUFF_DECOMPRESS_H_
#define _DYN_HUFF_DECOMPRESS_H_

#include <corecrt.h>

errno_t dynHuffDecompress(const char *compText, const char *outfilename);

errno_t dynHuffDecompressFile(const char *infilename, const char *outfilename);

#endif
