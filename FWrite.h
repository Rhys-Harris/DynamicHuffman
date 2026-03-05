#ifndef _F_WRITE_H_
#define _F_WRITE_H_

#include <corecrt.h>

#include "./def.h"

errno_t FWriteWhole(const char filename[], const byte *out, const int outLen);

#endif
