#include <stdio.h>

#include "./FWrite.h"

errno_t FWriteWhole(const char filename[], const byte *out, const int outLen) {
	FILE *f;
	errno_t err = fopen_s(&f, filename, "wb");
	if (err) {
		printf("Couldn't open output file\n");
		return 1;
	}
	fwrite(out, sizeof(byte), outLen, f);
	fclose(f);
	return 0;
}
