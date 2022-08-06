#ifndef NIN_TEX_UTILS_GFD_STRUCT_H_
#define NIN_TEX_UTILS_GFD_STRUCT_H_

#include "gfdEnum.h"

typedef struct _GFDHeader
{
    u32 magic;
    u32 size;
    u32 majorVersion;
    u32 minorVersion;
    GFDGPUVersion gpuVersion;
    union
    {
        // v6.0
        u32 reserved[3];

        // Later versions
        struct
        {
            GFDAlignMode alignMode;
            u32 reserved1;
            u32 reserved2;
        };
    };
}
GFDHeader;
static_assert(sizeof(GFDHeader) == 0x20, "GFDHeader size mismatch");

typedef struct _GFDBlockHeader
{
    u32 magic;
    u32 size;
    u32 majorVersion;
    u32 minorVersion;
    union
    {
        GFDBlockTypeV0 typeV0;
        GFDBlockTypeV1 typeV1;
        GFDBlockType   type;
    };
    u32 dataSize;
    u32 reserved[2];
}
GFDBlockHeader;
static_assert(sizeof(GFDBlockHeader) == 0x20, "GFDBlockHeader size mismatch");

#ifdef __cplusplus
extern "C"
{
#endif

void LoadGFDHeader(
    const void* data,
    GFDHeader*  header,
#ifdef __cplusplus
    bool        serialized  = true,
    bool        isBigEndian = true
#else
    bool        serialized,
    bool        isBigEndian
#endif
);

inline void SaveGFDHeader(
    void* data,
    const GFDHeader* header,
#ifdef __cplusplus
    bool        serialized  = true,
    bool        isBigEndian = true
#else
    bool        serialized,
    bool        isBigEndian
#endif
)
{
    if (serialized && header && header->majorVersion == 6 && header->minorVersion == 0)
    {
        GFDAlignMode alignMode = header->alignMode;

        GFDHeader* header_cc = (GFDHeader*)header;
        header_cc->alignMode = (GFDAlignMode)0;

        LoadGFDHeader(header, (GFDHeader*)data, serialized, isBigEndian);

        if (header != data)
            header_cc->alignMode = alignMode;
    }
    else
    {
        LoadGFDHeader(header, (GFDHeader*)data, serialized, isBigEndian);
    }
}

void LoadGFDBlockHeader(
    const void* data,
    GFDBlockHeader* block,
#ifdef __cplusplus
    bool        serialized  = true,
    bool        isBigEndian = true
#else
    bool        serialized,
    bool        isBigEndian
#endif
);

inline void SaveGFDBlockHeader(
    void* data,
    const GFDBlockHeader* block,
#ifdef __cplusplus
    bool        serialized  = true,
    bool        isBigEndian = true
#else
    bool        serialized,
    bool        isBigEndian
#endif
)
{
    LoadGFDBlockHeader(block, (GFDBlockHeader*)data, serialized, isBigEndian);
}

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include "gfdFile.hpp"
#endif

#endif
