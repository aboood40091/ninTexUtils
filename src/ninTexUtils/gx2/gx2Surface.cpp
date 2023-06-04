#include <ninTexUtils/gx2/gx2Surface.h>
#include <ninTexUtils/util.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

extern "C"
{

static inline
bool isMacroTiled(GX2TileMode tileMode)
{
    return tileMode >= GX2_TILE_MODE_TILED_2D_THIN1 &&
           tileMode != GX2_TILE_MODE_LINEAR_SPECIAL;
}

static inline
u32 calcNumLevelsForSize(u32 size)
{
   return 32 - __builtin_clz(size);
}

static inline
u32 calcNumLevels(GX2Surface* surf)
{
    if (surf->numMips <= 1)
        return 1;

    u32 levels = std::max(calcNumLevelsForSize(surf->width),
                          calcNumLevelsForSize(surf->height));
    if (surf->dim == GX2_SURFACE_DIM_3D)
        levels = std::max(calcNumLevelsForSize(surf->depth),
                          levels);

   return levels;
}

void GX2ComputeLevelSurfaceInfo(const GX2Surface* surf, u32 level, ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pSurfInfoOut)
{
    std::memset(pSurfInfoOut, 0, sizeof(ADDR_COMPUTE_SURFACE_INFO_OUTPUT));

    if (surf->tileMode == GX2_TILE_MODE_LINEAR_SPECIAL)
    {
        u32 elemSize = GX2SurfaceIsCompressed(surf->format) ? 4 : 1;
        u32 numSamples = 1 << surf->aa;
        u32 bpp = GX2GetSurfaceFormatBitsPerPixel(surf->format);

        pSurfInfoOut->size = sizeof(ADDR_COMPUTE_SURFACE_INFO_OUTPUT);
        pSurfInfoOut->bpp = bpp;
        pSurfInfoOut->pixelBits = bpp;
        pSurfInfoOut->baseAlign = 1;
        pSurfInfoOut->pitchAlign = 1;
        pSurfInfoOut->heightAlign = 1;
        pSurfInfoOut->depthAlign = 1;

        u32 pixelPitch  = RoundUp(std::max(surf->width  >> level, 1u), elemSize);
        u32 pixelHeight = RoundUp(std::max(surf->height >> level, 1u), elemSize);
        u32 pitch = pixelPitch / elemSize;

        //assert(surf->dim != GX2_SURFACE_DIM_2D_MSAA &&
        //       surf->dim != GX2_SURFACE_DIM_2D_MSAA_ARRAY);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch"

        switch (surf->dim)
        {
        case GX2_SURFACE_DIM_1D:
            pSurfInfoOut->height = 1;
            pSurfInfoOut->depth = 1;
            break;
        case GX2_SURFACE_DIM_2D:
            pSurfInfoOut->height = std::max(surf->height >> level, 1u);
            pSurfInfoOut->depth = 1;
            break;
        case GX2_SURFACE_DIM_3D:
            pSurfInfoOut->height = std::max(surf->height >> level, 1u);
            pSurfInfoOut->depth = std::max(surf->depth >> level, 1u);
            break;
        case GX2_SURFACE_DIM_CUBE:
            pSurfInfoOut->height = std::max(surf->height >> level, 1u);
            pSurfInfoOut->depth = std::max(surf->depth, 6u);
            break;
        case GX2_SURFACE_DIM_1D_ARRAY:
            pSurfInfoOut->height = 1;
            pSurfInfoOut->depth = surf->depth;
            break;
        case GX2_SURFACE_DIM_2D_ARRAY:
            pSurfInfoOut->height = std::max(surf->height >> level, 1u);
            pSurfInfoOut->depth = surf->depth;
        }

#pragma GCC diagnostic pop

        u32 height = RoundUp(pSurfInfoOut->height, elemSize) / elemSize;

        pSurfInfoOut->pixelPitch = std::max(pixelPitch, elemSize);
        pSurfInfoOut->pixelHeight = std::max(pixelHeight, elemSize);
        pSurfInfoOut->pitch = std::max(pitch, 1u);
        pSurfInfoOut->height = std::max(height, 1u);

        u32 numElem = pitch * height;

        u64 surfSize = (u64)numElem * pSurfInfoOut->depth * numSamples * bpp / 8;
        pSurfInfoOut->surfSize = surfSize;

        u32 sliceSize = (u32)surfSize;
        if (surf->dim != GX2_SURFACE_DIM_3D)
            sliceSize /= pSurfInfoOut->depth;

        pSurfInfoOut->sliceSize = sliceSize;

        pSurfInfoOut->pitchTileMax = pitch / 8 - 1;
        pSurfInfoOut->heightTileMax = height / 8 - 1;
        pSurfInfoOut->sliceTileMax = numElem / 64 - 1;
    }
    else
    {
        ADDR_COMPUTE_SURFACE_INFO_INPUT surfInfoIn;
        std::memset(&surfInfoIn, 0, sizeof(ADDR_COMPUTE_SURFACE_INFO_INPUT));

        surfInfoIn.size = sizeof(ADDR_COMPUTE_SURFACE_INFO_INPUT);
        surfInfoIn.tileMode = GX2TileModeToAddrTileMode(surf->tileMode);
        surfInfoIn.format = GX2SurfaceFormatToAddrFormat(surf->format);
        surfInfoIn.bpp = GX2GetSurfaceFormatBitsPerPixel(surf->format);
        surfInfoIn.width = std::max(surf->width >> level, 1u);
        surfInfoIn.numSamples = 1 << surf->aa;

        switch (surf->dim)
        {
        case GX2_SURFACE_DIM_1D:
            surfInfoIn.height = 1;
            surfInfoIn.numSlices = 1;
            break;
        case GX2_SURFACE_DIM_2D:
        case GX2_SURFACE_DIM_2D_MSAA:
            surfInfoIn.height = std::max(surf->height >> level, 1u);
            surfInfoIn.numSlices = 1;
            break;
        case GX2_SURFACE_DIM_3D:
            surfInfoIn.height = std::max(surf->height >> level, 1u);
            surfInfoIn.numSlices = std::max(surf->depth >> level, 1u);
            break;
        case GX2_SURFACE_DIM_CUBE:
            surfInfoIn.height = std::max(surf->height >> level, 1u);
            surfInfoIn.numSlices = std::max(surf->depth, 6u);
            surfInfoIn.flags.value |= 0x8000000;
            break;
        case GX2_SURFACE_DIM_1D_ARRAY:
            surfInfoIn.height = 1;
            surfInfoIn.numSlices = surf->depth;
            break;
        case GX2_SURFACE_DIM_2D_ARRAY:
        case GX2_SURFACE_DIM_2D_MSAA_ARRAY:
            surfInfoIn.height = std::max(surf->height >> level, 1u);
            surfInfoIn.numSlices = surf->depth;
        }

        surfInfoIn.slice = 0;

        if (surf->use & GX2_SURFACE_USE_DEPTH_BUFFER)
            surfInfoIn.flags.value |= 0x40000000;

        if (surf->use & GX2_SURFACE_USE_SCAN_BUFFER)
            surfInfoIn.flags.value |= 0x40000;

        if (surf->dim == GX2_SURFACE_DIM_3D)
            surfInfoIn.flags.value |= 0x4000000;

        surfInfoIn.flags.value &= ~(1u << 0x13);
        surfInfoIn.flags.value |= (u32)(level == 0) << 0x13;

        surfInfoIn.mipLevel = level;
        surfInfoIn.numFrags = surfInfoIn.numSamples;

        pSurfInfoOut->size = sizeof(ADDR_COMPUTE_SURFACE_INFO_OUTPUT);
        AddrComputeSurfaceInfo(&surfInfoIn, pSurfInfoOut);
    }
}

void GX2CalcSurfaceSizeAndAlignment(GX2Surface* surf)
{
    GX2TileMode lastMipTileMode = surf->tileMode;
    u32 lastMipSize = 0;
    u32 mip0Offs = 0;

    bool tileModeChanged = false;
    if (lastMipTileMode == GX2_TILE_MODE_DEFAULT)
    {
        if (surf->dim == GX2_SURFACE_DIM_1D &&
            !(surf->use & GX2_SURFACE_USE_DEPTH_BUFFER) &&
            surf->aa == GX2_AA_MODE_1X)
        {
            lastMipTileMode = GX2_TILE_MODE_LINEAR_ALIGNED;
            surf->tileMode = lastMipTileMode;
        }
        else if (surf->dim == GX2_SURFACE_DIM_3D &&
                 !(surf->use & GX2_SURFACE_USE_COLOR_BUFFER))
        {
            lastMipTileMode = GX2_TILE_MODE_TILED_2D_THICK;
            surf->tileMode = lastMipTileMode;
            tileModeChanged = true;
        }
        else
        {
            lastMipTileMode = GX2_TILE_MODE_TILED_2D_THIN1;
            surf->tileMode = lastMipTileMode;
            tileModeChanged = true;
        }

    }

    surf->numMips = std::max(surf->numMips, 1u);
    surf->numMips = std::min(surf->numMips, calcNumLevels(surf));

    surf->mipOffset[0] = 0;

    surf->swizzle &= 0xFF00FFFFu;
    if (isMacroTiled(surf->tileMode))
        surf->swizzle |= 13 << 16;

    for (u32 level = 0; level < surf->numMips; level++)
    {
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT surfInfo;
        GX2ComputeLevelSurfaceInfo(surf, level, &surfInfo);

        if (level == 0)
        {
            if (tileModeChanged)
            {
                if (surf->tileMode != (GX2TileMode)surfInfo.tileMode)
                {
                    surf->tileMode = (GX2TileMode)surfInfo.tileMode;
                    GX2ComputeLevelSurfaceInfo(surf, 0, &surfInfo);
                    lastMipTileMode = surf->tileMode;
                    if (!isMacroTiled(surf->tileMode))
                        surf->swizzle &= 0xFF00FFFFu;
                }

                if (surf->width < surfInfo.pitchAlign &&
                    surf->height < surfInfo.heightAlign)
                {
                    surf->tileMode = (surf->tileMode == GX2_TILE_MODE_TILED_2D_THICK) ?
                                        GX2_TILE_MODE_TILED_1D_THICK :
                                            GX2_TILE_MODE_TILED_1D_THIN1;
                    GX2ComputeLevelSurfaceInfo(surf, 0, &surfInfo);
                    lastMipTileMode = surf->tileMode;
                    surf->swizzle &= 0xFF00FFFFu;
                }
            }

            surf->imageSize = surfInfo.surfSize;
            surf->alignment = surfInfo.baseAlign;
            surf->pitch = surfInfo.pitch;
        }
        else
        {
            u32 mipPad = 0;

            if (isMacroTiled(lastMipTileMode) && !isMacroTiled((GX2TileMode)surfInfo.tileMode))
            {
                surf->swizzle &= 0xFF00FFFFu;
                surf->swizzle |= level << 16;
                lastMipTileMode = (GX2TileMode)surfInfo.tileMode;

                if (level > 1)
                    mipPad = surf->swizzle & 0xFFFFu;
            }

            mipPad += (surfInfo.baseAlign - (lastMipSize % surfInfo.baseAlign))
                        % surfInfo.baseAlign;

            if (level == 1)
                mip0Offs = lastMipSize + mipPad;

            else
                surf->mipOffset[level - 1] = surf->mipOffset[level - 2] + lastMipSize + mipPad;
        }

        lastMipSize = surfInfo.surfSize;
    }

    u32 mipSize = 0;
    if (surf->numMips > 1)
        mipSize = surf->mipOffset[surf->numMips - 2] + lastMipSize;

    surf->mipSize = mipSize;
    surf->mipOffset[0] = mip0Offs;

    if (surf->format == GX2_SURFACE_FORMAT_UNORM_NV12)
    {
        u32 pad = (surf->alignment - (surf->imageSize % surf->alignment))
                    % surf->alignment;
        surf->mipOffset[0] = surf->imageSize + pad;
        surf->imageSize = surf->imageSize + pad + (surf->imageSize >> 1);
    }
}

void GX2CopySurface(const GX2Surface* src, u32 srcLevel, u32 srcSlice,
                          GX2Surface* dst, u32 dstLevel, u32 dstSlice)
{
    GX2SurfaceFormat srcFormat = src->format;
    GX2SurfaceFormat dstFormat = dst->format;
    u32 bitsPerPixel = GX2GetSurfaceFormatBitsPerPixel(srcFormat);
    u32 bytesPerPixel = bitsPerPixel / 8;

    ADDR_COMPUTE_SURFACE_INFO_OUTPUT srcAddrSurfInfo;
    GX2ComputeLevelSurfaceInfo(src, srcLevel, &srcAddrSurfInfo);

    ADDR_COMPUTE_SURFACE_INFO_OUTPUT dstAddrSurfInfo;
    GX2ComputeLevelSurfaceInfo(dst, dstLevel, &dstAddrSurfInfo);

    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT srcAddrFromCoordIn;
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT dstAddrFromCoordIn;
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT srcAddrFromCoordOut;
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT dstAddrFromCoordOut;

    std::memset(&srcAddrFromCoordIn,  0, sizeof(ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT));
    std::memset(&dstAddrFromCoordIn,  0, sizeof(ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT));
    std::memset(&srcAddrFromCoordOut, 0, sizeof(ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT));
    std::memset(&dstAddrFromCoordOut, 0, sizeof(ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT));

    srcAddrFromCoordIn.size  = sizeof(ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT);
    dstAddrFromCoordIn.size  = sizeof(ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT);
    srcAddrFromCoordOut.size = sizeof(ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT);
    dstAddrFromCoordOut.size = sizeof(ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT);

    srcAddrFromCoordIn.slice = srcSlice;
    srcAddrFromCoordIn.sample = 0;
    srcAddrFromCoordIn.bpp = bitsPerPixel;
    srcAddrFromCoordIn.pitch = srcAddrSurfInfo.pitch;
    srcAddrFromCoordIn.height = srcAddrSurfInfo.height;
    srcAddrFromCoordIn.numSlices = std::max(srcAddrSurfInfo.depth, 1u);
    srcAddrFromCoordIn.numSamples = 1 << src->aa;
    srcAddrFromCoordIn.tileMode = srcAddrSurfInfo.tileMode;
    srcAddrFromCoordIn.isDepth = src->use & GX2_SURFACE_USE_DEPTH_BUFFER;
    srcAddrFromCoordIn.tileBase = 0;
    srcAddrFromCoordIn.compBits = 0;
    //srcAddrFromCoordIn.numFrags = 0;

    //ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT srcExtractBankPipeSwizzleIn;
    //srcExtractBankPipeSwizzleIn.size = sizeof(ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT);
    //ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT srcExtractBankPipeSwizzleOut;
    //srcExtractBankPipeSwizzleOut.size = sizeof(ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT);

    //srcExtractBankPipeSwizzleIn.base256b = src->swizzle >> 8 & 0xFF;
    //AddrExtractBankPipeSwizzle(&srcExtractBankPipeSwizzleIn, &srcExtractBankPipeSwizzleOut);
    //srcAddrFromCoordIn.pipeSwizzle = srcExtractBankPipeSwizzleOut.pipeSwizzle;
    //srcAddrFromCoordIn.bankSwizzle = srcExtractBankPipeSwizzleOut.bankSwizzle;

    R600AddrLib::ExtractBankPipeSwizzle(src->swizzle >> 8 & 0xFF,
                                        &srcAddrFromCoordIn.bankSwizzle,
                                        &srcAddrFromCoordIn.pipeSwizzle);

    dstAddrFromCoordIn.slice = dstSlice;
    dstAddrFromCoordIn.sample = 0;
    dstAddrFromCoordIn.bpp = bitsPerPixel;
    dstAddrFromCoordIn.pitch = dstAddrSurfInfo.pitch;
    dstAddrFromCoordIn.height = dstAddrSurfInfo.height;
    dstAddrFromCoordIn.numSlices = std::max(dstAddrSurfInfo.depth, 1u);
    dstAddrFromCoordIn.numSamples = 1 << dst->aa;
    dstAddrFromCoordIn.tileMode = dstAddrSurfInfo.tileMode;
    dstAddrFromCoordIn.isDepth = dst->use & GX2_SURFACE_USE_DEPTH_BUFFER;
    dstAddrFromCoordIn.tileBase = 0;
    dstAddrFromCoordIn.compBits = 0;
    //dstAddrFromCoordIn.numFrags = 0;

    //ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT dstExtractBankPipeSwizzleIn;
    //dstExtractBankPipeSwizzleIn.size = sizeof(ADDR_EXTRACT_BANKPIPE_SWIZZLE_INPUT);
    //ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT dstExtractBankPipeSwizzleOut;
    //dstExtractBankPipeSwizzleOut.size = sizeof(ADDR_EXTRACT_BANKPIPE_SWIZZLE_OUTPUT);

    //dstExtractBankPipeSwizzleIn.base256b = dst->swizzle >> 8 & 0xFF;
    //AddrExtractBankPipeSwizzle(&dstExtractBankPipeSwizzleIn, &dstExtractBankPipeSwizzleOut);
    //dstAddrFromCoordIn.pipeSwizzle = dstExtractBankPipeSwizzleOut.pipeSwizzle;
    //dstAddrFromCoordIn.bankSwizzle = dstExtractBankPipeSwizzleOut.bankSwizzle;

    R600AddrLib::ExtractBankPipeSwizzle(dst->swizzle >> 8 & 0xFF,
                                        &dstAddrFromCoordIn.bankSwizzle,
                                        &dstAddrFromCoordIn.pipeSwizzle);

    u32 srcLevelWidth = std::max(src->width >> srcLevel, 1u);
    u32 srcLevelHeight = std::max(src->height >> srcLevel, 1u);

    if (GX2SurfaceIsCompressed(srcFormat))
    {
        srcLevelWidth = DivRoundUp(srcLevelWidth, 4);
        srcLevelHeight = DivRoundUp(srcLevelHeight, 4);
    }

    u32 dstLevelWidth = std::max(dst->width >> dstLevel, 1u);
    u32 dstLevelHeight = std::max(dst->height >> dstLevel, 1u);

    if (GX2SurfaceIsCompressed(dstFormat))
    {
        dstLevelWidth = DivRoundUp(dstLevelWidth, 4);
        dstLevelHeight = DivRoundUp(dstLevelHeight, 4);
    }

    uintptr_t pSrcImageData;
    uintptr_t pDstImageData;

    if (srcLevel == 0)
        pSrcImageData = (uintptr_t)src->imagePtr;

    else
    {
        pSrcImageData = (uintptr_t)src->mipPtr;
        if (srcLevel != 1)
            pSrcImageData += src->mipOffset[srcLevel - 1];
    }

    if (dstLevel == 0)
        pDstImageData = (uintptr_t)dst->imagePtr;

    else
    {
        pDstImageData = (uintptr_t)dst->mipPtr;
        if (dstLevel != 1)
            pDstImageData += dst->mipOffset[dstLevel - 1];
    }

    u32 srcSliceOffs = srcSlice * srcAddrSurfInfo.pitch * srcLevelHeight;
    u32 dstSliceOffs = dstSlice * dstAddrSurfInfo.pitch * dstLevelHeight;

    for (u32 y = 0; y < dstLevelHeight; y++)
    {
        for (u32 x = 0; x < dstLevelWidth; x++)
        {
            srcAddrFromCoordIn.x = (x * srcLevelWidth) / dstLevelWidth;
            srcAddrFromCoordIn.y = (y * srcLevelHeight) / dstLevelHeight;
            dstAddrFromCoordIn.x = x;
            dstAddrFromCoordIn.y = y;

            if (src->tileMode == GX2_TILE_MODE_LINEAR_SPECIAL)
                srcAddrFromCoordOut.addr = (srcSliceOffs + y * srcAddrSurfInfo.pitch + x) * bytesPerPixel;

            else
                AddrComputeSurfaceAddrFromCoord(&srcAddrFromCoordIn, &srcAddrFromCoordOut);

            if (dst->tileMode == GX2_TILE_MODE_LINEAR_SPECIAL)
                dstAddrFromCoordOut.addr = (dstSliceOffs + y * dstAddrSurfInfo.pitch + x) * bytesPerPixel;

            else
                AddrComputeSurfaceAddrFromCoord(&dstAddrFromCoordIn, &dstAddrFromCoordOut);

            uintptr_t pSrcPixel = pSrcImageData + (u32)srcAddrFromCoordOut.addr;
            uintptr_t pDstPixel = pDstImageData + (u32)dstAddrFromCoordOut.addr;

            switch (bitsPerPixel)
            {
            case 128: *((u64*)pDstPixel + 1) = *((u64*)pSrcPixel + 1);
            case 64:  *(u64*)pDstPixel = *(u64*)pSrcPixel; break;
            case 32:  *(u32*)pDstPixel = *(u32*)pSrcPixel; break;
            case 16:  *(u16*)pDstPixel = *(u16*)pSrcPixel; break;
            case 8 :  *(u8 *)pDstPixel = *(u8 *)pSrcPixel; break;
            }
        }
    }
}

void LoadGX2Surface(const void* data, GX2Surface* surf, bool serialized, bool isBigEndian)
{
    const GX2Surface* src = (const GX2Surface*)data;
    GX2Surface* dst = surf;

    assert(src != NULL);
    assert(dst != NULL);

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        dst->dim           =    (GX2SurfaceDim)__builtin_bswap32((u32)src->dim);
        dst->width         =                   __builtin_bswap32(     src->width);
        dst->height        =                   __builtin_bswap32(     src->height);
        dst->depth         =                   __builtin_bswap32(     src->depth);
        dst->numMips       =                   __builtin_bswap32(     src->numMips);
        dst->format        = (GX2SurfaceFormat)__builtin_bswap32((u32)src->format);
        dst->aa            =        (GX2AAMode)__builtin_bswap32((u32)src->aa);
        dst->use           =    (GX2SurfaceUse)__builtin_bswap32((u32)src->use);
        dst->imageSize     =                   __builtin_bswap32(     src->imageSize);
        dst->imagePtr      =            (void*)__builtin_bswap32((u32)src->imagePtr);
        dst->mipSize       =                   __builtin_bswap32(     src->mipSize);
        dst->mipPtr        =            (void*)__builtin_bswap32((u32)src->mipPtr);
        dst->tileMode      =      (GX2TileMode)__builtin_bswap32((u32)src->tileMode);
        dst->swizzle       =                   __builtin_bswap32(     src->swizzle);
        dst->alignment     =                   __builtin_bswap32(     src->alignment);
        dst->pitch         =                   __builtin_bswap32(     src->pitch);
        dst->mipOffset[ 0] =                   __builtin_bswap32(     src->mipOffset[ 0]);
        dst->mipOffset[ 1] =                   __builtin_bswap32(     src->mipOffset[ 1]);
        dst->mipOffset[ 2] =                   __builtin_bswap32(     src->mipOffset[ 2]);
        dst->mipOffset[ 3] =                   __builtin_bswap32(     src->mipOffset[ 3]);
        dst->mipOffset[ 4] =                   __builtin_bswap32(     src->mipOffset[ 4]);
        dst->mipOffset[ 5] =                   __builtin_bswap32(     src->mipOffset[ 5]);
        dst->mipOffset[ 6] =                   __builtin_bswap32(     src->mipOffset[ 6]);
        dst->mipOffset[ 7] =                   __builtin_bswap32(     src->mipOffset[ 7]);
        dst->mipOffset[ 8] =                   __builtin_bswap32(     src->mipOffset[ 8]);
        dst->mipOffset[ 9] =                   __builtin_bswap32(     src->mipOffset[ 9]);
        dst->mipOffset[10] =                   __builtin_bswap32(     src->mipOffset[10]);
        dst->mipOffset[11] =                   __builtin_bswap32(     src->mipOffset[11]);
        dst->mipOffset[12] =                   __builtin_bswap32(     src->mipOffset[12]);
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GX2Surface));
    }

    if (serialized)
    {
        assert(dst->width     != 0);
        assert(dst->height    != 0);
        assert(dst->numMips   <= 14);
        assert(dst->imageSize != 0);
        assert(dst->numMips   >  1 ?
                dst->mipSize  != 0 :
                dst->mipSize  == 0);
        assert(dst->pitch     != 0);
        assert(dst->format    >  GX2_SURFACE_FORMAT_INVALID);
        assert(dst->imagePtr  == NULL);
        assert(dst->mipPtr    == NULL);
        assert(dst->tileMode  >  GX2_TILE_MODE_DEFAULT &&
               dst->tileMode  <= GX2_TILE_MODE_LINEAR_SPECIAL);

        dst->depth = std::max(dst->depth, 1u);
        dst->numMips = std::max(dst->numMips, 1u);
    }
}

static const char* const surface_dim_str[8] = {
    "GX2_SURFACE_DIM_1D",            // 0
    "GX2_SURFACE_DIM_2D",            // 1
    "GX2_SURFACE_DIM_3D",            // 2
    "GX2_SURFACE_DIM_CUBE",          // 3
    "GX2_SURFACE_DIM_1D_ARRAY",      // 4
    "GX2_SURFACE_DIM_2D_ARRAY",      // 5
    "GX2_SURFACE_DIM_2D_MSAA",       // 6
    "GX2_SURFACE_DIM_2D_MSAA_ARRAY"  // 7
};

static const char* const aa_mode[4] = {
    "GX2_AA_MODE_1X", // 0
    "GX2_AA_MODE_2X", // 1
    "GX2_AA_MODE_4X", // 2
    "GX2_AA_MODE_8X"  // 3
};

static const char* const tile_mode_str[17] = {
    "GX2_TILE_MODE_DEFAULT",        // 0
    "GX2_TILE_MODE_LINEAR_ALIGNED", // 1
    "GX2_TILE_MODE_TILED_1D_THIN1", // 2
    "GX2_TILE_MODE_TILED_1D_THICK", // 3
    "GX2_TILE_MODE_TILED_2D_THIN1", // 4
    "GX2_TILE_MODE_TILED_2D_THIN2", // 5
    "GX2_TILE_MODE_TILED_2D_THIN4", // 6
    "GX2_TILE_MODE_TILED_2D_THICK", // 7
    "GX2_TILE_MODE_TILED_2B_THIN1", // 8
    "GX2_TILE_MODE_TILED_2B_THIN2", // 9
    "GX2_TILE_MODE_TILED_2B_THIN4", // 10
    "GX2_TILE_MODE_TILED_2B_THICK", // 11
    "GX2_TILE_MODE_TILED_3D_THIN1", // 12
    "GX2_TILE_MODE_TILED_3D_THICK", // 13
    "GX2_TILE_MODE_TILED_3B_THIN1", // 14
    "GX2_TILE_MODE_TILED_3B_THICK", // 15
    "GX2_TILE_MODE_LINEAR_SPECIAL"  // 16
};

void GX2SurfacePrintInfo(const GX2Surface* surf)
{
    std::cout << std::endl;
    std::cout << "// ----- GX2Surface Info ----- "                       << std::endl;
    std::cout << "  dim             = " << surface_dim_str[surf->dim]    << std::endl;
    std::cout << "  width           = " << surf->width                   << std::endl;
    std::cout << "  height          = " << surf->height                  << std::endl;
    std::cout << "  depth           = " << surf->depth                   << std::endl;
    std::cout << "  numMips         = " << surf->numMips                 << std::endl;
    std::cout << "  format          = " << "0x" << std::hex              // TODO
                                        << (u32)surf->format
                                        << std::dec                      << std::endl;
    std::cout << "  aa              = " << aa_mode[surf->aa]             << std::endl;
    std::cout << "  use             = " << "0x" << std::hex              // TODO
                                        << (u32)surf->use
                                        << std::dec                      << std::endl;
    std::cout << "  imageSize       = " << surf->imageSize               << std::endl;
    std::cout << "  mipSize         = " << surf->mipSize                 << std::endl;
    std::cout << "  tileMode        = " << tile_mode_str[surf->tileMode] << std::endl;
    std::cout << "  swizzle         = " << surf->swizzle << ", 0x"
                                        << std::hex
                                        << surf->swizzle
                                        << std::dec                      << std::endl;
    std::cout << "  alignment       = " << surf->alignment               << std::endl;
    std::cout << "  pitch           = " << surf->pitch                   << std::endl;
}

}
