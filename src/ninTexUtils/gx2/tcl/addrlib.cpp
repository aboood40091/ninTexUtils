#include <ninTexUtils/gx2/tcl/addrlib.h>

static inline bool IsPow2(u32 dim)
{
    return !(dim & (dim - 1));
}

static inline bool IsPow2(u64 dim)
{
    return !(dim & (dim - 1));
}

static inline u32 PowTwoAlign(u32 x, u32 align)
{
    return (x + (align - 1)) & (~(align - 1));
}

static inline u64 PowTwoAlign(u64 x, u64 align)
{
    return (x + (align - 1)) & (~(align - 1));
}

static inline u32 NextPow2(u32 dim)
{
    u32 newDim = 1;

    if (dim > 0x7FFFFFFF)
        newDim = 0x80000000;

    else
    {
        while (newDim < dim)
            newDim <<= 1;
    }

    return newDim;
}

static inline u32 Log2(u32 x)
{
    u32 y = 0;

    while (x > 1)
    {
        x >>= 1;
        y++;
    }

    return y;
}

u32 AddrElemLib::GetBitsPerPixel(
    AddrFormat format,
    AddrElemMode* pElemMode,
    u32* pExpandX,
    u32* pExpandY,
    u32* pUnusedBits
)
{
    u32 bpp;
    u32 expandX = 1;
    u32 expandY = 1;
    u32 bitUnused = 0;
    AddrElemMode elemMode = ADDR_UNCOMPRESSED;

    switch(format)
    {
    case ADDR_FMT_8:
    case ADDR_FMT_4_4:
    case ADDR_FMT_3_3_2:
        bpp = 8;
        break;
    case ADDR_FMT_16:
    case ADDR_FMT_16_FLOAT:
    case ADDR_FMT_8_8:
    case ADDR_FMT_5_6_5:
    case ADDR_FMT_6_5_5:
    case ADDR_FMT_1_5_5_5:
    case ADDR_FMT_4_4_4_4:
    case ADDR_FMT_5_5_5_1:
        bpp = 16;
        break;
    case ADDR_FMT_32:
    case ADDR_FMT_32_FLOAT:
    case ADDR_FMT_16_16:
    case ADDR_FMT_16_16_FLOAT:
    case ADDR_FMT_8_24:
    case ADDR_FMT_8_24_FLOAT:
    case ADDR_FMT_24_8:
    case ADDR_FMT_24_8_FLOAT:
    case ADDR_FMT_10_11_11:
    case ADDR_FMT_10_11_11_FLOAT:
    case ADDR_FMT_11_11_10:
    case ADDR_FMT_11_11_10_FLOAT:
    case ADDR_FMT_2_10_10_10:
    case ADDR_FMT_8_8_8_8:
    case ADDR_FMT_10_10_10_2:
    case ADDR_FMT_32_AS_8:
    case ADDR_FMT_32_AS_8_8:
    case ADDR_FMT_5_9_9_9_SHAREDEXP:
        bpp = 32;
        break;
    case ADDR_FMT_X24_8_32_FLOAT:
        bpp = 64;
        bitUnused = 24;
        break;
    case ADDR_FMT_32_32:
    case ADDR_FMT_32_32_FLOAT:
    case ADDR_FMT_16_16_16_16:
    case ADDR_FMT_16_16_16_16_FLOAT:
    case ADDR_FMT_CTX1:
        bpp = 64;
        break;
    case ADDR_FMT_32_32_32_32:
    case ADDR_FMT_32_32_32_32_FLOAT:
        bpp = 128;
        break;
    case ADDR_FMT_1:
        elemMode = ADDR_PACKED_STD;
        expandX = 8;
        bpp = 1;
        break;
    case ADDR_FMT_1_REVERSED:
        elemMode = ADDR_PACKED_REV;
        expandX = 8;
        bpp = 1;
        break;
    case ADDR_FMT_GB_GR:
        elemMode = ADDR_PACKED_GBGR;
        bpp = 16;
        break;
    case ADDR_FMT_BG_RG:
        elemMode = ADDR_PACKED_BGRG;
        bpp = 16;
        break;
    case ADDR_FMT_8_8_8:
        elemMode = ADDR_EXPANDED;
        expandX = 3;
        bpp = 24;
        break;
    case ADDR_FMT_16_16_16:
    case ADDR_FMT_16_16_16_FLOAT:
        elemMode = ADDR_EXPANDED;
        expandX = 3;
        bpp = 48;
        break;
    case ADDR_FMT_32_32_32:
    case ADDR_FMT_32_32_32_FLOAT:
        elemMode = ADDR_EXPANDED;
        expandX = 3;
        bpp = 96;
        break;
    case ADDR_FMT_BC1:
        elemMode = ADDR_PACKED_BC1;
        expandX = 4;
        expandY = 4;
        bpp = 64;
        break;
    case ADDR_FMT_BC2:
        elemMode = ADDR_PACKED_BC2;
        expandX = 4;
        expandY = 4;
        bpp = 128;
        break;
    case ADDR_FMT_BC3:
        elemMode = ADDR_PACKED_BC3;
        expandX = 4;
        expandY = 4;
        bpp = 128;
        break;
    case ADDR_FMT_BC4:
        elemMode = ADDR_PACKED_BC4;
        expandX = 4;
        expandY = 4;
        bpp = 64;
        break;
    case ADDR_FMT_BC5:
    case ADDR_FMT_BC6:
    case ADDR_FMT_BC7:
        elemMode = ADDR_PACKED_BC5;
        expandX = 4;
        expandY = 4;
        bpp = 128;
        break;
    default:
        bpp = 0;
    }

    if (pExpandX)
        *pExpandX = expandX;
    if (pExpandY)
        *pExpandY = expandY;
    if (pUnusedBits)
        *pUnusedBits = bitUnused;
    if (pElemMode)
        *pElemMode = elemMode;

    return bpp;
}

void AddrElemLib::AdjustSurfaceInfo(
    AddrElemMode elemMode,
    u32 expandX,
    u32 expandY,
    u32* pBpp,
    u32* pWidth,
    u32* pHeight
)
{
    u32 packedBits;
    u32 width;
    u32 height;
    u32 bpp;
    bool bBCnFormat = false;

    if (pBpp)
    {
        bpp = *pBpp;

        switch (elemMode)
        {
        case ADDR_EXPANDED:
            packedBits = bpp / expandX / expandY;
            break;
        case ADDR_PACKED_STD:
        case ADDR_PACKED_REV:
            packedBits = bpp * expandX * expandY;
            break;
        case ADDR_PACKED_BC1:
        case ADDR_PACKED_BC4:
            packedBits = 64;
            bBCnFormat = true;
            break;
        case ADDR_PACKED_BC2:
        case ADDR_PACKED_BC3:
        case ADDR_PACKED_BC5:
            bBCnFormat = true;
            packedBits = 128;
            break;
        default:
            packedBits = bpp;
        }

        *pBpp = packedBits;
    }

    if (pWidth && pHeight)
    {
        width  = *pWidth;
        height = *pHeight;

        if (expandX > 1 || expandY > 1)
        {
            if (elemMode == ADDR_EXPANDED)
            {
                width  *= expandX;
                height *= expandY;
            }
            else if (bBCnFormat)
            {
                width  = width  / expandX;
                height = height / expandY;
            }
            else
            {
                width  = (width  + expandX - 1) / expandX;
                height = (height + expandY - 1) / expandY;
            }

            *pWidth  = (width  == 0) ? 1 : width;
            *pHeight = (height == 0) ? 1 : height;
        }
    }
}

void AddrElemLib::RestoreSurfaceInfo(
    AddrElemMode elemMode,
    u32 expandX,
    u32 expandY,
    u32* pBpp,
    u32* pWidth,
    u32* pHeight
)
{
    u32 originalBits;
    u32 width;
    u32 height;
    u32 bpp;

    if (pBpp)
    {
        bpp = *pBpp;

        switch (elemMode)
        {
        case ADDR_EXPANDED:
            originalBits = bpp * expandX * expandY;
            break;
        case ADDR_PACKED_STD:
        case ADDR_PACKED_REV:
            originalBits = bpp / expandX / expandY;
            break;
        case ADDR_PACKED_BC1:
        case ADDR_PACKED_BC4:
            originalBits = 64;
            break;
        case ADDR_PACKED_BC2:
        case ADDR_PACKED_BC3:
        case ADDR_PACKED_BC5:
            originalBits = 128;
            break;
        default:
            originalBits = bpp;
        }

        *pBpp = originalBits;
    }

    if (pWidth && pHeight)
    {
        width  = *pWidth;
        height = *pHeight;

        if (expandX > 1 || expandY > 1)
        {
            if (elemMode == ADDR_EXPANDED)
            {
                width  /= expandX;
                height /= expandY;
            }
            else
            {
                width  *= expandX;
                height *= expandY;
            }
        }

        *pWidth  = (width  == 0) ? 1 : width;
        *pHeight = (height == 0) ? 1 : height;
    }
}

void AddrLib::ComputeMipLevel(
    u32* pWidth,
    u32* pHeight,
    u32* pNumSlices,
    ADDR_SURFACE_FLAGS flags,
    AddrFormat format,
    u32 mipLevel
)
{
    if (AddrElemLib::IsBlockCompressed(format))
    {
        if (mipLevel == 0 || (flags.value >> 0x13 & 1))
        {
            *pWidth = PowTwoAlign(*pWidth, 4);
            *pHeight = PowTwoAlign(*pHeight, 4);
        }
    }

    bool hwlHandled = R600AddrLib::HwlComputeMipLevel(pWidth, pHeight,
                                                      pNumSlices, flags,
                                                      format, mipLevel);
    if (hwlHandled)
    {
        if (mipLevel > 0 && (flags.value >> 0x13 & 1))
        {
            u32 width  = *pWidth >> mipLevel;
            u32 height = *pHeight >> mipLevel;
            u32 numSlices = *pNumSlices;
            if (!(flags.value >> 0x1B & 1))
                numSlices >>= mipLevel;

            if (width == 0)
                width = 1;

            if (height == 0)
                height = 1;

            if (numSlices == 0)
                numSlices = 1;

            if (format != ADDR_FMT_32_32_32 &&
                format != ADDR_FMT_32_32_32_FLOAT)
            {
                width = NextPow2(width);
                height = NextPow2(height);
                numSlices = NextPow2(numSlices);
            }

            *pWidth = width;
            *pHeight = height;
            *pNumSlices = numSlices;
        }
    }
}

ADDR_E_RETURNCODE AddrLib::ComputeSurfaceInfo(
    ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut
)
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;
    //if ((m_configFlags.value & 0x2000000) && (
    //    pIn->size != sizeof(ADDR_COMPUTE_SURFACE_INFO_INPUT) ||
    //    pOut->size != sizeof(ADDR_COMPUTE_SURFACE_INFO_OUTPUT)
    //))
    //    returnCode = ADDR_PARAMSIZEMISMATCH;

    if (pIn->bpp > 128)
        returnCode = ADDR_INVALIDPARAMS;

    if (returnCode == ADDR_OK)
    {
        ComputeMipLevel(&pIn->width, &pIn->height, &pIn->numSlices,
                        pIn->flags, pIn->format, pIn->mipLevel);

        u32 width = pIn->width;
        u32 height = pIn->height;
        u32 bpp = pIn->bpp;
        u32 expandX = 1;
        u32 expandY = 1;
        // Originally left uninitialized
        AddrElemMode elemMode = ADDR_UNCOMPRESSED;

        //u32 sliceFlags = m_configFlags.value >> 26 & 3;

        //ADDR_TILEINFO tileInfoNull = { 0 };
        //if ((m_configFlags.value & 0x1000000)
        //    && pIn->tileIndex != -1
        //    && pIn->pTileInfo == NULL)
        //{
        //    if (pOut->pTileInfo == NULL)
        //    {
        //        pOut->pTileInfo = &tileInfoNull;
        //        pIn->pTileInfo = &tileInfoNull;
        //    }
        //    else
        //    {
        //        pIn->pTileInfo = pOut->pTileInfo;
        //    }
        //}

        //returnCode = HwlSetupTileCfg(pIn->tileIndex,
        //                             pIn->pTileInfo,
        //                             &pIn->tileMode,
        //                             &pIn->tileType);
        //if (returnCode == ADDR_OK)
        //{
            pOut->pixelBits = pIn->bpp;

            if (pIn->format != ADDR_FMT_INVALID)
            {
                bpp = AddrElemLib::GetBitsPerPixel(pIn->format, &elemMode,
                                                   &expandX, &expandY);

                if (elemMode == ADDR_EXPANDED
                    && expandX == 3
                    && pIn->tileMode == ADDR_TM_LINEAR_ALIGNED)
                {
                    pIn->flags.value |= 0x400000;
                }

                AddrElemLib::AdjustSurfaceInfo(elemMode,
                                               expandX,
                                               expandY,
                                               &bpp,
                                               &width,
                                               &height);

                pIn->width = width;
                pIn->height = height;
                pIn->bpp = bpp;
            }
            else if (pIn->bpp != 0)
            {
                if (pIn->width == 0)
                    pIn->width = 1;

                if (pIn->height == 0)
                    pIn->height = 1;
            }
            else
            {
                returnCode = ADDR_INVALIDPARAMS;
            }

            if (returnCode == ADDR_OK)
                returnCode = R600AddrLib::HwlComputeSurfaceInfo(pIn, pOut);

            if (returnCode == ADDR_OK)
            {
                pOut->bpp = pIn->bpp;
                pOut->pixelPitch = pOut->pitch;
                pOut->pixelHeight = pOut->height;

                if (pIn->format != ADDR_FMT_INVALID
                    && !((pIn->flags.value & 0x400000) && pIn->mipLevel > 0))
                {
                    AddrElemLib::RestoreSurfaceInfo(elemMode,
                                                    expandX,
                                                    expandY,
                                                    &bpp,
                                                    &pOut->pixelPitch,
                                                    &pOut->pixelHeight);
                }

                //if ((pIn->flags.value & 0x8000) && pOut->pStereoInfo != NULL)
                //    ComputeQbStereoInfo(pOut);

                //if (sliceFlags == 0)
                //{
                    if (pIn->flags.value & 0x4000000)
                        pOut->sliceSize = (u32)pOut->surfSize;

                    else
                    {
                        pOut->sliceSize = (u32)(pOut->surfSize / pOut->depth);
                        if (pIn->slice == pIn->numSlices - 1
                            && pIn->numSlices > 1)
                        {
                            pOut->sliceSize += pOut->sliceSize
                                            * (pOut->depth - pIn->numSlices);
                        }
                    }
                //}
                //else if (sliceFlags == 1)
                //{
                //    pOut->sliceSize = (u32)(((u64)pOut->pitch * pOut->height * pOut->bpp
                //                             * pIn->numSamples + 7) / 8);
                //}

                pOut->pitchTileMax = (pOut->pitch >> 3) - 1;
                pOut->heightTileMax = (pOut->height >> 3) - 1;
                pOut->sliceTileMax = (pOut->height * pOut->pitch >> 6) - 1;
            }
        //}
    }

    return returnCode;
}

ADDR_E_RETURNCODE AddrLib::ComputeSurfaceAddrFromCoord(
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut
)
{
    ADDR_E_RETURNCODE returnCode = ADDR_OK;
    //if ((m_configFlags.value & 0x2000000) && (
    //    pIn->size != sizeof(ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT) ||
    //    pOut->size != sizeof(ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT)
    //))
    //    returnCode = ADDR_PARAMSIZEMISMATCH;

    //if (returnCode == ADDR_OK)
    //{
        //ADDR_TILEINFO tileInfoNull = { 0 };
        //ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT input;

        //if ((m_configFlags.value & 0x1000000) && pIn->tileIndex != -1)
        //{
        //    input = *pIn;

        //    if (pIn->pTileInfo == NULL)
        //    {
        //        input.pTileInfo = &tileInfoNull;
        //    }

        //    returnCode = HwlSetupTileCfg(input.tileIndex,
        //                                 input.pTileInfo,
        //                                 &input.tileMode,
        //                                 &input.tileType);

        //    pIn = &input;
        //}

        //if (returnCode == ADDR_OK)
        //{
            returnCode = R600AddrLib::HwlComputeSurfaceAddrFromCoord(pIn, pOut);
        //}
    //}

    return returnCode;
}

u32 AddrLib::ComputeSurfaceThickness(AddrTileMode tileMode)
{
    switch(tileMode)
    {
    case ADDR_TM_1D_TILED_THICK:
    case ADDR_TM_2D_TILED_THICK:
    case ADDR_TM_2B_TILED_THICK:
    case ADDR_TM_3D_TILED_THICK:
    case ADDR_TM_3B_TILED_THICK:
        return 4;
    case ADDR_TM_2D_TILED_XTHICK:
    case ADDR_TM_3D_TILED_XTHICK:
        return 8;
    default:
        return 1;
    }
}

void AddrLib::AdjustPitchAlignment(
    ADDR_SURFACE_FLAGS flags,
    u32* pPitchAlign
)
{
    if (flags.value >> 0x12 & 1)
        *pPitchAlign = PowTwoAlign(*pPitchAlign, 32);
}

u32 AddrLib::PadDimensions(
    AddrTileMode tileMode,
    u32 padDims,
    bool cube,
    bool cubeAsArray,
    u32* pPitch,
    u32 pitchAlign,
    u32* pHeight,
    u32 heightAlign,
    u32* pSlices,
    u32 sliceAlign
)
{
    u32 thickness = ComputeSurfaceThickness(tileMode);

    if (padDims == 0)
        padDims = 3;

    if (IsPow2(pitchAlign))
        *pPitch = PowTwoAlign((*pPitch), pitchAlign);

    else
    {
        *pPitch += pitchAlign - 1;
        *pPitch /= pitchAlign;
        *pPitch *= pitchAlign;
    }

    if (padDims > 1)
        *pHeight = PowTwoAlign((*pHeight), heightAlign);

    if (padDims > 2 || thickness > 1)
    {
        if (cube && (/* !(m_configFlags.value & 0x10000000) */ true
                     || cubeAsArray))
            *pSlices = NextPow2(*pSlices);

        if (thickness > 1)
            *pSlices = PowTwoAlign((*pSlices), sliceAlign);
    }

    return padDims;
}

u64 AddrLib::ComputeSurfaceAddrFromCoordLinear(
    u32 x,
    u32 y,
    u32 slice,
    u32 sample,
    u32 bpp,
    u32 pitch,
    u32 height,
    u32 numSlices,
    u32* pBitPosition
)
{
    const u64 sliceSize = (u64)pitch * height;

    u64 sliceOffset = (slice + sample * numSlices) * sliceSize;
    u64 rowOffset   = (u64)y * pitch;
    u64 pixOffset   = x;

    u64 addr = (sliceOffset + rowOffset + pixOffset) * bpp;

    *pBitPosition = (u32)(addr % 8);
    addr /= 8;

    return addr;
}

u32 AddrLib::ComputePixelIndexWithinMicroTile(
    u32 x,
    u32 y,
    u32 z,
    u32 bpp,
    AddrTileMode tileMode,
    AddrTileType microTileType
)
{
    u32 pixelBit0 = 0;
    u32 pixelBit1 = 0;
    u32 pixelBit2 = 0;
    u32 pixelBit3 = 0;
    u32 pixelBit4 = 0;
    u32 pixelBit5 = 0;
    u32 pixelBit6 = 0;
    u32 pixelBit7 = 0;
    u32 pixelBit8 = 0;
    u32 pixelNumber;

    u32 x0 = (x >> 0) & 1;
    u32 x1 = (x >> 1) & 1;
    u32 x2 = (x >> 2) & 1;
    u32 y0 = (y >> 0) & 1;
    u32 y1 = (y >> 1) & 1;
    u32 y2 = (y >> 2) & 1;
    u32 z0 = (z >> 0) & 1;
    u32 z1 = (z >> 1) & 1;
    u32 z2 = (z >> 2) & 1;

    u32 thickness = ComputeSurfaceThickness(tileMode);

    if (microTileType != ADDR_THICK)
    {
        if (microTileType == ADDR_DISPLAYABLE)
        {
            switch (bpp)
            {
            case 8:
                pixelBit0 = x0;
                pixelBit1 = x1;
                pixelBit2 = x2;
                pixelBit3 = y1;
                pixelBit4 = y0;
                break;
            case 16:
                pixelBit0 = x0;
                pixelBit1 = x1;
                pixelBit2 = x2;
                pixelBit3 = y0;
                pixelBit4 = y1;
                break;
            //case 32:
            default:
                pixelBit0 = x0;
                pixelBit1 = x1;
                pixelBit2 = y0;
                pixelBit3 = x2;
                pixelBit4 = y1;
                break;
            case 64:
                pixelBit0 = x0;
                pixelBit1 = y0;
                pixelBit2 = x1;
                pixelBit3 = x2;
                pixelBit4 = y1;
                break;
            case 128:
                pixelBit0 = y0;
                pixelBit1 = x0;
                pixelBit2 = x1;
                pixelBit3 = x2;
                pixelBit4 = y1;
                break;
            }
        }
        else
        {
            pixelBit0 = x0;
            pixelBit1 = y0;
            pixelBit2 = x1;
            pixelBit3 = y1;
            pixelBit4 = x2;
        }

        pixelBit5 = y2;

        if (thickness > 1)
        {
            pixelBit6 = z0;
            pixelBit7 = z1;
        }
    }
    else
    {
        pixelBit0 = x0;
        pixelBit1 = y0;
        pixelBit2 = z0;
        pixelBit3 = x1;
        pixelBit4 = y1;
        pixelBit5 = z1;
        pixelBit6 = x2;
        pixelBit7 = y2;
    }

    if (thickness == 8)
        pixelBit8 = z2;

    pixelNumber = (pixelBit0 << 0) |
                  (pixelBit1 << 1) |
                  (pixelBit2 << 2) |
                  (pixelBit3 << 3) |
                  (pixelBit4 << 4) |
                  (pixelBit5 << 5) |
                  (pixelBit6 << 6) |
                  (pixelBit7 << 7) |
                  (pixelBit8 << 8);

    return pixelNumber;
}

u32 R600AddrLib::ComputeSurfaceNumSplits(
    AddrTileMode tileMode,
    u32 bpp,
    u32 numSamples
)
{
    u32 numSplits = 1;
    u32 tileSizePerSample = (bpp * 64 + 7) / 8;

    if (ComputeSurfaceThickness(tileMode) > 1)
        numSamples = 4;

    if (tileSizePerSample != 0)
    {
        u32 samplesPerSplit = 2048u / tileSizePerSample;
        if (samplesPerSplit != 0)
        {
            numSplits = numSamples / samplesPerSplit;
            if (numSplits == 0)
                numSplits = 1;
        }
    }

    return numSplits;
}

u32 R600AddrLib::ComputeBankPipeRotation(
    AddrTileMode tileMode
)
{
    u32 numPipes = 2;
    u32 numBanks = 4;
    u32 rotation;

    switch (tileMode)
    {
    case ADDR_TM_2D_TILED_THIN1:
    case ADDR_TM_2D_TILED_THIN2:
    case ADDR_TM_2D_TILED_THIN4:
    case ADDR_TM_2D_TILED_THICK:
    case ADDR_TM_2B_TILED_THIN1:
    case ADDR_TM_2B_TILED_THIN2:
    case ADDR_TM_2B_TILED_THIN4:
    case ADDR_TM_2B_TILED_THICK:
        rotation = numPipes * (numBanks / 2 - 1);
        break;
    case ADDR_TM_3D_TILED_THIN1:
    case ADDR_TM_3D_TILED_THICK:
    case ADDR_TM_3B_TILED_THIN1:
    case ADDR_TM_3B_TILED_THICK:
        rotation = (numPipes < 4) ? 1 : (numPipes / 2 - 1);
        break;
    default:
        rotation = 0;
    }

    return rotation;
}

AddrTileMode R600AddrLib::DegradeBankSwapTileMode(
    AddrTileMode baseTileMode
)
{
    switch (baseTileMode)
    {
    case ADDR_TM_2B_TILED_THIN1:
        return ADDR_TM_2D_TILED_THIN1;
    case ADDR_TM_2B_TILED_THIN2:
        return ADDR_TM_2D_TILED_THIN2;
    case ADDR_TM_2B_TILED_THIN4:
        return ADDR_TM_2D_TILED_THIN4;
    case ADDR_TM_2B_TILED_THICK:
        return ADDR_TM_2D_TILED_THICK;
    case ADDR_TM_3B_TILED_THIN1:
        return ADDR_TM_3D_TILED_THIN1;
    case ADDR_TM_3B_TILED_THICK:
        return ADDR_TM_3D_TILED_THICK;
    default:
        return baseTileMode;
    }
}

AddrTileMode R600AddrLib::ComputeSurfaceMipLevelTileMode(
    AddrTileMode baseTileMode,
    u32 bpp,
    u32 mipLevel,
    u32 pitch,
    u32 height,
    u32 numSlices,
    u32 numSamples,
    bool isDepthSampleOrder,
    bool noRecurrence
)
{
    AddrTileMode expTileMode = baseTileMode;
    u32 numPipes = 2u;
    u32 numBanks = 4u;
    u32 groupBytes = 256u;

    u32 numSplits = ComputeSurfaceNumSplits(expTileMode, bpp, numSamples);

    switch (expTileMode)
    {
    case ADDR_TM_1D_TILED_THIN1:
        if (numSamples > 1) // && m_configFlags.value & 0x20000000
            expTileMode = ADDR_TM_2D_TILED_THIN1;
        break;
    case ADDR_TM_1D_TILED_THICK:
        if (numSamples > 1 || isDepthSampleOrder)
        {
            expTileMode = ADDR_TM_1D_TILED_THIN1;
            if (numSamples == 2 || numSamples == 4)
                expTileMode = ADDR_TM_2D_TILED_THICK;
        }
        break;
    //case ADDR_TM_2D_TILED_THIN2:
    //    if (m_pipeInterleaveBytes * 2 > m_splitSize)
    //        expTileMode = ADDR_TM_2D_TILED_THIN1;
    //    break;
    //case ADDR_TM_2D_TILED_THIN4:
    //    if (m_pipeInterleaveBytes * 4 > m_splitSize)
    //        expTileMode = ADDR_TM_2D_TILED_THIN2;
    //    break;
    case ADDR_TM_2D_TILED_THICK:
        if (numSamples > 1 || numSplits > 1 || isDepthSampleOrder)
            expTileMode = ADDR_TM_2D_TILED_THIN1;
        break;
    //case ADDR_TM_2B_TILED_THIN2:
    //    if (m_pipeInterleaveBytes * 2 > m_splitSize)
    //        expTileMode = ADDR_TM_2B_TILED_THIN1;
    //    break;
    //case ADDR_TM_2B_TILED_THIN4:
    //    if (m_pipeInterleaveBytes * 4 > m_splitSize)
    //        expTileMode = ADDR_TM_2B_TILED_THIN2;
    //    break;
    case ADDR_TM_2B_TILED_THICK:
        if (numSamples > 1 || numSplits > 1 || isDepthSampleOrder)
            expTileMode = ADDR_TM_2B_TILED_THIN1;
        break;
    case ADDR_TM_3D_TILED_THICK:
        if (numSamples > 1 || numSplits > 1 || isDepthSampleOrder)
            expTileMode = ADDR_TM_3D_TILED_THIN1;
        break;
    case ADDR_TM_3B_TILED_THICK:
        if (numSamples > 1 || numSplits > 1 || isDepthSampleOrder)
            expTileMode = ADDR_TM_3B_TILED_THIN1;
    }

    if (ComputeBankPipeRotation(expTileMode) % 2u == 0)
    {
        switch (expTileMode)
        {
        case ADDR_TM_3D_TILED_THIN1:
            expTileMode = ADDR_TM_2D_TILED_THIN1;
            break;
        case ADDR_TM_3D_TILED_THICK:
            expTileMode = ADDR_TM_2D_TILED_THICK;
            break;
        case ADDR_TM_3B_TILED_THIN1:
            expTileMode = ADDR_TM_2B_TILED_THIN1;
            break;
        case ADDR_TM_3B_TILED_THICK:
            expTileMode = ADDR_TM_2B_TILED_THICK;
        }
    }

    if (!noRecurrence)
    {
        if (bpp == 96 || bpp == 48 || bpp == 24)
            bpp /= 3;

        pitch = NextPow2(pitch);
        height = NextPow2(height);
        numSlices = NextPow2(numSlices);

        if (mipLevel > 0)
        {
            expTileMode = DegradeBankSwapTileMode(expTileMode);
            u32 microTileThickness = ComputeSurfaceThickness(expTileMode);
            u32 microTileBytes = (numSamples * bpp * microTileThickness * 64 + 7) / 8;

            u32 microTilesPerPipeInterleave = 1;
            if (microTileBytes < groupBytes)
                microTilesPerPipeInterleave = groupBytes / microTileBytes;

            u32 macroTilePitch = numBanks * 8;
            u32 macroTileHeight = numPipes * 8;

            switch (expTileMode)
            {
            case ADDR_TM_2D_TILED_THIN2:
                macroTilePitch /= 2;
                macroTileHeight *= 2;
                break;
            case ADDR_TM_2D_TILED_THIN4:
                macroTilePitch /= 4;
                macroTileHeight *= 4;
            }

            switch (expTileMode)
            {
            case ADDR_TM_1D_TILED_THICK:
                if (numSlices < 4)
                    expTileMode = ADDR_TM_1D_TILED_THIN1;
                break;
            case ADDR_TM_2D_TILED_THIN1:
            case ADDR_TM_2D_TILED_THIN2:
            case ADDR_TM_2D_TILED_THIN4:
            case ADDR_TM_3D_TILED_THIN1:
                if (pitch < macroTilePitch * microTilesPerPipeInterleave
                        || height < macroTileHeight)
                    expTileMode = ADDR_TM_1D_TILED_THIN1;
                break;
            case ADDR_TM_2D_TILED_THICK:
                if (pitch < macroTilePitch * microTilesPerPipeInterleave
                        || height < macroTileHeight)
                    expTileMode = ADDR_TM_1D_TILED_THICK;
                if (numSlices < 4)
                    expTileMode = ADDR_TM_2D_TILED_THIN1;
                break;
            case ADDR_TM_3D_TILED_THICK:
                if (pitch < macroTilePitch * microTilesPerPipeInterleave
                        || height < macroTileHeight)
                    expTileMode = ADDR_TM_1D_TILED_THICK;
                if (numSlices < 4)
                    expTileMode = ADDR_TM_3D_TILED_THIN1;
            }

            expTileMode = ComputeSurfaceMipLevelTileMode(
                expTileMode,
                bpp,
                mipLevel,
                pitch,
                height,
                numSlices,
                numSamples,
                isDepthSampleOrder,
                true
            );
        }
    }

    return expTileMode;
}

u32 R600AddrLib::GetPitchAlignmentLinear(
    u32 bpp
)
{
    u32 pitchAlign = 2048u / bpp;
    if (pitchAlign < 64)
        pitchAlign = 64;

    return pitchAlign;
}

bool R600AddrLib::ComputeSurfaceAlignmentsLinear(
    AddrTileMode tileMode,
    u32 bpp,
    ADDR_SURFACE_FLAGS flags,
    u32* pBaseAlign,
    u32* pPitchAlign,
    u32* pHeightAlign
)
{
    switch (tileMode)
    {
    case ADDR_TM_LINEAR_GENERAL:
        *pBaseAlign = 1;
        *pPitchAlign = (bpp == 1) ? 8 : 1;
        *pHeightAlign = 1;
        break;
    case ADDR_TM_LINEAR_ALIGNED:
        *pBaseAlign = 256u;
        *pPitchAlign = GetPitchAlignmentLinear(bpp);
        *pHeightAlign = 1;
        break;
    default:
        *pBaseAlign = 1;
        *pPitchAlign = 1;
        *pHeightAlign = 1;
    }

    AdjustPitchAlignment(flags, pPitchAlign);

    return true;
}

bool R600AddrLib::ComputeSurfaceInfoLinear(
    AddrTileMode expTileMode,
    u32 bpp,
    u32 numSamples,
    u32 pitchIn,
    u32 heightIn,
    u32 numSlicesIn,
    u32 mipLevel,
    u32 padDims,
    ADDR_SURFACE_FLAGS flags,
    u32* pPitchOut,
    u32* pHeightOut,
    u32* pNumSlicesOut,
    u64* pSurfSize,
    u32* pBaseAlign,
    u32* pPitchAlign,
    u32* pHeightAlign,
    u32* pDepthAlign
)
{
    u32 microTileThickness;
    u32 expPitch = pitchIn;
    u32 expHeight = heightIn;
    u32 expNumSlices = numSlicesIn;

    microTileThickness = ComputeSurfaceThickness(expTileMode);

    u32 baseAlign;
    u32 pitchAlign;
    u32 heightAlign;

    ComputeSurfaceAlignmentsLinear(expTileMode,
                                   bpp,
                                   flags,
                                   &baseAlign,
                                   &pitchAlign,
                                   &heightAlign);

    if ((flags.value >> 0x16 & 1) && mipLevel == 0)
        expPitch = NextPow2(pitchIn / 3);

    else if (mipLevel > 0)
    {
        expPitch = NextPow2(pitchIn);
        expHeight = NextPow2(heightIn);

        if (!(flags.value >> 0x1B & 1))
            expNumSlices = NextPow2(numSlicesIn);

        else
        {
            expNumSlices = numSlicesIn;

            padDims = 2;
            if (numSlicesIn > 1)
                padDims = 0;
        }
    }

    PadDimensions(expTileMode,
                  padDims,
                  flags.value >> 0x1B & 1,
                  flags.value >> 0x18 & 1,
                  &expPitch, pitchAlign,
                  &expHeight, heightAlign,
                  &expNumSlices, microTileThickness);

    if ((flags.value >> 0x16 & 1) && mipLevel == 0)
        expPitch *= 3;

    u32 slices = (numSamples * expNumSlices) / microTileThickness;

    if (pPitchOut != NULL)
        *pPitchOut = expPitch;

    if (pHeightOut != NULL)
        *pHeightOut = expHeight;

    if (pNumSlicesOut != NULL)
        *pNumSlicesOut = expNumSlices;

    u64 surfSize = ((u64)expPitch * expHeight * slices * bpp
                    * numSamples + 7) / 8;

    if (pSurfSize != NULL)
        *pSurfSize = surfSize;

    if (pBaseAlign != NULL)
        *pBaseAlign = baseAlign;

    if (pPitchAlign != NULL)
        *pPitchAlign = pitchAlign;

    if (pHeightAlign != NULL)
        *pHeightAlign = heightAlign;

    if (pDepthAlign != NULL)
        *pDepthAlign = microTileThickness;

    return true;
}

u32 R600AddrLib::GetPitchAlignmentMicroTiled(
    AddrTileMode tileMode,
    u32 bpp,
    ADDR_SURFACE_FLAGS flags,
    u32 numSamples
)
{
    u32 pitchAlign;
    u32 microTileThickness;

    microTileThickness = ComputeSurfaceThickness(tileMode);

    pitchAlign = ((256u / bpp) / numSamples) / microTileThickness;
    if (pitchAlign < 8)
        pitchAlign = 8;

    return pitchAlign;
}

bool R600AddrLib::ComputeSurfaceAlignmentsMicroTiled(
    AddrTileMode tileMode,
    u32 bpp,
    ADDR_SURFACE_FLAGS flags,
    u32 numSamples,
    u32* pBaseAlign,
    u32* pPitchAlign,
    u32* pHeightAlign
)
{
    if (bpp == 96 || bpp == 48 || bpp == 24)
        bpp /= 3;

    *pBaseAlign = 256u;
    *pPitchAlign = GetPitchAlignmentMicroTiled(tileMode, bpp, flags, numSamples);
    *pHeightAlign = 8;

    AdjustPitchAlignment(flags, pPitchAlign);

    return true;
}

bool R600AddrLib::ComputeSurfaceInfoMicroTiled(
    AddrTileMode expTileMode,
    u32 bpp,
    u32 numSamples,
    u32 pitchIn,
    u32 heightIn,
    u32 numSlicesIn,
    u32 mipLevel,
    u32 padDims,
    ADDR_SURFACE_FLAGS flags,
    u32* pPitchOut,
    u32* pHeightOut,
    u32* pNumSlicesOut,
    AddrTileMode* pTileModeOut,
    u64* pSurfSize,
    u32* pBaseAlign,
    u32* pPitchAlign,
    u32* pHeightAlign,
    u32* pDepthAlign
)
{
    u32 microTileThickness;
    u32 expPitch = pitchIn;
    u32 expHeight = heightIn;
    u32 expNumSlices = numSlicesIn;

    microTileThickness = ComputeSurfaceThickness(expTileMode);

    if (mipLevel > 0)
    {
        expPitch = NextPow2(pitchIn);
        expHeight = NextPow2(heightIn);

        if (!(flags.value >> 0x1B & 1))
            expNumSlices = NextPow2(numSlicesIn);

        else
        {
            expNumSlices = numSlicesIn;

            padDims = 2;
            if (numSlicesIn > 1)
                padDims = 0;
        }

        if (expTileMode == ADDR_TM_1D_TILED_THICK && expNumSlices < 4)
        {
            expTileMode = ADDR_TM_1D_TILED_THIN1;
            microTileThickness = 1;
        }
    }

    u32 baseAlign;
    u32 pitchAlign;
    u32 heightAlign;

    ComputeSurfaceAlignmentsMicroTiled(expTileMode,
                                       bpp,
                                       flags,
                                       numSamples,
                                       &baseAlign,
                                       &pitchAlign,
                                       &heightAlign);

    PadDimensions(expTileMode,
                  padDims,
                  flags.value >> 0x1B & 1,
                  flags.value >> 0x18 & 1,
                  &expPitch, pitchAlign,
                  &expHeight, heightAlign,
                  &expNumSlices, microTileThickness);

    if (pPitchOut != NULL)
        *pPitchOut = expPitch;

    if (pHeightOut != NULL)
        *pHeightOut = expHeight;

    if (pNumSlicesOut != NULL)
        *pNumSlicesOut = expNumSlices;

    u64 surfSize = ((u64)expPitch * expHeight * expNumSlices * bpp
                    * numSamples + 7) / 8;

    if (pSurfSize != NULL)
        *pSurfSize = surfSize;

    if (pTileModeOut != NULL)
        *pTileModeOut = expTileMode;

    if (pBaseAlign != NULL)
        *pBaseAlign = baseAlign;

    if (pPitchAlign != NULL)
        *pPitchAlign = pitchAlign;

    if (pHeightAlign != NULL)
        *pHeightAlign = heightAlign;

    if (pDepthAlign != NULL)
        *pDepthAlign = microTileThickness;

    return true;
}

u32 R600AddrLib::ComputeSurfaceBankSwapWidth(
    AddrTileMode tileMode,
    u32 bpp,
    u32 numSamples,
    u32 pitch,
    u32* pNumSplits
)
{
    u32 numPipes = 2;
    u32 numBanks = 4;
    u32 groupBytes = 256u;
    u32 rowSize = 2048u;
    u32 swapSize = 256u;

    u32 bankSwapWidth = 0;

    u32 numSplits = 1;
    u32 tileSizePerSample = (bpp * 64 + 7) / 8;

    u32 samplesPerSplit = 2048u / tileSizePerSample;
    if (samplesPerSplit != 0)
    {
        numSplits = numSamples / samplesPerSplit;
        if (numSplits == 0)
            numSplits = 1;
    }

    if (pNumSplits != NULL)
        *pNumSplits = numSplits;

    if (tileMode == ADDR_TM_2D_TILED_THICK || tileMode == ADDR_TM_2B_TILED_THICK ||
        tileMode == ADDR_TM_3D_TILED_THICK || tileMode == ADDR_TM_3B_TILED_THICK)
    {
        numSamples = 4;
    }

    if (tileMode == ADDR_TM_2B_TILED_THIN1 ||
        tileMode == ADDR_TM_2B_TILED_THIN2 ||
        tileMode == ADDR_TM_2B_TILED_THIN4 ||
        tileMode == ADDR_TM_2B_TILED_THICK ||
        tileMode == ADDR_TM_3B_TILED_THIN1 ||
        tileMode == ADDR_TM_3B_TILED_THICK)
    {
        u32 macroAspectRatio = 1;
        switch (tileMode)
        {
        case ADDR_TM_2D_TILED_THIN2:
        case ADDR_TM_2B_TILED_THIN2:
            macroAspectRatio = 2;
            break;
        case ADDR_TM_2D_TILED_THIN4:
        case ADDR_TM_2B_TILED_THIN4:
            macroAspectRatio = 4;
        }

        u32 swapTiles = (swapSize / 2) / bpp;
        if (swapTiles == 0)
            swapTiles = 1;

        u32 swapWidth = swapTiles * numBanks * 8;

        u32 swapMax = (rowSize * numBanks * numPipes) /
            ((bpp * numPipes * macroAspectRatio * numSamples) / numSplits);

        u32 swapMin = (groupBytes * numBanks * 8) /
            ((tileSizePerSample * numSamples) / numSplits);

        bankSwapWidth =   (swapWidth > swapMax) ? swapMax :
                         ((swapWidth < swapMin) ? swapMin :
                                                  swapWidth);

        while (bankSwapWidth >= pitch * 2)
            bankSwapWidth /= 2;
    }

    return bankSwapWidth;
}

bool R600AddrLib::ComputeSurfaceAlignmentsMacroTiled(
    AddrTileMode tileMode,
    u32 bpp,
    ADDR_SURFACE_FLAGS flags,
    u32 numSamples,
    u32* pBaseAlign,
    u32* pPitchAlign,
    u32* pHeightAlign,
    u32* pMacroWidth,
    u32* pMacroHeight
)
{
    u32 numPipes = 2;
    u32 numBanks = 4;
    u32 groupBytes = 256u;
    u32 tileSplitBytes = 2048u;

    u32 macroAspectRatio = 1;
    switch (tileMode)
    {
    case ADDR_TM_2D_TILED_THIN2:
    case ADDR_TM_2B_TILED_THIN2:
        macroAspectRatio = 2;
        break;
    case ADDR_TM_2D_TILED_THIN4:
    case ADDR_TM_2B_TILED_THIN4:
        macroAspectRatio = 4;
    }

    u32 thickness = ComputeSurfaceThickness(tileMode);

    if (bpp == 96 || bpp == 48 || bpp == 24)
        bpp /= 3;

    if (bpp == 3)
        bpp = 1;

    u32 macroTileWidth = 8 * numBanks / macroAspectRatio;

    u32 pitchAlign = (((groupBytes / bpp) / (thickness * 8)) / numSamples) * macroTileWidth;
    if (pitchAlign < macroTileWidth)
        pitchAlign = macroTileWidth;

    u32 macroTileHeight = 8 * numPipes * macroAspectRatio;

    AdjustPitchAlignment(flags, pPitchAlign);

    u32 macroTileSize = (macroTileWidth * macroTileHeight * bpp + 7) / 8 * numSamples;
    //if (m_chipFamily == ADDR_CHIP_FAMILY_R6XX && numSamples == 1)
    //    macroTileSize *= 2;

    u32 macroTileBitsPerSample = pitchAlign * macroTileHeight * bpp;
    u32 macroTileBytes;
    u32 samplePixelBits;

    if (thickness == 1)
    {
        macroTileBytes = (macroTileBitsPerSample * numSamples + 7) / 8;
        if (macroTileBytes < macroTileSize)
            macroTileBytes = macroTileSize;

        samplePixelBits = numSamples * bpp;
    }
    else
    {
        macroTileBytes = (macroTileBitsPerSample * 4 + 7) / 8;
        if (macroTileBytes < groupBytes)
            macroTileBytes = groupBytes;

        samplePixelBits = thickness * numSamples * bpp;
    }

    u32 microTileBytes = (samplePixelBits * 64 + 7) / 8;

    u32 slicesPerTile = 1;
    if (tileSplitBytes <= microTileBytes)
        slicesPerTile = microTileBytes / tileSplitBytes;

    u32 baseAlign = macroTileBytes / slicesPerTile;
    //if (IsDualBaseAlignNeeded(tileMode))
    //{
    //    ...
    //}

    *pBaseAlign = baseAlign;
    *pPitchAlign = pitchAlign;
    *pHeightAlign = macroTileHeight;
    *pMacroWidth = macroTileWidth;
    *pMacroHeight = macroTileHeight;

    return true;
}

bool R600AddrLib::ComputeSurfaceInfoMacroTiled(
    AddrTileMode baseTileMode,
    AddrTileMode origTileMode,
    u32 bpp,
    u32 numSamples,
    u32 pitchIn,
    u32 heightIn,
    u32 numSlicesIn,
    u32 mipLevel,
    u32 padDims,
    ADDR_SURFACE_FLAGS flags,
    u32* pPitchOut,
    u32* pHeightOut,
    u32* pNumSlicesOut,
    AddrTileMode* pTileModeOut,
    u64* pSurfSize,
    u32* pBaseAlign,
    u32* pPitchAlign,
    u32* pHeightAlign,
    u32* pDepthAlign
)
{
    u32 expPitch = pitchIn;
    u32 expHeight = heightIn;
    u32 expNumSlices = numSlicesIn;

    AddrTileMode expTileMode = baseTileMode;
    u32 microTileThickness = ComputeSurfaceThickness(baseTileMode);

    if (mipLevel > 0)
    {
        expPitch = NextPow2(pitchIn);
        expHeight = NextPow2(heightIn);

        if (!(flags.value >> 0x1B & 1))
            expNumSlices = NextPow2(numSlicesIn);

        else
        {
            expNumSlices = numSlicesIn;

            padDims = 2;
            if (numSlicesIn > 1)
                padDims = 0;
        }

        if (expTileMode == ADDR_TM_2D_TILED_THICK && expNumSlices < 4)
        {
            expTileMode = ADDR_TM_2D_TILED_THIN1;
            microTileThickness = 1;
        }
    }

    u32 baseAlign;
    u32 pitchAlign;
    u32 heightAlign;
    u32 macroTileWidth;
    u32 macroTileHeight;

    if (baseTileMode != origTileMode && mipLevel > 0)
    {
        if ((origTileMode == ADDR_TM_2D_TILED_THICK || origTileMode == ADDR_TM_2B_TILED_THICK ||
             origTileMode == ADDR_TM_3D_TILED_THICK || origTileMode == ADDR_TM_3B_TILED_THICK)
            && !(baseTileMode == ADDR_TM_2D_TILED_THICK || baseTileMode == ADDR_TM_2B_TILED_THICK ||
                 baseTileMode == ADDR_TM_3D_TILED_THICK || baseTileMode == ADDR_TM_3B_TILED_THICK))
        {
            ComputeSurfaceAlignmentsMacroTiled(origTileMode, bpp, flags,
                                               numSamples, &baseAlign,
                                               &pitchAlign, &heightAlign,
                                               &macroTileWidth,
                                               &macroTileHeight);

            u32 macroTilesPerPipeInterleave = (256u / 8) / bpp;
            if (macroTilesPerPipeInterleave == 0)
                macroTilesPerPipeInterleave = 1;

            if (expPitch < pitchAlign * macroTilesPerPipeInterleave
                    || expHeight < heightAlign)
                return ComputeSurfaceInfoMicroTiled(ADDR_TM_1D_TILED_THIN1,
                                                     bpp, numSamples,
                                                     pitchIn, heightIn, numSlicesIn,
                                                     mipLevel, padDims, flags,
                                                     pPitchOut,pHeightOut,
                                                     pNumSlicesOut, pTileModeOut,
                                                     pSurfSize, pBaseAlign,
                                                     pPitchAlign, pHeightAlign,
                                                     pDepthAlign);
        }
    }
    ComputeSurfaceAlignmentsMacroTiled(baseTileMode, bpp, flags,
                                       numSamples, &baseAlign,
                                       &pitchAlign, &heightAlign,
                                       &macroTileWidth,
                                       &macroTileHeight);

    u32 bankSwapWidth = ComputeSurfaceBankSwapWidth(baseTileMode, bpp,
                                                    numSamples, pitchIn, NULL);
    if (pitchAlign < bankSwapWidth)
        pitchAlign = bankSwapWidth;

    //if (IsDualPitchAlignNeeded(baseTileMode, flags.value >> 0x1E & 1, mipLevel))
    //{
    //    ...
    //}

    PadDimensions(baseTileMode,
                  padDims,
                  flags.value >> 0x1B & 1,
                  flags.value >> 0x18 & 1,
                  &expPitch, pitchAlign,
                  &expHeight, heightAlign,
                  &expNumSlices, microTileThickness);

    if (pPitchOut != NULL)
        *pPitchOut = expPitch;

    if (pHeightOut != NULL)
        *pHeightOut = expHeight;

    if (pNumSlicesOut != NULL)
        *pNumSlicesOut = expNumSlices;

    u64 surfSize = ((u64)expPitch * expHeight * expNumSlices * bpp
                    * numSamples + 7) / 8;

    if (pSurfSize != NULL)
        *pSurfSize = surfSize;

    if (pTileModeOut != NULL)
        *pTileModeOut = expTileMode;

    if (pBaseAlign != NULL)
        *pBaseAlign = baseAlign;

    if (pPitchAlign != NULL)
        *pPitchAlign = pitchAlign;

    if (pHeightAlign != NULL)
        *pHeightAlign = heightAlign;

    if (pDepthAlign != NULL)
        *pDepthAlign = microTileThickness;

    return true;
}

bool R600AddrLib::DispatchComputeSurfaceInfo(
    ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut
)
{
    AddrTileMode tileMode = pIn->tileMode;
    u32 bpp = pIn->bpp;
    u32 numSamples = (pIn->numSamples != 0) ? pIn->numSamples : 1;
    u32 pitch = pIn->width;
    u32 height = pIn->height;
    u32 numSlices = pIn->numSlices;
    u32 mipLevel = pIn->mipLevel;
    ADDR_SURFACE_FLAGS flags = pIn->flags;

    u32 padDims = 0;
    bool valid;

    u32* pPitchOut = &pOut->pitch;
    u32* pHeightOut = &pOut->height;
    u32* pNumSlicesOut = &pOut->depth;
    AddrTileMode* pTileModeOut = &pOut->tileMode;
    u64* pSurfSize = &pOut->surfSize;
    u32* pBaseAlign = &pOut->baseAlign;
    u32* pPitchAlign = &pOut->pitchAlign;
    u32* pHeightAlign = &pOut->heightAlign;
    u32* pDepthAlign = &pOut->depthAlign;

    if (flags.value >> 0x1B & 1)
        if (mipLevel == 0)
            padDims = 2;

    AddrTileMode expTileMode;
    if (!(flags.value >> 0x19 & 1))
        expTileMode = ComputeSurfaceMipLevelTileMode(tileMode, bpp, mipLevel,
                                                     pitch, height, numSlices,
                                                     numSamples,
                                                     flags.value >> 0x1E & 1);

    else
        expTileMode = DegradeBankSwapTileMode(tileMode);

    switch (expTileMode)
    {
    case ADDR_TM_LINEAR_GENERAL:
    case ADDR_TM_LINEAR_ALIGNED:
        valid = ComputeSurfaceInfoLinear(expTileMode, bpp, numSamples, pitch,
                                         height, numSlices, mipLevel, padDims,
                                         flags, pPitchOut, pHeightOut,
                                         pNumSlicesOut, pSurfSize, pBaseAlign,
                                         pPitchAlign, pHeightAlign,
                                         pDepthAlign);
        *pTileModeOut = expTileMode;
        break;
    case ADDR_TM_1D_TILED_THIN1:
    case ADDR_TM_1D_TILED_THICK:
        valid = ComputeSurfaceInfoMicroTiled(expTileMode, bpp, numSamples,
                                             pitch, height, numSlices,
                                             mipLevel, padDims, flags,
                                             pPitchOut, pHeightOut,
                                             pNumSlicesOut, pTileModeOut,
                                             pSurfSize, pBaseAlign,
                                             pPitchAlign, pHeightAlign,
                                             pDepthAlign);
        break;
    case ADDR_TM_2D_TILED_THIN1:
    case ADDR_TM_2D_TILED_THIN2:
    case ADDR_TM_2D_TILED_THIN4:
    case ADDR_TM_2D_TILED_THICK:
    case ADDR_TM_2B_TILED_THIN1:
    case ADDR_TM_2B_TILED_THIN2:
    case ADDR_TM_2B_TILED_THIN4:
    case ADDR_TM_2B_TILED_THICK:
    case ADDR_TM_3D_TILED_THIN1:
    case ADDR_TM_3D_TILED_THICK:
    case ADDR_TM_3B_TILED_THIN1:
    case ADDR_TM_3B_TILED_THICK:
        valid = ComputeSurfaceInfoMacroTiled(expTileMode, tileMode, bpp,
                                             numSamples, pitch, height,
                                             numSlices, mipLevel, padDims,
                                             flags, pPitchOut, pHeightOut,
                                             pNumSlicesOut, pTileModeOut,
                                             pSurfSize, pBaseAlign,
                                             pPitchAlign, pHeightAlign,
                                             pDepthAlign);
        break;
    default:
        valid = false;
    }

    return valid;
}

u64 R600AddrLib::ComputeSurfaceAddrFromCoordMicroTiled(
    u32 x,
    u32 y,
    u32 slice,
    u32 bpp,
    u32 pitch,
    u32 height,
    AddrTileMode tileMode,
    bool isDepthSampleOrder,
    u32 tileBase,
    u32 compBits,
    u32* pBitPosition
)
{
    u32 microTileThickness = (tileMode == ADDR_TM_1D_TILED_THICK) ? 4 : 1;

    u32 microTileBytes = (bpp * 64 + 7) / 8 * microTileThickness;

    u64 sliceBytes = ((u64)pitch * height * microTileThickness * bpp + 7) / 8;

    u32 microTilesPerRow = pitch / 8;

    u32 microTileIndexX = x / 8;
    u32 microTileIndexY = y / 8;
    u32 microTileIndexZ = slice / microTileThickness;

    u64 sliceOffset = (u64)microTileIndexZ * sliceBytes;

    u64 microTileOffset = ((u64)microTileIndexY * microTilesPerRow + microTileIndexX)
                           * microTileBytes;

    u32 pixelIndex = ComputePixelIndexWithinMicroTile(x,
                                                      y,
                                                      slice,
                                                      bpp,
                                                      tileMode,
                                                      (AddrTileType)isDepthSampleOrder);

    u32 elemOffset;

    if (compBits != 0 && compBits != bpp && isDepthSampleOrder)
        elemOffset = tileBase + pixelIndex * compBits;

    else
        elemOffset = pixelIndex * bpp;

    *pBitPosition = elemOffset % 8;
    elemOffset /= 8;

    u64 addr = sliceOffset + microTileOffset + elemOffset;

    return addr;
}

u32 R600AddrLib::ComputePipeFromCoord(
    u32 x,
    u32 y
)
{
    u32 numPipes = 2;
    switch (numPipes)
    {
    case 2:
        return (x ^ y) >> 3 & 1;
    case 4:
        return (((x >> 3 ^ y >> 4) & 1) << 1) |
                ((x >> 4 ^ y >> 3) & 1);
    case 8:
        return ((x >> 5          ^ y >> 3) & 1) |
              (((x >> 4 ^ x >> 5 ^ y >> 4) & 1) << 1) |
              (((x >> 3          ^ y >> 5) & 1) << 2);
    default:
        return 0;
    }
}

u32 R600AddrLib::ComputeBankFromCoord(
    u32 x,
    u32 y
)
{
    u32 numPipes = 2;
    u32 numBanks = 4;

    u32 uVar1 = /* m_configFlags.value >> 0x1E & 1 */ 0;
    u32 uVar2;
    u32 uVar3;

    switch (numBanks)
    {
    case 4:
        uVar3 = (x >> 3 ^ y / (numPipes << 4)) & 1;
        if (uVar1 == 1 && numPipes == 8)
            uVar3 ^= x >> 5 & 1;

        return ((x >> 4 ^ y / (numPipes << 3)) & 1) << 1 | uVar3;
    case 8:
        uVar2 = y / (numPipes << 5);
        uVar3 = (x >> 3 ^ uVar2) & 1;
        if (uVar1 == 1 && numPipes == 8)
            uVar3 ^= x >> 6 & 1;

        return ((x >> 5 ^ y / (numPipes << 3)        ) & 1) << 2 |
               ((x >> 4 ^ y / (numPipes << 4) ^ uVar2) & 1) << 1 | uVar3;
    default:
        return 0;
    }
}

u64 R600AddrLib::ComputeSurfaceAddrFromCoordMacroTiled(
    u32 x,
    u32 y,
    u32 slice,
    u32 sample,
    u32 bpp,
    u32 pitch,
    u32 height,
    u32 numSamples,
    AddrTileMode tileMode,
    bool isDepthSampleOrder,
    u32 tileBase,
    u32 compBits,
    u32 pipeSwizzle,
    u32 bankSwizzle,
    u32* pBitPosition
)
{
    u32 numPipes = 2;
    u32 numBanks = 4;

    u32 numGroupBits = Log2(256u);
    u32 numPipeBits = Log2(numPipes);
    u32 numBankBits = Log2(numBanks);

    u32 microTileThickness = ComputeSurfaceThickness(tileMode);

    u32 microTileBits = 64 * microTileThickness * bpp * numSamples;
    u32 microTileBytes = microTileBits / 8;

    u32 pixelIndex = ComputePixelIndexWithinMicroTile(x,
                                                      y,
                                                      slice,
                                                      bpp,
                                                      tileMode,
                                                      (AddrTileType)isDepthSampleOrder);

    u32 elementOffset;

    if (isDepthSampleOrder)
    {
        if (compBits != 0 && compBits != bpp)
        {
            elementOffset = tileBase + sample * compBits + pixelIndex * compBits * numSamples;
        }
        else
        {
            elementOffset = sample * bpp + pixelIndex * bpp * numSamples;
        }
    }
    else
    {
        elementOffset = sample * (microTileBits / numSamples) + pixelIndex * bpp;
    }

    *pBitPosition = elementOffset % 8;

    u32 samplesPerSplit;
    u32 slicesPerTile;
    u32 tileSplitSlice;

    if (numSamples > 1 && microTileBytes > 2048u)
    {
        samplesPerSplit = 2048u / (microTileBytes / numSamples);
        slicesPerTile = numSamples / samplesPerSplit;
        u32 tileBits = microTileBits / slicesPerTile;
        tileSplitSlice = elementOffset / tileBits;
        elementOffset %= tileBits;
    }
    else
    {
        samplesPerSplit = numSamples;
        slicesPerTile = 1;
        tileSplitSlice = 0;
    }

    elementOffset /= 8;

    u32 pipe = ComputePipeFromCoord(x, y);
    u32 bank = ComputeBankFromCoord(x, y);

    u32 bankPipe = bank * numPipes + pipe;
    u32 bankPipeRotation = ComputeBankPipeRotation(tileMode);
    u32 bankPipeSwizzle = bankSwizzle * numPipes + pipeSwizzle;

    u32 microTileIndexZ = slice;
    if (tileMode == ADDR_TM_2D_TILED_THICK || tileMode == ADDR_TM_2B_TILED_THICK ||
        tileMode == ADDR_TM_3D_TILED_THICK || tileMode == ADDR_TM_3B_TILED_THICK)
    {
        microTileIndexZ /= 4;
    }

    bankPipe ^= bankPipeRotation * microTileIndexZ + bankPipeSwizzle;
    bankPipe ^= tileSplitSlice * ((numBanks / 2) + 1) * numPipes;
    bankPipe %= numBanks * numPipes;

    bank = bankPipe / numPipes;
    pipe = bankPipe % numPipes;

    u64 sliceBytes = ((u64)pitch * height * microTileThickness * bpp * samplesPerSplit + 7) / 8;

    u64 sliceOffset = sliceBytes * ((slice * slicesPerTile + tileSplitSlice) / microTileThickness);

    u32 macroTilePitch = numBanks * 8;
    u32 macroTileHeight = numPipes * 8;

    switch (tileMode)
    {
    case ADDR_TM_2D_TILED_THIN2:
    case ADDR_TM_2B_TILED_THIN2:
        macroTilePitch /= 2;
        macroTileHeight *= 2;
        break;
    case ADDR_TM_2D_TILED_THIN4:
    case ADDR_TM_2B_TILED_THIN4:
        macroTilePitch /= 4;
        macroTileHeight *= 4;
    }

    u32 macroTileBytes = (macroTilePitch * macroTileHeight * bpp * microTileThickness
                          * samplesPerSplit + 7) / 8;

    u32 macroTilesPerRow = pitch / macroTilePitch;

    u32 macroTileIndexX = x / macroTilePitch;
    u32 macroTileIndexY = y / macroTileHeight;

    u64 macroTileOffset = ((u64)macroTileIndexY * macroTilesPerRow + macroTileIndexX)
                           * macroTileBytes;

    if (tileMode == ADDR_TM_2B_TILED_THIN1 ||
        tileMode == ADDR_TM_2B_TILED_THIN2 ||
        tileMode == ADDR_TM_2B_TILED_THIN4 ||
        tileMode == ADDR_TM_2B_TILED_THICK ||
        tileMode == ADDR_TM_3B_TILED_THIN1 ||
        tileMode == ADDR_TM_3B_TILED_THICK)
    {
        static const u32 bankSwapRotation[] = { 0, 1, 3, 2, 6, 7, 5, 4 };
        u32 bankSwapWidth = ComputeSurfaceBankSwapWidth(tileMode, bpp, samplesPerSplit, pitch, NULL);
        bank ^= bankSwapRotation[((macroTileIndexX * macroTilePitch) / bankSwapWidth) % numBanks];
    }

    u64 totalOffset = ((sliceOffset + macroTileOffset) >> (numPipeBits + numBankBits)) + elementOffset;

    u64 groupMask = (1 << numGroupBits) - 1;
    u64 offsetLo  =  totalOffset &  groupMask;
    u64 offsetHi  = (totalOffset & ~groupMask) << (numPipeBits + numBankBits);

    u64 addr  = offsetLo;
    addr     |= offsetHi;
    addr     |= pipe << numGroupBits;
    addr     |= bank << (numGroupBits + numPipeBits);

    return addr;
}

u64 R600AddrLib::DispatchComputeSurfaceAddrFromCoord(
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut
)
{
    u32 x = pIn->x;
    u32 y = pIn->y;
    u32 slice = pIn->slice;
    u32 sample = pIn->sample;
    u32 bpp = pIn->bpp;
    u32 pitch = pIn->pitch;
    u32 height = pIn->height;
    u32 numSlices = pIn->numSlices;
    u32 numSamples = (pIn->numSamples == 0) ? 1 : pIn->numSamples;
    AddrTileMode tileMode = pIn->tileMode;
    bool isDepthSampleOrder = pIn->isDepth;
    u32 tileBase = pIn->tileBase;
    u32 compBits = pIn->compBits;
    u32 pipeSwizzle = pIn->pipeSwizzle;
    u32 bankSwizzle = pIn->bankSwizzle;

    u32* pBitPosition = &pOut->bitPosition;
    u64 addr;

    switch (tileMode)
    {
    case ADDR_TM_LINEAR_GENERAL:
    case ADDR_TM_LINEAR_ALIGNED:
        addr = ComputeSurfaceAddrFromCoordLinear(x,
                                                 y,
                                                 slice,
                                                 sample,
                                                 bpp,
                                                 pitch,
                                                 height,
                                                 numSlices,
                                                 pBitPosition);
        break;
    case ADDR_TM_1D_TILED_THIN1:
    case ADDR_TM_1D_TILED_THICK:
        addr = ComputeSurfaceAddrFromCoordMicroTiled(x,
                                                     y,
                                                     slice,
                                                     bpp,
                                                     pitch,
                                                     height,
                                                     tileMode,
                                                     isDepthSampleOrder,
                                                     tileBase,
                                                     compBits,
                                                     pBitPosition);
        break;
    case ADDR_TM_2D_TILED_THIN1:
    case ADDR_TM_2D_TILED_THIN2:
    case ADDR_TM_2D_TILED_THIN4:
    case ADDR_TM_2D_TILED_THICK:
    case ADDR_TM_2B_TILED_THIN1:
    case ADDR_TM_2B_TILED_THIN2:
    case ADDR_TM_2B_TILED_THIN4:
    case ADDR_TM_2B_TILED_THICK:
    case ADDR_TM_3D_TILED_THIN1:
    case ADDR_TM_3D_TILED_THICK:
    case ADDR_TM_3B_TILED_THIN1:
    case ADDR_TM_3B_TILED_THICK:
        addr = ComputeSurfaceAddrFromCoordMacroTiled(x,
                                                     y,
                                                     slice,
                                                     sample,
                                                     bpp,
                                                     pitch,
                                                     height,
                                                     numSamples,
                                                     tileMode,
                                                     isDepthSampleOrder,
                                                     tileBase,
                                                     compBits,
                                                     pipeSwizzle,
                                                     bankSwizzle,
                                                     pBitPosition);
        break;
    default:
        addr = 0;
    }

    return addr;
}

ADDR_E_RETURNCODE R600AddrLib::HwlComputeSurfaceInfo(
    ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut
)
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;
    if (!DispatchComputeSurfaceInfo(pIn, pOut))
        retCode = ADDR_INVALIDPARAMS;

    return retCode;
}

ADDR_E_RETURNCODE R600AddrLib::HwlComputeSurfaceAddrFromCoord(
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut
)
{
    ADDR_E_RETURNCODE retCode = ADDR_OK;
    if (pIn->pipeSwizzle >= 2 ||
        pIn->bankSwizzle >= 4 ||
        pIn->x > pIn->pitch   ||
        pIn->y > pIn->height  ||
        pIn->numSamples > 8)
    {
        retCode = ADDR_INVALIDPARAMS;
    }
    else
    {
        pOut->addr = DispatchComputeSurfaceAddrFromCoord(pIn, pOut);
    }

    return retCode;
}

bool R600AddrLib::HwlComputeMipLevel(
    u32* pWidth,
    u32* pHeight,
    u32* pNumSlices,
    ADDR_SURFACE_FLAGS flags,
    AddrFormat format,
    u32 mipLevel
)
{
    if (!AddrElemLib::IsBlockCompressed(format))
        return false;

    if (mipLevel > 0)
    {
        u32 width  = *pWidth;
        u32 height = *pHeight;
        u32 numSlices = *pNumSlices;

        if (flags.value >> 0x13 & 1)
        {
            width >>= mipLevel;
            height >>= mipLevel;
            if (!(flags.value >> 0x1B & 1))
                numSlices >>= mipLevel;

            if (width == 0)
                width = 1;

            if (height == 0)
                height = 1;

            if (numSlices == 0)
                numSlices = 1;
        }

        width = NextPow2(width);
        height = NextPow2(height);

        *pWidth = width;
        *pHeight = height;
        *pNumSlices = numSlices;
    }

    return true;
}

extern "C"
{

ADDR_E_RETURNCODE AddrComputeSurfaceInfo(
    ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut
)
{
    return AddrLib::ComputeSurfaceInfo(pIn, pOut);
}

ADDR_E_RETURNCODE AddrComputeSurfaceAddrFromCoord(
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut
)
{
    return AddrLib::ComputeSurfaceAddrFromCoord(pIn, pOut);
}

}
