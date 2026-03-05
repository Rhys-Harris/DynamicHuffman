#ifndef _F_READ_H_
#define _F_READ_H_

#include <corecrt.h>

#include "./def.h"

errno_t FReadWhole(const char filename[], byte **out, long *len);

#endif
