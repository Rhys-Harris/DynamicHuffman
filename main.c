#include <corecrt.h>
#include <stdio.h>
#include <time.h>

#include "DynHuff/DynHuff.h"

errno_t testHuffMethod(const char infilename[]) {
	printf("== TESTING DYNAMIC HUFF ==\n\n");

	clock_t t1 = clock();
	if (dynHuffCompressFile(infilename, "./compoutputs/out.dynhuff")) {
		printf("Couldn't compress\n");
		return 1;
	}
	clock_t t2 = clock();

	printf("FINISHED COMPRESSION\n");
	printf("\n\n");

	clock_t t3 = clock();
	if (dynHuffDecompressFile("./compoutputs/out.dynhuff", "./finaloutputs/outdyn.mp4")) {
		printf("Couldn't decompress\n");
		return 1;
	}
	clock_t t4 = clock();
	
	printf("FINISHED DECOMPRESSION\n");

	printf("  COMP TIME: %lis (%lims)\n", (t2-t1)/CLOCKS_PER_SEC, t2-t1);
	printf("DECOMP TIME: %lis (%lims)\n", (t4-t3)/CLOCKS_PER_SEC, t4-t3);

	printf("== END TESTING DYNAMIC HUFF ==\n\n");

	return 0;
}

int main(const int argc, char *argv[]) {
	const char infilename[] = "./inputs/bayblades.mp4";

	if (testHuffMethod(infilename)) {
		printf("Fail on testing dynamic huff\n");
		return 1;
	}

	return 0;
}
