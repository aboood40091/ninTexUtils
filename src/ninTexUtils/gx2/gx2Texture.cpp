#include <ninTexUtils/gx2/gx2Texture.h>

#include <algorithm>
#include <cassert>
#include <cstring>
#include <iostream>

extern "C"
{

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
        dst->regs[0]        = __builtin_bswap32(src->regs[0]);
        dst->regs[1]        = __builtin_bswap32(src->regs[1]);
        dst->regs[2]        = __builtin_bswap32(src->regs[2]);
        dst->regs[3]        = __builtin_bswap32(src->regs[3]);
        dst->regs[4]        = __builtin_bswap32(src->regs[4]);
    }
    else if (src != dst)
    {
        std::memmove(&dst->viewFirstMip, &src->viewFirstMip, sizeof(GX2Texture) - sizeof(GX2Surface));
    }

    if (serialized)
    {
        assert(dst->surface.aa == GX2_AA_MODE_1X);
        assert(dst->surface.use & GX2_SURFACE_USE_TEXTURE);

        dst->viewNumMips = std::max(dst->viewNumMips, 1u);
        dst->viewNumSlices = std::max(dst->viewNumSlices, 1u);

        assert(0 <= dst->viewFirstMip   && dst->viewFirstMip   <= dst->surface.numMips - 1);
        assert(1 <= dst->viewNumMips    && dst->viewNumMips    <= dst->surface.numMips - dst->viewFirstMip);
        assert(0 <= dst->viewFirstSlice && dst->viewFirstSlice <= dst->surface.depth   - 1);
        assert(1 <= dst->viewNumSlices  && dst->viewNumSlices  <= dst->surface.depth   - dst->viewFirstSlice);
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

}
