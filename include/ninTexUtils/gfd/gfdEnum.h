#ifndef NIN_TEX_UTILS_GFD_ENUM_H_
#define NIN_TEX_UTILS_GFD_ENUM_H_

#include <ninTexUtils/types.h>

typedef enum _GFDGPUVersion
{
    GFD_GPU_VERSION_0    = 0,
    GFD_GPU_VERSION_1    = 1,
    GFD_GPU_VERSION_GPU7 = 2
}
GFDGPUVersion;
static_assert(sizeof(GFDGPUVersion) == 4, "GFDGPUVersion size mismatch");

typedef enum _GFDAlignMode
{
    GFD_ALIGN_MODE_UNDEF     = -1, // For v6.0
    GFD_ALIGN_MODE_DISABLE   =  0,
    GFD_ALIGN_MODE_ENABLE    =  1
}
GFDAlignMode;
static_assert(sizeof(GFDAlignMode) == 4, "GFDAlignMode size mismatch");

typedef enum _GFDBlockTypeV0
{
    // Basic
    GFD_BLOCK_TYPE_V0_INVALID        =  0,
    GFD_BLOCK_TYPE_V0_END            =  1,
    GFD_BLOCK_TYPE_V0_PAD            =  2,
    GFD_BLOCK_TYPE_V0_USR            = 16,

    // GX2 Vertex Shader
    GFD_BLOCK_TYPE_V0_GX2_VS_HEADER  =  3,
    GFD_BLOCK_TYPE_V0_GX2_VS_PROGRAM =  5,

    // GX2 Pixel Shader
    GFD_BLOCK_TYPE_V0_GX2_PS_HEADER  =  6,
    GFD_BLOCK_TYPE_V0_GX2_PS_PROGRAM =  7,

    // GX2 Geometry Shader
    GFD_BLOCK_TYPE_V0_GX2_GS_HEADER  =  8,
    GFD_BLOCK_TYPE_V0_GX2_GS_PROGRAM =  9,

    /*     version-0-specific values     */

    // GX2 Texture
    GFD_BLOCK_TYPE_V0_GX2_TEX_HEADER      = 10,
    GFD_BLOCK_TYPE_V0_GX2_TEX_IMAGE_DATA  = 11,
    GFD_BLOCK_TYPE_V0_GX2_TEX_MIP_DATA    = 12,

    // GX2 Geometry Shader (cont.)
    GFD_BLOCK_TYPE_V0_GX2_GS_COPY_PROGRAM = 13,

    // Reserved
    GFD_BLOCK_TYPE_V0_RESERVED_1          = 14,
    GFD_BLOCK_TYPE_V0_RESERVED_2          = 15
}
GFDBlockTypeV0;
static_assert(sizeof(GFDBlockTypeV0) == 4, "GFDBlockTypeV0 size mismatch");

typedef enum _GFDBlockTypeV1
{
    // Basic
    GFD_BLOCK_TYPE_V1_INVALID        =  0,
    GFD_BLOCK_TYPE_V1_END            =  1,
    GFD_BLOCK_TYPE_V1_PAD            =  2,
    GFD_BLOCK_TYPE_V1_USR            = 16,

    // GX2 Vertex Shader
    GFD_BLOCK_TYPE_V1_GX2_VS_HEADER  =  3,
    GFD_BLOCK_TYPE_V1_GX2_VS_PROGRAM =  5,

    // GX2 Pixel Shader
    GFD_BLOCK_TYPE_V1_GX2_PS_HEADER  =  6,
    GFD_BLOCK_TYPE_V1_GX2_PS_PROGRAM =  7,

    // GX2 Geometry Shader
    GFD_BLOCK_TYPE_V1_GX2_GS_HEADER  =  8,
    GFD_BLOCK_TYPE_V1_GX2_GS_PROGRAM =  9,

    /*     version-1-specific values     */

    // GX2 Geometry Shader (cont.)
    GFD_BLOCK_TYPE_V1_GX2_GS_COPY_PROGRAM = 10,

    // GX2 Texture
    GFD_BLOCK_TYPE_V1_GX2_TEX_HEADER      = 11,
    GFD_BLOCK_TYPE_V1_GX2_TEX_IMAGE_DATA  = 12,
    GFD_BLOCK_TYPE_V1_GX2_TEX_MIP_DATA    = 13,

    // GX2 Compute Shader
    GFD_BLOCK_TYPE_V1_GX2_CS_HEADER       = 14,
    GFD_BLOCK_TYPE_V1_GX2_CS_PROGRAM      = 15,

    /*              Aliases              */

    GFD_BLOCK_TYPE_INVALID        = GFD_BLOCK_TYPE_V1_INVALID,
    GFD_BLOCK_TYPE_END            = GFD_BLOCK_TYPE_V1_END,
    GFD_BLOCK_TYPE_PAD            = GFD_BLOCK_TYPE_V1_PAD,
    GFD_BLOCK_TYPE_USR            = GFD_BLOCK_TYPE_V1_USR,
    GFD_BLOCK_TYPE_GX2_VS_HEADER  = GFD_BLOCK_TYPE_V1_GX2_VS_HEADER,
    GFD_BLOCK_TYPE_GX2_VS_PROGRAM = GFD_BLOCK_TYPE_V1_GX2_VS_PROGRAM,
    GFD_BLOCK_TYPE_GX2_PS_HEADER  = GFD_BLOCK_TYPE_V1_GX2_PS_HEADER,
    GFD_BLOCK_TYPE_GX2_PS_PROGRAM = GFD_BLOCK_TYPE_V1_GX2_PS_PROGRAM,
    GFD_BLOCK_TYPE_GX2_GS_HEADER  = GFD_BLOCK_TYPE_V1_GX2_GS_HEADER,
    GFD_BLOCK_TYPE_GX2_GS_PROGRAM = GFD_BLOCK_TYPE_V1_GX2_GS_PROGRAM
}
GFDBlockTypeV1;
static_assert(sizeof(GFDBlockTypeV1) == 4, "GFDBlockTypeV1 size mismatch");

typedef GFDBlockTypeV1 GFDBlockType;

#endif
