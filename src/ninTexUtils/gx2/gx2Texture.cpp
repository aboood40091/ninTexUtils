#include <ninTexUtils/gx2/gx2Texture.h>
#include <ninTexUtils/dds.h>

#include <algorithm>
#include <array>
#include <cassert>
#include <cstring>
#include <iostream>
#include <unordered_map>

#ifndef GX2_TEXTURE_PERF_MODULATION
//#define GX2_TEXTURE_PERF_MODULATION   0
#define   GX2_TEXTURE_PERF_MODULATION   7   // It was changed from 0 to 7 in some version of CafeSDK post 2.08.03
#endif // GX2_TEXTURE_PERF_MODULATION

extern "C"
{

void GX2InitTextureRegs(GX2Texture* texture, bool gfd_v7)
{
    assert(texture->surface.use & GX2_SURFACE_USE_TEXTURE);
    assert(texture->surface.tileMode != GX2_TILE_MODE_DEFAULT);
    assert(texture->surface.tileMode != GX2_TILE_MODE_LINEAR_SPECIAL);
    assert(texture->surface.alignment != 0);
    assert(texture->surface.pitch != 0);

    if (texture->viewNumMips     == 0) texture->viewNumMips     = 1;
    if (texture->viewNumSlices   == 0) texture->viewNumSlices   = 1;
    if (texture->surface.height  == 0) texture->surface.height  = 1;
    if (texture->surface.depth   == 0) texture->surface.depth   = 1;
    if (texture->surface.numMips == 0) texture->surface.numMips = 1;

    assert(texture->viewFirstMip == 0 || texture->viewFirstMip < texture->surface.numMips);
    assert(texture->viewFirstMip + texture->viewNumMips <= texture->surface.numMips);

    assert(texture->viewFirstSlice == 0 || texture->viewFirstSlice < texture->surface.depth);
    assert(texture->viewFirstSlice + texture->viewNumSlices <= texture->surface.depth);

    assert(texture->surface.aa == GX2_AA_MODE_1X || texture->surface.numMips == 1);
    assert(texture->surface.aa == GX2_AA_MODE_1X || texture->surface.dim == GX2_SURFACE_DIM_2D_MSAA || texture->surface.dim == GX2_SURFACE_DIM_2D_MSAA_ARRAY);

    u32 _reg0 = texture->surface.dim & 7;
    _reg0 |= GX2TileModeToAddrTileMode(texture->surface.tileMode) << 3;

    AddrFormat hwFormat = GX2SurfaceFormatToAddrFormat(texture->surface.format);

    u32 tileType = 0;
    if (texture->surface.use & GX2_SURFACE_USE_DEPTH_BUFFER)
    {
        assert(texture->surface.format != GX2_SURFACE_FORMAT_UNORM_D24S8);
        if (gfd_v7) { assert(texture->surface.format != GX2_SURFACE_FORMAT_FLOAT_D32_UINT_S8X24); }
        assert(hwFormat == ADDR_FMT_16 || hwFormat == ADDR_FMT_32_FLOAT || hwFormat == ADDR_FMT_8_24 || hwFormat == ADDR_FMT_X24_8_32_FLOAT);

        tileType = 1;
    }
    _reg0 |= tileType << 7;

    _reg0 |= ((texture->surface.pitch * (AddrFormatIsCompressed(hwFormat) ? 4 : 1) / 8 - 1) & 0x7FF) << 8;
    _reg0 |= (texture->surface.width - 1) << 19;

    texture->_regs[0] = _reg0;

    u32 _reg1 = (texture->surface.height - 1) & 0x1FFF;

    u16 depth;
    switch (texture->surface.dim)
    {
    case GX2_SURFACE_DIM_3D:
    case GX2_SURFACE_DIM_1D_ARRAY:
    case GX2_SURFACE_DIM_2D_ARRAY:
    case GX2_SURFACE_DIM_2D_MSAA_ARRAY:
        depth = texture->surface.depth - 1;
        break;
    case GX2_SURFACE_DIM_CUBE:
        assert((texture->surface.depth % 6) == 0);
        depth = texture->surface.depth / 6 - 1;
        break;
    default:
        depth = 0;
        break;
    }
    _reg1 |= (depth & 0x1FFF) << 13;

    _reg1 |= hwFormat << 26;

    texture->_regs[1] = _reg1;

    u32 formatComp;
    if (texture->surface.format & 0x200)
        formatComp = 1; // SIGNED
    else
        formatComp = 0; // UNSIGNED

    u32 _reg2 = formatComp |
                formatComp << 2 |
                formatComp << 4 |
                formatComp << 6;

    u32 numFormat;
    if (texture->surface.format & 0x800)
        numFormat = 2; // FLOAT
    else if (texture->surface.format & 0x100)
        numFormat = 1; // INT
    else
        numFormat = 0; // NORM

    _reg2 |= numFormat << 8;

    u32 surfMode;
    if (gfd_v7)
        surfMode = 0;
    else
    {
        if (texture->surface.format & 0x800)
            surfMode = 0;
        else
            surfMode = 1;
    }

    _reg2 |= surfMode << 10;

    u32 forceDegamma;
    if (texture->surface.format & 0x400)
        forceDegamma = 1;
    else
        forceDegamma = 0;

    _reg2 |= forceDegamma << 11;

    u32 endian = 0; // Format-dependent, but it's actually just 0 for all formats which GX2 supports
    _reg2 |= endian << 12;

    u32 requestSize = 2;
    _reg2 |= requestSize << 14;

    _reg2 |= ((texture->compSel >> 24) & 7) << 16 |
             ((texture->compSel >> 16) & 7) << 19 |
             ((texture->compSel >>  8) & 7) << 22 |
             ( texture->compSel        & 7) << 25;

    _reg2 |= texture->viewFirstMip << 28;

    texture->_regs[2] = _reg2;

    u32 lastLevel;
    switch (texture->surface.aa)
    {
    case GX2_AA_MODE_2X:
        lastLevel = 1; // log2(2)
        break;
    case GX2_AA_MODE_4X:
        lastLevel = 2; // log2(4)
        break;
    case GX2_AA_MODE_8X:
        lastLevel = 3; // log2(8)
        break;
    default:
        lastLevel = texture->viewFirstMip + texture->viewNumMips - 1;
        break;
    }

    u32 _reg3 = lastLevel & 0xF;

    _reg3 |= (texture->viewFirstSlice & 0x1FFF) << 4;

    u32 lastArray = texture->viewFirstSlice + texture->viewNumSlices - 1;
    _reg3 |= (lastArray & 0x1FFF) << 17;

    u32 yuvConv = 0;
    if (gfd_v7 && texture->surface.dim == GX2_SURFACE_DIM_CUBE && (_reg1 >> 13 & 0x1FFF) != 0)  // _reg1 >> 13 & 0x1FFF -> depth
        yuvConv = 1;
    _reg3 |= yuvConv << 30;

    texture->_regs[3] = _reg3;

    u32 maxAnisoRatio = 4;
    u32 _reg4 = maxAnisoRatio << 2;

    u32 perfModulation = 0;
    if (gfd_v7)
        perfModulation = GX2_TEXTURE_PERF_MODULATION;
    _reg4 |= perfModulation << 5;

    u32 type = 2; // Valid texture
    _reg4 |= type << 30;

    texture->_regs[4] = _reg4;
}

void GX2TextureVerifyForSerialization(const GX2Texture* tex)
{
    assert(tex->surface.aa == GX2_AA_MODE_1X);
    assert(tex->surface.use & GX2_SURFACE_USE_TEXTURE);

    const u32 numMips = std::max(tex->surface.numMips, 1u);
    const u32 depth = std::max(tex->surface.depth, 1u);
    const u32 viewNumMips = std::max(tex->viewNumMips, 1u);
    const u32 viewNumSlices = std::max(tex->viewNumSlices, 1u);

    assert(tex->viewFirstMip == 0 || tex->viewFirstMip < numMips);
    assert(tex->viewFirstMip + viewNumMips <= numMips);

    assert(tex->viewFirstSlice == 0 || tex->viewFirstSlice < depth);
    assert(tex->viewFirstSlice + viewNumSlices <= depth);
}

void LoadGX2Texture(const void* data, GX2Texture* tex, bool serialized, bool isBigEndian)
{
    const GX2Texture* src = (const GX2Texture*)data;
    GX2Texture* dst = tex;

    assert(src != NULL);
    assert(dst != NULL);

    LoadGX2Surface(&src->surface, &dst->surface, serialized, isBigEndian);

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        dst->viewFirstMip   = __builtin_bswap32(src->viewFirstMip);
        dst->viewNumMips    = __builtin_bswap32(src->viewNumMips);
        dst->viewFirstSlice = __builtin_bswap32(src->viewFirstSlice);
        dst->viewNumSlices  = __builtin_bswap32(src->viewNumSlices);
        dst->compSel        = __builtin_bswap32(src->compSel);
        dst->_regs[0]       = __builtin_bswap32(src->_regs[0]);
        dst->_regs[1]       = __builtin_bswap32(src->_regs[1]);
        dst->_regs[2]       = __builtin_bswap32(src->_regs[2]);
        dst->_regs[3]       = __builtin_bswap32(src->_regs[3]);
        dst->_regs[4]       = __builtin_bswap32(src->_regs[4]);
    }
    else if (src != dst)
    {
        std::memmove(&dst->viewFirstMip, &src->viewFirstMip, sizeof(GX2Texture) - sizeof(GX2Surface));
    }

    if (serialized)
    {
        GX2TextureVerifyForSerialization(dst);

        dst->viewNumMips = std::max(dst->viewNumMips, 1u);
        dst->viewNumSlices = std::max(dst->viewNumSlices, 1u);
    }
}

static const char* const comp_sel_str[6] = {
    "Red",   // 0
    "Green", // 1
    "Blue",  // 2
    "Alpha", // 3
    "Zero",  // 4
    "One"    // 5
};

void GX2TexturePrintInfo(const GX2Texture* tex)
{
    GX2SurfacePrintInfo(&tex->surface);

    const u32& compSel = tex->compSel;

    std::cout << std::endl;
    std::cout << "// ----- GX2 Component Selectors ----- " << std::endl;
    std::cout << "  Red Channel     = " << comp_sel_str[compSel >> 24 & 0xFF] << std::endl;
    std::cout << "  Green Channel   = " << comp_sel_str[compSel >> 16 & 0xFF] << std::endl;
    std::cout << "  Blue Channel    = " << comp_sel_str[compSel >>  8 & 0xFF] << std::endl;
    std::cout << "  Alpha Channel   = " << comp_sel_str[compSel >>  0 & 0xFF] << std::endl;
}

void GX2TextureFromLinear2D(GX2Texture* texture, u32 width, u32 height, u32 numMips, GX2SurfaceFormat format, u32 compSel, const u8* imagePtr, size_t imageSize, GX2TileMode tileMode, u32 swizzle, const u8* mipPtr, size_t mipSize, bool gfd_v7)
{
    // Create a new GX2Surface to store the untiled texture
    GX2Surface linear_surface;
    linear_surface.dim = GX2_SURFACE_DIM_2D;
    linear_surface.width = width;
    linear_surface.height = height;
    linear_surface.depth = 1;
    linear_surface.numMips = numMips;
    linear_surface.format = format;
    linear_surface.aa = GX2_AA_MODE_1X;
    linear_surface.use = GX2_SURFACE_USE_TEXTURE;
    linear_surface.tileMode = GX2_TILE_MODE_LINEAR_SPECIAL;
    linear_surface.swizzle = 0;

    GX2CalcSurfaceSizeAndAlignment(&linear_surface);

    // Validate and set the image data
    assert(imageSize >= linear_surface.imageSize);
    linear_surface.imagePtr.set(const_cast<u8*>(imagePtr));

    // Validate and set the mip data
    if (numMips > 1)
    {
        assert(mipSize >= linear_surface.mipSize);
        linear_surface.mipPtr.set(const_cast<u8*>(mipPtr));
    }
    else
    {
        linear_surface.mipPtr.set(nullptr);
    }

    // Set up GX2Texture for the tiled texture
    std::memset(texture, 0, sizeof(GX2Texture));
    texture->surface.dim = GX2_SURFACE_DIM_2D;
    texture->surface.width = width;
    texture->surface.height = height;
    texture->surface.depth = 1;
    texture->surface.numMips = numMips;
    texture->surface.format = format;
    texture->surface.aa = GX2_AA_MODE_1X;
    texture->surface.use = GX2_SURFACE_USE_TEXTURE;
    texture->surface.tileMode = tileMode;
    texture->surface.swizzle = swizzle << 8;

    GX2CalcSurfaceSizeAndAlignment(&texture->surface);

    texture->viewFirstMip = 0;
    texture->viewNumMips = numMips;
    texture->viewFirstSlice = 0;
    texture->viewNumSlices = 1;
    texture->compSel = compSel;

    GX2InitTextureRegs(texture, gfd_v7);

    texture->surface.imagePtr.set((u8*)std::malloc(texture->surface.imageSize));
    if (numMips > 1)
        texture->surface.mipPtr.set((u8*)std::malloc(texture->surface.mipSize));
    else
        texture->surface.mipPtr.set(nullptr);

    // Tile our texture
    GX2CopySurface(&linear_surface, 0, 0, &texture->surface, 0, 0);
    for (u32 i = 1; i < numMips; i++)
        GX2CopySurface(&linear_surface, i, 0, &texture->surface, i, 0);
}

static const std::unordered_map<std::string, const u32> fourCCs_import {
    { "DXT1", GX2_SURFACE_FORMAT_UNORM_BC1 << 8 |  8 },
    { "DXT2", GX2_SURFACE_FORMAT_UNORM_BC2 << 8 | 16 },
    { "DXT3", GX2_SURFACE_FORMAT_UNORM_BC2 << 8 | 16 },
    { "DXT4", GX2_SURFACE_FORMAT_UNORM_BC3 << 8 | 16 },
    { "DXT5", GX2_SURFACE_FORMAT_UNORM_BC3 << 8 | 16 },
    { "ATI1", GX2_SURFACE_FORMAT_UNORM_BC4 << 8 |  8 },
    { "BC4U", GX2_SURFACE_FORMAT_UNORM_BC4 << 8 |  8 },
    { "BC4S", GX2_SURFACE_FORMAT_SNORM_BC4 << 8 |  8 },
    { "ATI2", GX2_SURFACE_FORMAT_UNORM_BC5 << 8 | 16 },
    { "BC5U", GX2_SURFACE_FORMAT_UNORM_BC5 << 8 | 16 },
    { "BC5S", GX2_SURFACE_FORMAT_SNORM_BC5 << 8 | 16 }
};

static const std::unordered_map< u32, const std::unordered_map< u32, const std::array<u32, 4> > > validComps_import {
    {  8, { { GX2_SURFACE_FORMAT_UNORM_R8,      { 0x000000ff,          0,          0,          0 } },
            { GX2_SURFACE_FORMAT_UNORM_RG4,     { 0x0000000f, 0x000000f0,          0,          0 } } } },
    { 16, { { GX2_SURFACE_FORMAT_UNORM_RG8,     { 0x000000ff, 0x0000ff00,          0,          0 } },
            { GX2_SURFACE_FORMAT_UNORM_RGB565,  { 0x0000001f, 0x000007e0, 0x0000f800,          0 } },
            { GX2_SURFACE_FORMAT_UNORM_RGB5A1,  { 0x0000001f, 0x000003e0, 0x00007c00, 0x00008000 } },
            { GX2_SURFACE_FORMAT_UNORM_RGBA4,   { 0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000 } } } },
    { 32, { { GX2_SURFACE_FORMAT_UNORM_RGB10A2, { 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 } },
            { GX2_SURFACE_FORMAT_UNORM_RGBA8,   { 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 } } } }
};

static inline bool FindMask(u32 mask, const std::array<u32, 4>& masks, u8* pComp)
{
    u32 i = 0;
    for (u32 mask_cmp : masks)
    {
        if (mask_cmp == 0)
            break;

        if (mask_cmp == mask)
        {
            *pComp = i;
            return true;
        }
        i++;
    }
    return false;
}

void GX2TextureFromDDS(GX2Texture* texture, const u8* file, size_t fileSize, GX2TileMode tileMode, u32 swizzle, bool SRGB, u32 compSelIdx, bool gfd_v7, bool printInfo)
{
    // Parse input
    DDSHeader header;
    bool success = DDSReadFile(file, &header);
    assert(success);

    assert(header.depth <= 1 && !(header.caps2 & DDS_CAPS2_VOLUME) && "3D textures are not supported!");
    assert(!(header.caps2 & (DDS_CAPS2_CUBE_MAP |
                             DDS_CAPS2_CUBE_MAP_POSITIVE_X |
                             DDS_CAPS2_CUBE_MAP_NEGATIVE_X |
                             DDS_CAPS2_CUBE_MAP_POSITIVE_Y |
                             DDS_CAPS2_CUBE_MAP_NEGATIVE_Y |
                             DDS_CAPS2_CUBE_MAP_POSITIVE_Z |
                             DDS_CAPS2_CUBE_MAP_NEGATIVE_Z)) && "Cube Maps are not supported!");

    // Make sure YUV is not being used
    assert(!(header.pixelFormat.flags & DDS_PIXEL_FORMAT_FLAGS_YUV) && "YUV color space is not supported!");

    const u32 width = header.width;
    const u32 height = header.height;
    const u32 numMips = header.mipMapCount;
    std::array<u8, 6> compSelArr;
    GX2SurfaceFormat format;
    u32 imageSize;

    // Treat uncompressed formats differently
    if (!(header.pixelFormat.flags & DDS_PIXEL_FORMAT_FLAGS_FOUR_CC))
    {
        // Get and validate the bits-per-pixel
        const u32 bitsPerPixel = header.pixelFormat.rgbBitCount;
        const auto& it_bpp_validComps = validComps_import.find(bitsPerPixel);
        if (it_bpp_validComps == validComps_import.end())
        {
            std::cerr << "Unrecognized number of bits per pixel: " << bitsPerPixel << std::endl;
            assert(false);
        }

        // Get the RGBA masks
        const u32 rMask = header.pixelFormat.rBitMask;
        const u32 gMask = header.pixelFormat.gBitMask;
        const u32 bMask = header.pixelFormat.bBitMask;
        const u32 aMask = header.pixelFormat.aBitMask;

        // Pixel format flags
        const bool alphaOnly = header.pixelFormat.flags & DDS_PIXEL_FORMAT_FLAGS_ALPHA;
        const bool hasAlpha = header.pixelFormat.flags & DDS_PIXEL_FORMAT_FLAGS_ALPHA_PIXELS;
        const bool RGB = header.pixelFormat.flags & DDS_PIXEL_FORMAT_FLAGS_RGB;

        // Determine the optimal format and component selectors from the RGBA masks
        bool found = false;
        for (const auto& it_fmt_masks : it_bpp_validComps->second)
        {
            format = (GX2SurfaceFormat)it_fmt_masks.first;
            const std::array<u32, 4>& masks = it_fmt_masks.second;
            if (alphaOnly) // Alpha-only
            {
                u8 aComp;
                if (FindMask(aMask, masks, &aComp))
                {
                    compSelArr = {
                        5,      // Red
                        5,      // Green
                        5,      // Blue
                        aComp,  // Alpha
                        4,      // Zero
                        5       // One
                    };
                    found = true;
                    break;
                }
            }
            else if (hasAlpha && RGB) // RGBA
            {
                u8 rComp, gComp, bComp, aComp;
                if (FindMask(rMask, masks, &rComp) && FindMask(gMask, masks, &gComp) && FindMask(bMask, masks, &bComp) && FindMask(aMask, masks, & aComp))
                {
                    compSelArr = {
                        rComp,  // Red
                        gComp,  // Green
                        bComp,  // Blue
                        aComp,  // Alpha
                        4,      // Zero
                        5       // One
                    };
                    found = true;
                    break;
                }
            }
            else if (hasAlpha) // LA (Luminance + Alpha)
            {
                u8 rComp, aComp;
                if (FindMask(rMask, masks, &rComp) && FindMask(aMask, masks, &aComp))
                {
                    compSelArr = {
                        rComp,  // Red
                        rComp,  // Green
                        rComp,  // Blue
                        aComp,  // Alpha
                        4,      // Zero
                        5       // One
                    };
                    found = true;
                    break;
                }
            }
            else if (RGB) // RGB
            {
                u8 rComp, gComp, bComp;
                if (FindMask(rMask, masks, &rComp) && FindMask(gMask, masks, &gComp) && FindMask(bMask, masks, &bComp))
                {
                    compSelArr = {
                        rComp,  // Red
                        gComp,  // Green
                        bComp,  // Blue
                        5,      // Alpha
                        4,      // Zero
                        5       // One
                    };
                    found = true;
                    break;
                }
            }
            else // Luminance
            {
                u8 rComp;
                if (FindMask(rMask, masks, &rComp))
                {
                    compSelArr = {
                        rComp,  // Red
                        rComp,  // Green
                        rComp,  // Blue
                        5,      // Alpha
                        4,      // Zero
                        5       // One
                    };
                    found = true;
                    break;
                }
            }
        }
        assert(found && "Could not determine the texture format of the input DDS file!");

        // If determined format is RGBA8, check and add SRGB mask
        if (format == GX2_SURFACE_FORMAT_UNORM_RGBA8 && SRGB)
            format = GX2_SURFACE_FORMAT_SRGB_RGBA8;

        // Calculate imageSize for this level
        imageSize = width * height * (bitsPerPixel >> 3);
    }
    else
    {
        const std::string fourCC(header.pixelFormat.fourCC, 4);

        // DX10 is not supported yet
        assert(fourCC != "DX10" && "DX10 DDS files are not supported!");

        // Validate FourCC
        const auto& it_fourCC = fourCCs_import.find(fourCC);
        if (it_fourCC == fourCCs_import.end())
        {
            std::cerr << "Unrecognized FourCC: " << fourCC << std::endl;
            assert(false);
        }

        // Determine the format and blockSize
        format = (GX2SurfaceFormat)(it_fourCC->second >> 8);
        const u8 blockSize = it_fourCC->second & 0xFF;

        // DDS is incapable of letting you select the components for BCn
        switch (GX2SurfaceFormatToAddrFormat(format))
        {
        case ADDR_FMT_BC1:
        case ADDR_FMT_BC2:
        case ADDR_FMT_BC3:
            compSelArr = {
                0,  // Red
                1,  // Green
                2,  // Blue
                3,  // Alpha
                4,  // Zero
                5   // One
            };
            // Check and add SRGB mask
            if (SRGB)
                format = (GX2SurfaceFormat)(0x400 | format);
            break;
        case ADDR_FMT_BC4:
            compSelArr = {
                0,  // Red
                4,  // Green
                4,  // Blue
                5,  // Alpha
                4,  // Zero
                5   // One
            };
            break;
        case ADDR_FMT_BC5:
            compSelArr = {
                0,  // Red
                1,  // Green
                4,  // Blue
                5,  // Alpha
                4,  // Zero
                5   // One
            };
            break;
        default:
            assert(false);
        }

        // Calculate imageSize for this level
        imageSize = ((width + 3) >> 2) * ((height + 3) >> 2) * blockSize;
    }

    // Get imagePtr and mipPtr
    const size_t imageOffs = sizeof(DDSHeader);
    const size_t mipOffs = imageOffs + imageSize;
    assert(fileSize >= mipOffs);
    const size_t mipSize = fileSize - mipOffs;
    const u8* const imagePtr = file + imageOffs;
    const u8* const mipPtr = file + mipOffs;

    // Combine the component selectors read from the input files and the user
    const u32 compSel = (u32)(compSelArr[compSelIdx >> 24 & 0xFF]) << 24 |
                        (u32)(compSelArr[compSelIdx >> 16 & 0xFF]) << 16 |
                        (u32)(compSelArr[compSelIdx >>  8 & 0xFF]) << 8  |
                        (u32)(compSelArr[compSelIdx       & 0xFF]);

    // Create the texture
    GX2TextureFromLinear2D(
        texture,
        width, height, numMips, format, compSel, imagePtr, imageSize,
        tileMode, swizzle, mipPtr, mipSize, gfd_v7
    );

    // Print debug info if specified
    if (printInfo)
        GX2TexturePrintInfo(texture);
}

static const std::unordered_map<u32, const std::string> fourCCs_export {
    { GX2_SURFACE_FORMAT_UNORM_BC1, "DXT1" },
    { GX2_SURFACE_FORMAT_UNORM_BC2, "DXT3" },
    { GX2_SURFACE_FORMAT_UNORM_BC3, "DXT5" },
    { GX2_SURFACE_FORMAT_UNORM_BC4, "ATI1" },
    { GX2_SURFACE_FORMAT_SNORM_BC4, "BC4S" },
    { GX2_SURFACE_FORMAT_UNORM_BC5, "ATI2" },
    { GX2_SURFACE_FORMAT_SNORM_BC5, "BC5S" }
};

static const std::unordered_map< u32, const std::array<u32, 4> > validComps_export {
    { GX2_SURFACE_FORMAT_UNORM_R8,      { 0x000000ff,          0,          0,          0 } },
    { GX2_SURFACE_FORMAT_UNORM_RG4,     { 0x0000000f, 0x000000f0,          0,          0 } },
    { GX2_SURFACE_FORMAT_UNORM_RG8,     { 0x000000ff, 0x0000ff00,          0,          0 } },
    { GX2_SURFACE_FORMAT_UNORM_RGB565,  { 0x0000001f, 0x000007e0, 0x0000f800,          0 } },
    { GX2_SURFACE_FORMAT_UNORM_RGB5A1,  { 0x0000001f, 0x000003e0, 0x00007c00, 0x00008000 } },
    { GX2_SURFACE_FORMAT_UNORM_RGBA4,   { 0x0000000f, 0x000000f0, 0x00000f00, 0x0000f000 } },
    { GX2_SURFACE_FORMAT_UNORM_RGB10A2, { 0x3ff00000, 0x000ffc00, 0x000003ff, 0xc0000000 } },
    { GX2_SURFACE_FORMAT_UNORM_RGBA8,   { 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000 } }
};

u8* GX2TextureToDDS(const GX2Texture* texture, size_t* fileSize, bool printInfo)
{
    // Print debug info if specified
    if (printInfo)
        GX2TexturePrintInfo(texture);

    // Check if format is supported
    const GX2SurfaceFormat format = texture->surface.format;
    switch (format)
    {
    case GX2_SURFACE_FORMAT_UNORM_R8:
    case GX2_SURFACE_FORMAT_UNORM_RG4:
    case GX2_SURFACE_FORMAT_UNORM_RG8:
    case GX2_SURFACE_FORMAT_UNORM_RGB565:
    case GX2_SURFACE_FORMAT_UNORM_RGB5A1:
    case GX2_SURFACE_FORMAT_UNORM_RGBA4:
    case GX2_SURFACE_FORMAT_UNORM_RGB10A2:
    case GX2_SURFACE_FORMAT_UNORM_RGBA8:
    case GX2_SURFACE_FORMAT_SRGB_RGBA8:
    case GX2_SURFACE_FORMAT_UNORM_BC1:
    case GX2_SURFACE_FORMAT_SRGB_BC1:
    case GX2_SURFACE_FORMAT_UNORM_BC2:
    case GX2_SURFACE_FORMAT_SRGB_BC2:
    case GX2_SURFACE_FORMAT_UNORM_BC3:
    case GX2_SURFACE_FORMAT_SRGB_BC3:
    case GX2_SURFACE_FORMAT_UNORM_BC4:
    case GX2_SURFACE_FORMAT_SNORM_BC4:
    case GX2_SURFACE_FORMAT_UNORM_BC5:
    case GX2_SURFACE_FORMAT_SNORM_BC5:
        break;
    default:
        assert(false && "Unimplemented texture format.");
        return nullptr;
    }

    assert(texture->surface.dim == GX2_SURFACE_DIM_2D);
    assert(texture->surface.depth <= 1);
    assert(texture->surface.aa == GX2_AA_MODE_1X);
    assert(texture->surface.use & GX2_SURFACE_USE_TEXTURE);

    const u32 width = texture->surface.width;
    const u32 height = texture->surface.height;
    const u32 numMips = texture->surface.numMips;

    // Create a new GX2Surface to store the untiled texture
    GX2Surface linear_surface;
    linear_surface.dim = GX2_SURFACE_DIM_2D;
    linear_surface.width = width;
    linear_surface.height = height;
    linear_surface.depth = 1;
    linear_surface.numMips = numMips;
    linear_surface.format = format;
    linear_surface.aa = GX2_AA_MODE_1X;
    linear_surface.use = GX2_SURFACE_USE_TEXTURE;
    linear_surface.tileMode = GX2_TILE_MODE_LINEAR_SPECIAL;
    linear_surface.swizzle = 0;

    GX2CalcSurfaceSizeAndAlignment(&linear_surface);

    // Allocate output buffer
    const size_t imageOffs = sizeof(DDSHeader);
    const size_t mipOffs = imageOffs + linear_surface.imageSize;
    const size_t _fileSize = mipOffs + linear_surface.mipSize;
    u8* file = (u8*)std::malloc(_fileSize);

    // Set the image data pointer
    linear_surface.imagePtr.set(file + imageOffs);

    // Set the mip data pointer
    if (numMips > 1)
        linear_surface.mipPtr.set(file + mipOffs);
    else
        linear_surface.mipPtr.set(nullptr);

    // Untile our texture
    GX2CopySurface(&texture->surface, 0, 0, &linear_surface, 0, 0);
    for (u32 i = 1; i < numMips; i++)
        GX2CopySurface(&texture->surface, i, 0, &linear_surface, i, 0);

    // Bits-per-pixel and bytes-per-pixel
    const u8 bitsPerPixel = GX2GetSurfaceFormatBitsPerPixel(format);
    const u8 bytesPerPixel = bitsPerPixel / 8;

    // Create a new DDSHeader object
    DDSHeader& header = *(new (file) DDSHeader);
    std::memcpy(header.magic, "DDS ", 4);
    header.size = sizeof(DDSHeader) - 4;
    header.flags = (DDSFlags)(DDS_FLAGS_CAPS | DDS_FLAGS_HEIGHT |
                              DDS_FLAGS_WIDTH | DDS_FLAGS_PIXEL_FORMAT);
    header.height = 0;
    header.width = 0;
    header.pitchOrLinearSize = 0;
    header.depth = 0;
    header.mipMapCount = 1;
    header.pixelFormat.size = sizeof(DDSPixelFormat);
    header.pixelFormat.flags = (DDSPixelFormatFlags)0;
    std::memset(header.pixelFormat.fourCC, 0, 4);
    header.pixelFormat.rgbBitCount = 0;
    header.pixelFormat.rBitMask = 0;
    header.pixelFormat.gBitMask = 0;
    header.pixelFormat.bBitMask = 0;
    header.pixelFormat.aBitMask = 0;
    header.caps = (DDSCaps)0;
    header.caps2 = (DDSCaps2)0;

    // Set misc. values
    header.width = width;
    header.height = height;
    header.caps |= DDS_CAPS_TEXTURE;

    // Set the mipmaps count and flags
    if (numMips > 1)
    {
        header.mipMapCount = numMips;
        header.flags |= DDS_FLAGS_MIP_MAP_COUNT;
        header.caps |= (DDS_CAPS_COMPLEX | DDS_CAPS_MIP_MAP);
    }

    const u32 compSel = texture->compSel;

    // Treat uncompressed formats differently
    if (!GX2SurfaceIsCompressed(format))
    {
        // Set the bits-per-pixel
        header.pixelFormat.rgbBitCount = bitsPerPixel;

        // Set the pitch and its flag
        header.pitchOrLinearSize = width * bytesPerPixel;
        header.flags |= DDS_FLAGS_PITCH;

        // Check if the Component Selectors are supported
        const u8 r = compSel >> 24 & 0xFF;
        const u8 g = compSel >> 16 & 0xFF;
        const u8 b = compSel >>  8 & 0xFF;
        const u8 a = compSel       & 0xFF;
        assert(0 <= r && r <= 5 &&
               0 <= g && g <= 5 &&
               0 <= b && b <= 5 &&
               0 <= a && a <= 5);
        assert(r != 4 && g != 4 && b != 4 && a != 4 && "Exporting as DDS with Component Selector set to Zero is not supported!");
        //   Check if One is used, but texture is not alpha-only
        assert(((r != 5 && g != 5 && b != 5) || (r == g && g == b && a < 4)) && "Exporting as DDS with RGB Component Selectors set to One and not alpha-only is not supported!");

        // Valid masks for this texture format
        const auto& it_masks = validComps_export.find(format & 0x3F);
        assert(it_masks != validComps_export.end());
        const std::array<u32, 4>& masks = it_masks->second;

        // Check alpha
        bool alphaOnly = false;
        if (a < 4)
        {
            // Validate Alpha
            const u32 aMask = masks[a];
            assert(aMask != 0 && "Invalid Alpha Channel Component Selector.");
            header.pixelFormat.aBitMask = aMask;

            if (r == g && g == b && b == 5)
            {
                // Alpha-only (specifically A8, but could be anything)
                alphaOnly = true;
                header.pixelFormat.flags |= DDS_PIXEL_FORMAT_FLAGS_ALPHA;
            }
            else
            {
                // Has alpha
                header.pixelFormat.flags |= DDS_PIXEL_FORMAT_FLAGS_ALPHA_PIXELS;
            }
        }
        else // a == 5
        {
            // No alpha
            header.pixelFormat.aBitMask = 0;
        }

        // Handle colors if not alpha-only
        if (!alphaOnly)
        {
            // Check for RGB vs Luminance
            if (r == g && g == b)
            {
                // Luminance (specifically L8/LA4/LA8, but could be anything)
                header.pixelFormat.flags |= DDS_PIXEL_FORMAT_FLAGS_LUMINANCE;
            }
            else
            {
                // Color
                header.pixelFormat.flags |= DDS_PIXEL_FORMAT_FLAGS_RGB;
            }

            assert(r < 4 && g < 4 && b < 4 && "Invalid RGB Component Selectors.");

            // Validate Red
            const u32 rMask = masks[r];
            assert(rMask != 0 && "Invalid Red Channel Component Selector.");

            // Validate Green
            const u32 gMask = masks[g];
            assert(gMask != 0 && "Invalid Green Channel Component Selector.");

            // Validate Blue
            const u32 bMask = masks[b];
            assert(bMask != 0 && "Invalid Blue Channel Component Selector.");

            // Set the RGB masks
            header.pixelFormat.rBitMask = rMask;
            header.pixelFormat.gBitMask = gMask;
            header.pixelFormat.bBitMask = bMask;
        }
    }
    else
    {
        // Set fourCC and its flag
        header.pixelFormat.flags |= DDS_PIXEL_FORMAT_FLAGS_FOUR_CC;
        const auto& it_fourCC = fourCCs_export.find(format & 0x23F);
        assert(it_fourCC != fourCCs_export.end());
        std::memcpy(header.pixelFormat.fourCC, it_fourCC->second.c_str(), 4);

        // Set the linear size and its flag
        header.pitchOrLinearSize = linear_surface.imageSize;
        header.flags |= DDS_FLAGS_LINEAR_SIZE;

        // DDS is incapable of letting you select the components for BCn
        if (format & 4 || compSel != 0x00010203)
        {
            std::clog << std::endl << "Warning: exporting as compressed DDS with application of RGBA Component Selectors is not possible!"
                      << std::endl << "Be noted of the current RGBA Component Selectors when viewing the ouput DDS file or re-importing it."
                      << std::endl << "If you want to see what this texture really looks like, consider exporting as PNG instead." << std::endl;
        }
    }

    *fileSize = _fileSize;
    return file;
}

}
