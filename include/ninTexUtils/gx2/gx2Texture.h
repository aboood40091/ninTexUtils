#ifndef NIN_TEX_UTILS_GX2_TEXTURE_H_
#define NIN_TEX_UTILS_GX2_TEXTURE_H_

#include "gx2Surface.h"

typedef struct _GX2Texture
{
    GX2Surface surface;
    u32 viewFirstMip;
    u32 viewNumMips;
    u32 viewFirstSlice;
    u32 viewNumSlices;
    u32 compSel;
    u32 regs[5];
}
GX2Texture;
static_assert(sizeof(GX2Texture) == 0x9C, "GX2Texture size mismatch");

#ifdef __cplusplus
extern "C"
{
#endif

void LoadGX2Texture(
    const void* data,
    GX2Texture* tex,
#ifdef __cplusplus
    bool        serialized  = true,
    bool        isBigEndian = true
#else
    bool        serialized,
    bool        isBigEndian
#endif
);

inline void SaveGX2Texture(
    void* data,
    const GX2Texture* tex,
#ifdef __cplusplus
    bool        serialized  = true,
    bool        isBigEndian = true
#else
    bool        serialized,
    bool        isBigEndian
#endif
)
{
    LoadGX2Texture(tex, (GX2Texture*)data, serialized, isBigEndian);
}

void GX2TexturePrintInfo(const GX2Texture* tex);

#ifdef __cplusplus
}
#endif

#endif
