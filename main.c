#include <corecrt.h>
#include <stdio.h>

#include "DynHuff/DynHuff.h"

errno_t testHuffMethod(const char infilename[]) {
	printf("== TESTING DYNAMIC HUFF ==\n\n");

	if (dynHuffCompressFile(infilename, "./compoutputs/out.dynhuff")) {
		printf("Couldn't compress\n");
		return 1;
	}

	printf("FINISHED COMPRESSION\n");
	printf("\n\n");

	if (dynHuffDecompressFile("./compoutputs/out.dynhuff", "./finaloutputs/outdyn.txt")) {
		printf("Couldn't decompress\n");
		return 1;
	}
	
	printf("FINISHED DECOMPRESSION\n");

	printf("== END TESTING DYNAMIC HUFF ==\n\n");

	return 0;
}

int main(const int argc, char *argv[]) {
	const char infilename[] = "inputs/in.txt";

	if (testHuffMethod(infilename)) {
		printf("Fail on testing dynamic huff\n");
		return 1;
	}

	return 0;
}
