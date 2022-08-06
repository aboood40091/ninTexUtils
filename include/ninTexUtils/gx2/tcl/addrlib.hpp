#pragma once

// This file is included inside addrlib.h
//#include "addrlib.h"

class AddrObject
{
};

enum AddrElemMode
{
    ADDR_ROUND_BY_HALF,
    ADDR_ROUND_TRUNCATE,
    ADDR_ROUND_DITHER,
    ADDR_UNCOMPRESSED,
    ADDR_EXPANDED,
    ADDR_PACKED_STD,
    ADDR_PACKED_REV,
    ADDR_PACKED_GBGR,
    ADDR_PACKED_BGRG,
    ADDR_PACKED_BC1,
    ADDR_PACKED_BC2,
    ADDR_PACKED_BC3,
    ADDR_PACKED_BC4,
    ADDR_PACKED_BC5
};

enum AddrTileType
{
    ADDR_DISPLAYABLE        = 0,
    ADDR_NON_DISPLAYABLE    = 1,
    ADDR_DEPTH_SAMPLE_ORDER = 1,
    ADDR_ROTATED            = 2, // ???
    ADDR_THICK              = 3
};

class AddrElemLib : public AddrObject
{
public:
    static inline bool IsBlockCompressed(AddrFormat format)
    {
        //return ADDR_FMT_BC1 <= format && format <= ADDR_FMT_BC7;
        return (u32)(format - ADDR_FMT_BC1) <= ADDR_FMT_BC7 - ADDR_FMT_BC1;
    }

private:
    static inline u32 GetBitsPerPixel(
        AddrFormat format,
        AddrElemMode* pElemMode,
        u32* pExpandX,
        u32* pExpandY,
        u32* pUnusedBits = NULL
    );

    static inline void AdjustSurfaceInfo(
        AddrElemMode elemMode,
        u32 expandX,
        u32 expandY,
        u32* pBpp,
        u32* pWidth,
        u32* pHeight
    );

    static inline void RestoreSurfaceInfo(
        AddrElemMode elemMode,
        u32 expandX,
        u32 expandY,
        u32* pBpp,
        u32* pWidth,
        u32* pHeight
    );

    friend class AddrLib;
    friend class R600AddrLib;
};

class AddrLib : public AddrObject
{
private:
    static inline void ComputeMipLevel(
        u32* pWidth,
        u32* pHeight,
        u32* pNumSlices,
        ADDR_SURFACE_FLAGS flags,
        AddrFormat format,
        u32 mipLevel
    );

    static inline ADDR_E_RETURNCODE ComputeSurfaceInfo(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut
    );
    friend ADDR_E_RETURNCODE AddrComputeSurfaceInfo(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut
    );

    static inline ADDR_E_RETURNCODE ComputeSurfaceAddrFromCoord(
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut
    );
    friend ADDR_E_RETURNCODE AddrComputeSurfaceAddrFromCoord(
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut
    );

    static inline u32 ComputeSurfaceThickness(
        AddrTileMode tileMode
    );

    static inline void AdjustPitchAlignment(
        ADDR_SURFACE_FLAGS flags,
        u32* pPitchAlign
    );

    static inline u32 PadDimensions(
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
    );

    static inline u64 ComputeSurfaceAddrFromCoordLinear(
        u32 x,
        u32 y,
        u32 slice,
        u32 sample,
        u32 bpp,
        u32 pitch,
        u32 height,
        u32 numSlices,
        u32* pBitPosition
    );

    static inline u32 ComputePixelIndexWithinMicroTile(
        u32 x,
        u32 y,
        u32 z,
        u32 bpp,
        AddrTileMode tileMode,
        AddrTileType microTileType
    );

    friend class R600AddrLib;
};

class R600AddrLib : public AddrLib
{
private:
    static inline u32 ComputeSurfaceNumSplits(
        AddrTileMode tileMode,
        u32 bpp,
        u32 numSamples
    );

    static inline u32 ComputeBankPipeRotation(
        AddrTileMode tileMode
    );

    static inline AddrTileMode DegradeBankSwapTileMode(
        AddrTileMode baseTileMode
    );

    static /* inline */ forceinline AddrTileMode ComputeSurfaceMipLevelTileMode(
        AddrTileMode baseTileMode,
        u32 bpp,
        u32 mipLevel,
        u32 pitch,
        u32 height,
        u32 numSlices,
        u32 numSamples,
        bool isDepthSampleOrder,
        bool noRecurrence = false
    );

    static inline u32 GetPitchAlignmentLinear(
        u32 bpp
    );

    static inline bool ComputeSurfaceAlignmentsLinear(
        AddrTileMode tileMode,
        u32 bpp,
        ADDR_SURFACE_FLAGS flags,
        u32* pBaseAlign,
        u32* pPitchAlign,
        u32* pHeightAlign
    );

    static inline bool ComputeSurfaceInfoLinear(
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
    );

    static inline u32 GetPitchAlignmentMicroTiled(
        AddrTileMode tileMode,
        u32 bpp,
        ADDR_SURFACE_FLAGS flags,
        u32 numSamples
    );

    static inline bool ComputeSurfaceAlignmentsMicroTiled(
        AddrTileMode tileMode,
        u32 bpp,
        ADDR_SURFACE_FLAGS flags,
        u32 numSamples,
        u32* pBaseAlign,
        u32* pPitchAlign,
        u32* pHeightAlign
    );

    static inline bool ComputeSurfaceInfoMicroTiled(
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
    );

    static inline u32 ComputeSurfaceBankSwapWidth(
        AddrTileMode tileMode,
        u32 bpp,
        u32 numSamples,
        u32 pitch,
        u32* pNumSplits
    );

    static inline bool ComputeSurfaceAlignmentsMacroTiled(
        AddrTileMode tileMode,
        u32 bpp,
        ADDR_SURFACE_FLAGS flags,
        u32 numSamples,
        u32* pBaseAlign,
        u32* pPitchAlign,
        u32* pHeightAlign,
        u32* pMacroWidth,
        u32* pMacroHeight
    );

    static /* inline */ forceinline bool ComputeSurfaceInfoMacroTiled(
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
    );

    static /* inline */ forceinline bool DispatchComputeSurfaceInfo(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut
    );

    static inline u64 ComputeSurfaceAddrFromCoordMicroTiled(
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
    );

    static inline u32 ComputePipeFromCoord(
        u32 x,
        u32 y
    );

    static inline u32 ComputeBankFromCoord(
        u32 x,
        u32 y
    );

    static /* inline */ forceinline u64 ComputeSurfaceAddrFromCoordMacroTiled(
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
    );

    static /* inline */ forceinline u64 DispatchComputeSurfaceAddrFromCoord(
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut
    );

    static /* inline */ forceinline ADDR_E_RETURNCODE HwlComputeSurfaceInfo(
        ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut
    );

    static inline ADDR_E_RETURNCODE HwlComputeSurfaceAddrFromCoord(
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
        ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut
    );

public:
    static inline void ExtractBankPipeSwizzle(
        u32 base256b,
        u32* pBankSwizzle,
        u32* pPipeSwizzle
    )
    {

        u32 numPipes   = 2;
        u32 bankBits   = 2; // Log2(numBanks = 4);
        u32 pipeBits   = 1; // Log2(numPipes = 2);
        u32 groupBytes = 256u;

        *pPipeSwizzle =
            (base256b / (groupBytes >> 8)) & ((1 << pipeBits) - 1);

        *pBankSwizzle =
            (base256b / (groupBytes >> 8) / numPipes) & ((1 << bankBits) - 1);
    }

private:
    static inline bool HwlComputeMipLevel(
        u32* pWidth,
        u32* pHeight,
        u32* pNumSlices,
        ADDR_SURFACE_FLAGS flags,
        AddrFormat format,
        u32 mipLevel
    );

    friend class AddrLib;
};
