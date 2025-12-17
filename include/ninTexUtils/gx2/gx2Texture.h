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
    u32 _regs[5];
}
GX2Texture;
static_assert(sizeof(GX2Texture) == 0x9C, "GX2Texture size mismatch");

#ifdef __cplusplus
extern "C"
{
#endif

void GX2InitTextureRegs(
    GX2Texture* texture,
#ifdef __cplusplus
    bool        gfd_v7 = true
#else
    bool        gfd_v7
#endif
);

void GX2TextureVerifyForSerialization(const GX2Texture* tex);

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
    bool        isBigEndian = true
#else
    bool        isBigEndian
#endif
)
{
    assert(tex);

    GX2Texture* tex_cc = (GX2Texture*)tex;
    ScopedSerializedPtrNullSetter imageNullSetter(&tex_cc->surface.imagePtr, tex == data);
    ScopedSerializedPtrNullSetter mipNullSetter(&tex_cc->surface.mipPtr, tex == data);

    GX2SurfaceVerifyForSerialization(&tex->surface);
    GX2TextureVerifyForSerialization(tex);
    LoadGX2Texture(tex, (GX2Texture*)data, false, isBigEndian);
}

void GX2TexturePrintInfo(const GX2Texture* tex);

void GX2TextureFromLinear2D(
    GX2Texture*      texture,
    u32              width,
    u32              height,
    u32              numMips,
    GX2SurfaceFormat format,
    u32              compSel,
    const u8*        imagePtr,
    size_t           imageSize,
#ifdef __cplusplus
    GX2TileMode      tileMode = GX2_TILE_MODE_DEFAULT,
    u32              swizzle  = 0,
    const u8*        mipPtr   = nullptr,
    size_t           mipSize  = 0,
    bool             gfd_v7   = true
#else
    GX2TileMode      tileMode,
    u32              swizzle,
    const u8*        mipPtr,
    size_t           mipSize,
    bool             gfd_v7
#endif
);

void GX2TextureFromDDS(
    GX2Texture* texture,
    const u8*   file,
    size_t      fileSize,
#ifdef __cplusplus
    GX2TileMode tileMode   = GX2_TILE_MODE_DEFAULT,
    u32         swizzle    = 0,
    bool        SRGB       = false,
    u32         compSelIdx = 0x00010203,
    bool        gfd_v7     = true,
    bool        printInfo  = true
#else
    GX2TileMode tileMode,
    u32         swizzle,
    bool        SRGB,
    u32         compSelIdx,
    bool        gfd_v7,
    bool        printInfo
#endif
);

u8* GX2TextureToDDS(
    const GX2Texture* texture,
    size_t*           fileSize,
#ifdef __cplusplus
    bool              printInfo  = true
#else
    bool              printInfo
#endif
);

#ifdef __cplusplus
}
#endif

#endif
