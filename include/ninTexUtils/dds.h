#ifndef NIN_TEX_UTILS_DDS_H_
#define NIN_TEX_UTILS_DDS_H_

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum _DDSFlags
{
    DDS_FLAGS_CAPS          = 0x000001,
    DDS_FLAGS_HEIGHT        = 0x000002,
    DDS_FLAGS_WIDTH         = 0x000004,
    DDS_FLAGS_PITCH         = 0x000008,
    DDS_FLAGS_PIXEL_FORMAT  = 0x001000,
    DDS_FLAGS_MIP_MAP_COUNT = 0x020000,
    DDS_FLAGS_LINEAR_SIZE   = 0x080000,
    DDS_FLAGS_DEPTH         = 0x800000
}
DDSFlags;
static_assert(sizeof(DDSFlags) == 4, "DDSFlags size mismatch");

typedef enum _DDSPixelFormatFlags
{
    DDS_PIXEL_FORMAT_FLAGS_ALPHA_PIXELS = 0x00001,
    DDS_PIXEL_FORMAT_FLAGS_ALPHA        = 0x00002,
    DDS_PIXEL_FORMAT_FLAGS_FOUR_CC      = 0x00004,
    DDS_PIXEL_FORMAT_FLAGS_RGB          = 0x00040,
    DDS_PIXEL_FORMAT_FLAGS_YUV          = 0x00200,
    DDS_PIXEL_FORMAT_FLAGS_LUMINANCE    = 0x20000
}
DDSPixelFormatFlags;
static_assert(sizeof(DDSPixelFormatFlags) == 4, "DDSPixelFormatFlags size mismatch");

typedef struct _DDSPixelFormat
{
    u32 size;
    DDSPixelFormatFlags flags;
    char fourCC[4];
    u32 rgbBitCount;
    u32 rBitMask;
    u32 gBitMask;
    u32 bBitMask;
    u32 aBitMask;
}
DDSPixelFormat;
static_assert(sizeof(DDSPixelFormat) == 0x20, "DDSPixelFormat size mismatch");

typedef enum _DDSCaps
{
    DDS_CAPS_COMPLEX = 0x000008,
    DDS_CAPS_TEXTURE = 0x001000,
    DDS_CAPS_MIP_MAP = 0x400000
}
DDSCaps;
static_assert(sizeof(DDSCaps) == 4, "DDSCaps size mismatch");

typedef enum _DDSCaps2
{
    DDS_CAPS2_CUBE_MAP            = 0x000200,
    DDS_CAPS2_CUBE_MAP_POSITIVE_X = 0x000400,
    DDS_CAPS2_CUBE_MAP_NEGATIVE_X = 0x000800,
    DDS_CAPS2_CUBE_MAP_POSITIVE_Y = 0x001000,
    DDS_CAPS2_CUBE_MAP_NEGATIVE_Y = 0x002000,
    DDS_CAPS2_CUBE_MAP_POSITIVE_Z = 0x004000,
    DDS_CAPS2_CUBE_MAP_NEGATIVE_Z = 0x008000,
    DDS_CAPS2_VOLUME              = 0x200000
}
DDSCaps2;
static_assert(sizeof(DDSCaps2) == 4, "DDSCaps2 size mismatch");

typedef struct _DDSHeader
{
    char magic[4];
    u32 size;
    DDSFlags flags;
    u32 height;
    u32 width;
    u32 pitchOrLinearSize;
    u32 depth;
    u32 mipMapCount;
    u8 reserved1[44];
    DDSPixelFormat pixelFormat;
    DDSCaps caps;
    DDSCaps2 caps2;
    u8 reserved2[12];
}
DDSHeader;
static_assert(sizeof(DDSHeader) == 0x80, "DDSHeader size mismatch");

/* Get offset to image data relative to header data */
inline u32 DDSGetImageDataOffset(const DDSHeader* header)
{
    /* TODO: DX10 is not supported */
    return sizeof(DDSHeader);
}

/* Read dds file contents into buffers provided by user */
bool DDSReadFile(const void* file, DDSHeader* out_header);

/* Read dds file contents in-place
   Returns same address as "file" on success and NULL on failure */
inline DDSHeader* DDSReadFilePtr(void* file)
{
    DDSHeader* header = (DDSHeader*)file;
    if (!DDSReadFile(file, header))
        return NULL;

    return header;
}

#ifdef __cplusplus
}
#endif

#endif
