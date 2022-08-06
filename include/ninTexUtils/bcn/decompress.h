#ifndef NIN_TEX_UTILS_BCN_DECOMPRESS_H_
#define NIN_TEX_UTILS_BCN_DECOMPRESS_H_

#include <ninTexUtils/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

void BCn_DecompressBC1 (u32 width, u32 height, const u8* in_data, u8* out_data);
void BCn_DecompressBC2 (u32 width, u32 height, const u8* in_data, u8* out_data);
void BCn_DecompressBC3 (u32 width, u32 height, const u8* in_data, u8* out_data);
void BCn_DecompressBC4S(u32 width, u32 height, const u8* in_data, u8* out_data);
void BCn_DecompressBC4U(u32 width, u32 height, const u8* in_data, u8* out_data);
void BCn_DecompressBC5S(u32 width, u32 height, const u8* in_data, u8* out_data);
void BCn_DecompressBC5U(u32 width, u32 height, const u8* in_data, u8* out_data);

inline void BCn_DecompressBC4(u32 width, u32 height, const u8* in_data, u8* out_data, bool snorm)
{
    if (snorm)
        BCn_DecompressBC4S(width, height, in_data, out_data);
    else
        BCn_DecompressBC4U(width, height, in_data, out_data);
}

inline void BCn_DecompressBC5(u32 width, u32 height, const u8* in_data, u8* out_data, bool snorm)
{
    if (snorm)
        BCn_DecompressBC5S(width, height, in_data, out_data);
    else
        BCn_DecompressBC5U(width, height, in_data, out_data);
}

#ifdef __cplusplus
}
#endif

#endif
