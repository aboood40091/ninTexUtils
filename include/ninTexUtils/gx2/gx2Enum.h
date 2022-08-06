#ifndef NIN_TEX_UTILS_GX2_ENUM_H_
#define NIN_TEX_UTILS_GX2_ENUM_H_

#include <ninTexUtils/gx2/tcl/addrlib.h>

typedef enum _GX2SurfaceDim
{
    GX2_SURFACE_DIM_1D            = 0,
    GX2_SURFACE_DIM_2D            = 1,
    GX2_SURFACE_DIM_3D            = 2,
    GX2_SURFACE_DIM_CUBE          = 3,
    GX2_SURFACE_DIM_1D_ARRAY      = 4,
    GX2_SURFACE_DIM_2D_ARRAY      = 5,
    GX2_SURFACE_DIM_2D_MSAA       = 6,
    GX2_SURFACE_DIM_2D_MSAA_ARRAY = 7
}
GX2SurfaceDim;
static_assert(sizeof(GX2SurfaceDim) == 4, "GX2SurfaceDim size mismatch");

typedef enum _GX2SurfaceFormat
{
    GX2_SURFACE_FORMAT_INVALID       = 0x000,

    GX2_SURFACE_FORMAT_UNORM_RG4     = 0x002,
    GX2_SURFACE_FORMAT_UNORM_RGBA4   = 0x00B,
    GX2_SURFACE_FORMAT_UNORM_R8      = 0x001,
    GX2_SURFACE_FORMAT_UNORM_RG8     = 0x007,
    GX2_SURFACE_FORMAT_UNORM_RGBA8   = 0x01A,
    GX2_SURFACE_FORMAT_UNORM_R16     = 0x005,
    GX2_SURFACE_FORMAT_UNORM_RG16    = 0x00F,
    GX2_SURFACE_FORMAT_UNORM_RGBA16  = 0x01F,
    GX2_SURFACE_FORMAT_UNORM_RGB565  = 0x008,
    GX2_SURFACE_FORMAT_UNORM_RGB5A1  = 0x00A,
    GX2_SURFACE_FORMAT_UNORM_A1BGR5  = 0x00C,
    GX2_SURFACE_FORMAT_UNORM_R24X8   = 0x011,
    GX2_SURFACE_FORMAT_UNORM_A2BGR10 = 0x01B,
    GX2_SURFACE_FORMAT_UNORM_RGB10A2 = 0x019,
    GX2_SURFACE_FORMAT_UNORM_BC1     = 0x031,
    GX2_SURFACE_FORMAT_UNORM_BC2     = 0x032,
    GX2_SURFACE_FORMAT_UNORM_BC3     = 0x033,
    GX2_SURFACE_FORMAT_UNORM_BC4     = 0x034,
    GX2_SURFACE_FORMAT_UNORM_BC5     = 0x035,
    GX2_SURFACE_FORMAT_UNORM_NV12    = 0x081,

    GX2_SURFACE_FORMAT_UINT_R8       = 0x101,
    GX2_SURFACE_FORMAT_UINT_RG8      = 0x107,
    GX2_SURFACE_FORMAT_UINT_RGBA8    = 0x11A,
    GX2_SURFACE_FORMAT_UINT_R16      = 0x105,
    GX2_SURFACE_FORMAT_UINT_RG16     = 0x10F,
    GX2_SURFACE_FORMAT_UINT_RGBA16   = 0x11F,
    GX2_SURFACE_FORMAT_UINT_R32      = 0x10D,
    GX2_SURFACE_FORMAT_UINT_RG32     = 0x11D,
    GX2_SURFACE_FORMAT_UINT_RGBA32   = 0x122,
    GX2_SURFACE_FORMAT_UINT_A2BGR10  = 0x11B,
    GX2_SURFACE_FORMAT_UINT_RGB10A2  = 0x119,
    GX2_SURFACE_FORMAT_UINT_X24G8    = 0x111,
    GX2_SURFACE_FORMAT_UINT_G8X24    = 0x11C,

    GX2_SURFACE_FORMAT_SNORM_R8      = 0x201,
    GX2_SURFACE_FORMAT_SNORM_RG8     = 0x207,
    GX2_SURFACE_FORMAT_SNORM_RGBA8   = 0x21A,
    GX2_SURFACE_FORMAT_SNORM_R16     = 0x205,
    GX2_SURFACE_FORMAT_SNORM_RG16    = 0x20F,
    GX2_SURFACE_FORMAT_SNORM_RGBA16  = 0x21F,
    GX2_SURFACE_FORMAT_SNORM_RGB10A2 = 0x219,
    GX2_SURFACE_FORMAT_SNORM_BC4     = 0x234,
    GX2_SURFACE_FORMAT_SNORM_BC5     = 0x235,

    GX2_SURFACE_FORMAT_SINT_R8       = 0x301,
    GX2_SURFACE_FORMAT_SINT_RG8      = 0x307,
    GX2_SURFACE_FORMAT_SINT_RGBA8    = 0x31A,
    GX2_SURFACE_FORMAT_SINT_R16      = 0x305,
    GX2_SURFACE_FORMAT_SINT_RG16     = 0x30F,
    GX2_SURFACE_FORMAT_SINT_RGBA16   = 0x31F,
    GX2_SURFACE_FORMAT_SINT_R32      = 0x30D,
    GX2_SURFACE_FORMAT_SINT_RG32     = 0x31D,
    GX2_SURFACE_FORMAT_SINT_RGBA32   = 0x322,
    GX2_SURFACE_FORMAT_SINT_RGB10A2  = 0x319,

    GX2_SURFACE_FORMAT_SRGB_RGBA8    = 0x41A,
    GX2_SURFACE_FORMAT_SRGB_BC1      = 0x431,
    GX2_SURFACE_FORMAT_SRGB_BC2      = 0x432,
    GX2_SURFACE_FORMAT_SRGB_BC3      = 0x433,

    GX2_SURFACE_FORMAT_FLOAT_R32     = 0x80E,
    GX2_SURFACE_FORMAT_FLOAT_RG32    = 0x81E,
    GX2_SURFACE_FORMAT_FLOAT_RGBA32  = 0x823,
    GX2_SURFACE_FORMAT_FLOAT_R16     = 0x806,
    GX2_SURFACE_FORMAT_FLOAT_RG16    = 0x810,
    GX2_SURFACE_FORMAT_FLOAT_RGBA16  = 0x820,
    GX2_SURFACE_FORMAT_FLOAT_RG11B10 = 0x816,
    GX2_SURFACE_FORMAT_FLOAT_D24S8   = 0x811,
    GX2_SURFACE_FORMAT_FLOAT_X8X24   = 0x81C
}
GX2SurfaceFormat;
static_assert(sizeof(GX2SurfaceFormat) == 4, "GX2SurfaceFormat size mismatch");

typedef enum _GX2AAMode
{
    GX2_AA_MODE_1X = 0,
    GX2_AA_MODE_2X = 1,
    GX2_AA_MODE_4X = 2,
    GX2_AA_MODE_8X = 3
}
GX2AAMode;
static_assert(sizeof(GX2AAMode) == 4, "GX2AAMode size mismatch");

typedef enum _GX2SurfaceUse
{
    GX2_SURFACE_USE_TEXTURE      = 0x00000001,
    GX2_SURFACE_USE_COLOR_BUFFER = 0x00000002,
    GX2_SURFACE_USE_DEPTH_BUFFER = 0x00000004,
    GX2_SURFACE_USE_SCAN_BUFFER  = 0x00000008,
    GX2_SURFACE_USE_TV           = 0x80000000,

    GX2_SURFACE_USE_COLOR_BUFFER_TEXTURE    = GX2_SURFACE_USE_TEXTURE |
                                              GX2_SURFACE_USE_COLOR_BUFFER,

    GX2_SURFACE_USE_DEPTH_BUFFER_TEXTURE    = GX2_SURFACE_USE_TEXTURE |
                                              GX2_SURFACE_USE_DEPTH_BUFFER,

    GX2_SURFACE_USE_COLOR_BUFFER_TEXTURE_TV = GX2_SURFACE_USE_TEXTURE |
                                              GX2_SURFACE_USE_COLOR_BUFFER |
                                              GX2_SURFACE_USE_TV,

    GX2_SURFACE_USE_COLOR_BUFFER_TV         = GX2_SURFACE_USE_COLOR_BUFFER |
                                              GX2_SURFACE_USE_TV
}
GX2SurfaceUse;
static_assert(sizeof(GX2SurfaceUse) == 4, "GX2SurfaceUse size mismatch");

typedef enum _GX2TileMode
{
    GX2_TILE_MODE_DEFAULT        = 0,
    GX2_TILE_MODE_LINEAR_ALIGNED = 1,
    GX2_TILE_MODE_TILED_1D_THIN1 = 2,
    GX2_TILE_MODE_TILED_1D_THICK = 3,
    GX2_TILE_MODE_TILED_2D_THIN1 = 4,
    GX2_TILE_MODE_TILED_2D_THIN2 = 5,
    GX2_TILE_MODE_TILED_2D_THIN4 = 6,
    GX2_TILE_MODE_TILED_2D_THICK = 7,
    GX2_TILE_MODE_TILED_2B_THIN1 = 8,
    GX2_TILE_MODE_TILED_2B_THIN2 = 9,
    GX2_TILE_MODE_TILED_2B_THIN4 = 10,
    GX2_TILE_MODE_TILED_2B_THICK = 11,
    GX2_TILE_MODE_TILED_3D_THIN1 = 12,
    GX2_TILE_MODE_TILED_3D_THICK = 13,
    GX2_TILE_MODE_TILED_3B_THIN1 = 14,
    GX2_TILE_MODE_TILED_3B_THICK = 15,
    GX2_TILE_MODE_LINEAR_SPECIAL = 16
}
GX2TileMode;
static_assert(sizeof(GX2TileMode) == 4, "GX2TileMode size mismatch");

typedef enum _GX2ShaderMode
{
    GX2_SHADER_MODE_UNIFORM_REGISTERS = 0,
    GX2_SHADER_MODE_UNIFORM_BLOCKS    = 1,
    GX2_SHADER_MODE_GEOMETRY_SHADER   = 2,
    GX2_SHADER_MODE_COMPUTE_SHADER    = 3
}
GX2ShaderMode;
static_assert(sizeof(GX2ShaderMode) == 4, "GX2ShaderMode size mismatch");

typedef enum _GX2VarType
{
    /* GLSL data-types
         GX2 probably(?) does not support double-precision types */
    GX2_VAR_TYPE_VOID    = 0,
    GX2_VAR_TYPE_BOOL    = 1,
    GX2_VAR_TYPE_INT     = 2,
    GX2_VAR_TYPE_UINT    = 3,
    GX2_VAR_TYPE_FLOAT   = 4,
    GX2_VAR_TYPE_DOUBLE  = 5,
    GX2_VAR_TYPE_DVEC2   = 6,
    GX2_VAR_TYPE_DVEC3   = 7,
    GX2_VAR_TYPE_DVEC4   = 8,
    GX2_VAR_TYPE_VEC2    = 9,
    GX2_VAR_TYPE_VEC3    = 10,
    GX2_VAR_TYPE_VEC4    = 11,
    GX2_VAR_TYPE_BVEC2   = 12,
    GX2_VAR_TYPE_BVEC3   = 13,
    GX2_VAR_TYPE_BVEC4   = 14,
    GX2_VAR_TYPE_IVEC2   = 15,
    GX2_VAR_TYPE_IVEC3   = 16,
    GX2_VAR_TYPE_IVEC4   = 17,
    GX2_VAR_TYPE_UVEC2   = 18,
    GX2_VAR_TYPE_UVEC3   = 19,
    GX2_VAR_TYPE_UVEC4   = 20,
    GX2_VAR_TYPE_MAT2    = 21,
    GX2_VAR_TYPE_MAT2X3  = 22,
    GX2_VAR_TYPE_MAT2X4  = 23,
    GX2_VAR_TYPE_MAT3X2  = 24,
    GX2_VAR_TYPE_MAT3    = 25,
    GX2_VAR_TYPE_MAT3X4  = 26,
    GX2_VAR_TYPE_MAT4X2  = 27,
    GX2_VAR_TYPE_MAT4X3  = 28,
    GX2_VAR_TYPE_MAT4    = 29,
    GX2_VAR_TYPE_DMAT2   = 30,
    GX2_VAR_TYPE_DMAT2X3 = 31,
    GX2_VAR_TYPE_DMAT2X4 = 32,
    GX2_VAR_TYPE_DMAT3X2 = 33,
    GX2_VAR_TYPE_DMAT3   = 34,
    GX2_VAR_TYPE_DMAT3X4 = 35,
    GX2_VAR_TYPE_DMAT4X2 = 36,
    GX2_VAR_TYPE_DMAT4X3 = 37,
    GX2_VAR_TYPE_DMAT4   = 38
}
GX2VarType;
static_assert(sizeof(GX2VarType) == 4, "GX2VarType size mismatch");

typedef enum _GX2SamplerType
{
    GX2_SAMPLER_TYPE_2D = 1
    /* ... */
}
GX2SamplerType;
static_assert(sizeof(GX2SamplerType) == 4, "GX2SamplerType size mismatch");

#ifdef __cplusplus
extern "C"
{
#endif

inline AddrFormat GX2SurfaceFormatToAddrFormat(GX2SurfaceFormat format)
{
    /* assert(format != GX2_SURFACE_FORMAT_UNORM_NV12); */
    return (AddrFormat)(format & 0x3F);
}

inline bool GX2SurfaceIsCompressed(GX2SurfaceFormat format)
{
    /* return ADDR_FMT_BC1 <= (format & 0x3F) && (format & 0x3F) <= ADDR_FMT_BC5; */
    return (u32)((format & 0x3F) - ADDR_FMT_BC1) <= ADDR_FMT_BC5 - ADDR_FMT_BC1;
}

inline u8 GX2GetSurfaceFormatBitsPerPixel(GX2SurfaceFormat format)
{
    switch (GX2SurfaceFormatToAddrFormat(format))
    {
    case ADDR_FMT_INVALID:           return   0;
    case ADDR_FMT_8:                 return   8;
    case ADDR_FMT_4_4:               return   8;
    case ADDR_FMT_3_3_2:             return   0;
    case ADDR_FMT_RESERVED_4:        return   0;
    case ADDR_FMT_16:                return  16;
    case ADDR_FMT_16_FLOAT:          return  16;
    case ADDR_FMT_8_8:               return  16;
    case ADDR_FMT_5_6_5:             return  16;
    case ADDR_FMT_6_5_5:             return  16;
    case ADDR_FMT_1_5_5_5:           return  16;
    case ADDR_FMT_4_4_4_4:           return  16;
    case ADDR_FMT_5_5_5_1:           return  16;
    case ADDR_FMT_32:                return  32;
    case ADDR_FMT_32_FLOAT:          return  32;
    case ADDR_FMT_16_16:             return  32;
    case ADDR_FMT_16_16_FLOAT:       return  32;
    case ADDR_FMT_8_24:              return  32;
    case ADDR_FMT_8_24_FLOAT:        return   0;
    case ADDR_FMT_24_8:              return  32;
    case ADDR_FMT_24_8_FLOAT:        return   0;
    case ADDR_FMT_10_11_11:          return   0;
    case ADDR_FMT_10_11_11_FLOAT:    return  32;
    case ADDR_FMT_11_11_10:          return   0;
    case ADDR_FMT_11_11_10_FLOAT:    return   0;
    case ADDR_FMT_2_10_10_10:        return  32;
    case ADDR_FMT_8_8_8_8:           return  32;
    case ADDR_FMT_10_10_10_2:        return  32;
    case ADDR_FMT_X24_8_32_FLOAT:    return  64;
    case ADDR_FMT_32_32:             return  64;
    case ADDR_FMT_32_32_FLOAT:       return  64;
    case ADDR_FMT_16_16_16_16:       return  64;
    case ADDR_FMT_16_16_16_16_FLOAT: return  64;
    case ADDR_FMT_RESERVED_33:       return   0;
    case ADDR_FMT_32_32_32_32:       return 128;
    case ADDR_FMT_32_32_32_32_FLOAT: return 128;
    case ADDR_FMT_RESERVED_36:       return   0;
    case ADDR_FMT_1:                 return   0;
    case ADDR_FMT_1_REVERSED:        return   0;
    case ADDR_FMT_GB_GR:             return  16;
    case ADDR_FMT_BG_RG:             return  16;
    case ADDR_FMT_32_AS_8:           return  32;
    case ADDR_FMT_32_AS_8_8:         return  32;
    case ADDR_FMT_5_9_9_9_SHAREDEXP: return  32;
    case ADDR_FMT_8_8_8:             return   0;
    case ADDR_FMT_16_16_16:          return   0;
    case ADDR_FMT_16_16_16_FLOAT:    return   0;
    case ADDR_FMT_32_32_32:          return  96;
    case ADDR_FMT_32_32_32_FLOAT:    return  96;
    case ADDR_FMT_BC1:               return  64;
    case ADDR_FMT_BC2:               return 128;
    case ADDR_FMT_BC3:               return 128;
    case ADDR_FMT_BC4:               return  64;
    case ADDR_FMT_BC5:               return 128;
    case ADDR_FMT_BC6:               return   0;
    case ADDR_FMT_BC7:               return   0;
    case ADDR_FMT_32_AS_32_32_32_32: return   0;
    case ADDR_FMT_APC3:              return   0;
    case ADDR_FMT_APC4:              return   0;
    case ADDR_FMT_APC5:              return   0;
    case ADDR_FMT_APC6:              return   0;
    case ADDR_FMT_APC7:              return   0;
    case ADDR_FMT_CTX1:              return   0;
    case ADDR_FMT_RESERVED_63:       return   0;
    }

    return 0;
}

inline u32 GX2GetSurfaceFormatBits(GX2SurfaceFormat format)
{
    if (GX2SurfaceIsCompressed(format))
        return GX2GetSurfaceFormatBitsPerPixel(format) >> 4;

    return GX2GetSurfaceFormatBitsPerPixel(format);
}

inline AddrTileMode GX2TileModeToAddrTileMode(GX2TileMode tile_mode)
{
    /* assert(tile_mode != GX2_TILE_MODE_DEFAULT); */
    return (AddrTileMode)(tile_mode & 0xF);
}

inline GX2TileMode AddrTileModeToGX2TileMode(AddrTileMode tile_mode)
{
    /* assert(tile_mode != ADDR_TM_2D_TILED_XTHICK && */
    /*        tile_mode != ADDR_TM_3D_TILED_XTHICK);  */

    if (tile_mode == ADDR_TM_LINEAR_GENERAL)
        return GX2_TILE_MODE_LINEAR_SPECIAL;

    return (GX2TileMode)tile_mode;
}

#ifdef __cplusplus
}
#endif

#endif
