#ifndef NIN_TEX_UTILS_UTIL_H_
#define NIN_TEX_UTILS_UTIL_H_

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

inline u32 DivRoundUp(u32 n, u32 d)
{
    return (n + (d - 1)) / d;
}

inline u32 RoundUp(u32 x, u32 y)
{
    return ((x - 1) | (y - 1)) + 1;
}

#ifdef __cplusplus
}
#endif

#endif
