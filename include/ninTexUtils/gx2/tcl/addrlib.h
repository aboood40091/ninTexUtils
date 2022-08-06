#ifndef NIN_TEX_UTILS_ADDR_LIB_H_
#define NIN_TEX_UTILS_ADDR_LIB_H_

#include <ninTexUtils/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum _ADDR_E_RETURNCODE
{
    ADDR_OK    = 0,
    ADDR_ERROR = 1,
    ADDR_OUTOFMEMORY,
    ADDR_INVALIDPARAMS,
    ADDR_NOTSUPPORTED,
    ADDR_NOTIMPLEMENTED,
    ADDR_PARAMSIZEMISMATCH,
    ADDR_INVALIDGBREGVALUES,

}
ADDR_E_RETURNCODE;

typedef enum _AddrTileMode
{
    ADDR_TM_LINEAR_GENERAL = 0,
    ADDR_TM_LINEAR_ALIGNED = 1,
    ADDR_TM_1D_TILED_THIN1 = 2,
    ADDR_TM_1D_TILED_THICK = 3,
    ADDR_TM_2D_TILED_THIN1 = 4,
    ADDR_TM_2D_TILED_THIN2 = 5,
    ADDR_TM_2D_TILED_THIN4 = 6,
    ADDR_TM_2D_TILED_THICK = 7,
    ADDR_TM_2B_TILED_THIN1 = 8,
    ADDR_TM_2B_TILED_THIN2 = 9,
    ADDR_TM_2B_TILED_THIN4 = 10,
    ADDR_TM_2B_TILED_THICK = 11,
    ADDR_TM_3D_TILED_THIN1 = 12,
    ADDR_TM_3D_TILED_THICK = 13,
    ADDR_TM_3B_TILED_THIN1 = 14,
    ADDR_TM_3B_TILED_THICK = 15,

    /* Not(?) supported by GX2 */
    ADDR_TM_2D_TILED_XTHICK = 16,
    ADDR_TM_3D_TILED_XTHICK = 17
}
AddrTileMode;
static_assert(sizeof(AddrTileMode) == 4, "AddrTileMode size mismatch");

typedef enum _AddrFormat
{
    ADDR_FMT_INVALID           = 0x00,
    ADDR_FMT_8                 = 0x01,
    ADDR_FMT_4_4               = 0x02,
    ADDR_FMT_3_3_2             = 0x03, /* Not(?) supported by GX2 */
    ADDR_FMT_RESERVED_4        = 0x04, /* Not(?) supported by GX2 */
    ADDR_FMT_16                = 0x05,
    ADDR_FMT_16_FLOAT          = 0x06,
    ADDR_FMT_8_8               = 0x07,
    ADDR_FMT_5_6_5             = 0x08,
    ADDR_FMT_6_5_5             = 0x09, /* Not(?) supported by GX2 */
    ADDR_FMT_1_5_5_5           = 0x0A,
    ADDR_FMT_4_4_4_4           = 0x0B,
    ADDR_FMT_5_5_5_1           = 0x0C,
    ADDR_FMT_32                = 0x0D,
    ADDR_FMT_32_FLOAT          = 0x0E,
    ADDR_FMT_16_16             = 0x0F,
    ADDR_FMT_16_16_FLOAT       = 0x10,
    ADDR_FMT_8_24              = 0x11,
    ADDR_FMT_8_24_FLOAT        = 0x12, /* GX2 incorrectly(?) uses 0x11 */
    ADDR_FMT_24_8              = 0x13, /* Not(?) supported by GX2 */
    ADDR_FMT_24_8_FLOAT        = 0x14, /* Not(?) supported by GX2 */
    ADDR_FMT_10_11_11          = 0x15, /* Not(?) supported by GX2 */
    ADDR_FMT_10_11_11_FLOAT    = 0x16,
    ADDR_FMT_11_11_10          = 0x17, /* Not(?) supported by GX2 */
    ADDR_FMT_11_11_10_FLOAT    = 0x18, /* Not(?) supported by GX2 */
    ADDR_FMT_2_10_10_10        = 0x19,
    ADDR_FMT_8_8_8_8           = 0x1A,
    ADDR_FMT_10_10_10_2        = 0x1B,
    ADDR_FMT_X24_8_32_FLOAT    = 0x1C,
    ADDR_FMT_32_32             = 0x1D,
    ADDR_FMT_32_32_FLOAT       = 0x1E,
    ADDR_FMT_16_16_16_16       = 0x1F,
    ADDR_FMT_16_16_16_16_FLOAT = 0x20,
    ADDR_FMT_RESERVED_33       = 0x21, /* Not(?) supported by GX2 */
    ADDR_FMT_32_32_32_32       = 0x22,
    ADDR_FMT_32_32_32_32_FLOAT = 0x23,
    ADDR_FMT_RESERVED_36       = 0x24, /* Not(?) supported by GX2 */
    ADDR_FMT_1                 = 0x25, /* Not(?) supported by GX2 */
    ADDR_FMT_1_REVERSED        = 0x26, /* Not(?) supported by GX2 */
    ADDR_FMT_GB_GR             = 0x27, /* Not(?) supported by GX2 */
    ADDR_FMT_BG_RG             = 0x28, /* Not(?) supported by GX2 */
    ADDR_FMT_32_AS_8           = 0x29, /* Not(?) supported by GX2 */
    ADDR_FMT_32_AS_8_8         = 0x2A, /* Not(?) supported by GX2 */
    ADDR_FMT_5_9_9_9_SHAREDEXP = 0x2B, /* Not(?) supported by GX2 */
    ADDR_FMT_8_8_8             = 0x2C, /* Not(?) supported by GX2 */
    ADDR_FMT_16_16_16          = 0x2D, /* Not(?) supported by GX2 */
    ADDR_FMT_16_16_16_FLOAT    = 0x2E, /* Not(?) supported by GX2 */
    ADDR_FMT_32_32_32          = 0x2F, /* Not(?) supported by GX2 */
    ADDR_FMT_32_32_32_FLOAT    = 0x30, /* Not(?) supported by GX2 */
    ADDR_FMT_BC1               = 0x31,
    ADDR_FMT_BC2               = 0x32,
    ADDR_FMT_BC3               = 0x33,
    ADDR_FMT_BC4               = 0x34,
    ADDR_FMT_BC5               = 0x35,
    ADDR_FMT_BC6               = 0x36, /* Not(?) supported by GX2 */
    ADDR_FMT_BC7               = 0x37, /* Not(?) supported by GX2 */
    ADDR_FMT_32_AS_32_32_32_32 = 0x38, /* Not(?) supported by GX2 */
    ADDR_FMT_APC3              = 0x39, /* Not(?) supported by GX2 */
    ADDR_FMT_APC4              = 0x3A, /* Not(?) supported by GX2 */
    ADDR_FMT_APC5              = 0x3B, /* Not(?) supported by GX2 */
    ADDR_FMT_APC6              = 0x3C, /* Not(?) supported by GX2 */
    ADDR_FMT_APC7              = 0x3D, /* Not(?) supported by GX2 */
    ADDR_FMT_CTX1              = 0x3E, /* Not(?) supported by GX2 */
    ADDR_FMT_RESERVED_63       = 0x3F  /* Not(?) supported by GX2 */
}
AddrFormat;
static_assert(sizeof(AddrFormat) == 4, "AddrFormat size mismatch");

typedef union _ADDR_SURFACE_FLAGS
{
    //struct
    //{
    //};
    u32 value;
}
ADDR_SURFACE_FLAGS;

typedef struct _ADDR_COMPUTE_SURFACE_INFO_INPUT
{
    u32 size;
    AddrTileMode tileMode;
    AddrFormat format;
    u32 bpp;
    u32 numSamples;
    u32 width;
    u32 height;
    u32 numSlices;
    u32 slice;
    u32 mipLevel;
    ADDR_SURFACE_FLAGS flags;
    u32 numFrags;
}
ADDR_COMPUTE_SURFACE_INFO_INPUT;

typedef struct _ADDR_COMPUTE_SURFACE_INFO_OUTPUT
{
    u32 size;
    u32 pitch;
    u32 height;
    u32 depth;
    u64 surfSize;
    AddrTileMode tileMode;
    u32 baseAlign;
    u32 pitchAlign;
    u32 heightAlign;
    u32 depthAlign;
    u32 bpp;
    u32 pixelPitch;
    u32 pixelHeight;
    u32 pixelBits;
    u64 sliceSize;
    u32 pitchTileMax;
    u32 heightTileMax;
    u32 sliceTileMax;
}
ADDR_COMPUTE_SURFACE_INFO_OUTPUT;

typedef struct _ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT
{
    u32 size;
    u32 x;
    u32 y;
    u32 slice;
    u32 sample;
    u32 bpp;
    u32 pitch;
    u32 height;
    u32 numSlices;
    u32 numSamples;
    AddrTileMode tileMode;
    bool isDepth;
    u32 tileBase;
    u32 compBits;
    u32 pipeSwizzle;
    u32 bankSwizzle;
}
ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT;

typedef struct _ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT
{
    u32 size;
    u64 addr;
    u32 bitPosition;
}
ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT;

ADDR_E_RETURNCODE AddrComputeSurfaceInfo(
    ADDR_COMPUTE_SURFACE_INFO_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_INFO_OUTPUT* pOut
);

ADDR_E_RETURNCODE AddrComputeSurfaceAddrFromCoord(
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_INPUT* pIn,
    ADDR_COMPUTE_SURFACE_ADDRFROMCOORD_OUTPUT* pOut
);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "addrlib.hpp"
#endif

#endif
