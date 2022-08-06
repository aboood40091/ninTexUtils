#include <ninTexUtils/bcn/decompress.h>

static inline u16 EXP5TO8R(u16 packedcol)
{
    return (packedcol >> 8 & 0xF8) | (packedcol >> 13 & 0x07);
}

static inline u16 EXP6TO8G(u16 packedcol)
{
    return (packedcol >> 3 & 0xFC) | (packedcol >>  9 & 0x03);
}

static inline u16 EXP5TO8B(u16 packedcol)
{
    return (packedcol << 3 & 0xF8) | (packedcol >>  2 & 0x07);
}

static inline u8 EXP4TO8(u8 col)
{
    return col | col << 4;
}

#define RCOMP 0
#define GCOMP 1
#define BCOMP 2
#define ACOMP 3

static inline u32 dxt135_decode_imageblock(const u8* img_block_src, u32 i, u32 j, bool is_dxt1)
{
    u32 pixel;
    u8* const pixel_U8 = (u8*)&pixel;
    pixel_U8[ACOMP] = 255;

    const u16 color0 = img_block_src[0] | img_block_src[1] << 8;
    const u16 color1 = img_block_src[2] | img_block_src[3] << 8;

    const u32 bits   = img_block_src[4]       | img_block_src[5] <<  8
                     | img_block_src[6] << 16 | img_block_src[7] << 24;

    const u8 bit_pos = 2 * (j * 4 + i);

    switch (bits >> bit_pos & 3)
    {
    case 0:
        pixel_U8[RCOMP] = EXP5TO8R(color0);
        pixel_U8[GCOMP] = EXP6TO8G(color0);
        pixel_U8[BCOMP] = EXP5TO8B(color0);
        break;
    case 1:
        pixel_U8[RCOMP] = EXP5TO8R(color1);
        pixel_U8[GCOMP] = EXP6TO8G(color1);
        pixel_U8[BCOMP] = EXP5TO8B(color1);
        break;
    case 2:
        if (color0 > color1)
        {
            pixel_U8[RCOMP] = (EXP5TO8R(color0) * 2 + EXP5TO8R(color1)) / 3;
            pixel_U8[GCOMP] = (EXP6TO8G(color0) * 2 + EXP6TO8G(color1)) / 3;
            pixel_U8[BCOMP] = (EXP5TO8B(color0) * 2 + EXP5TO8B(color1)) / 3;
        }
        else
        {
            pixel_U8[RCOMP] = (EXP5TO8R(color0) + EXP5TO8R(color1)) / 2;
            pixel_U8[GCOMP] = (EXP6TO8G(color0) + EXP6TO8G(color1)) / 2;
            pixel_U8[BCOMP] = (EXP5TO8B(color0) + EXP5TO8B(color1)) / 2;
        }
        break;
    case 3:
        if (!is_dxt1 || color0 > color1)
        {
            pixel_U8[RCOMP] = (EXP5TO8R(color0) + EXP5TO8R(color1) * 2) / 3;
            pixel_U8[GCOMP] = (EXP6TO8G(color0) + EXP6TO8G(color1) * 2) / 3;
            pixel_U8[BCOMP] = (EXP5TO8B(color0) + EXP5TO8B(color1) * 2) / 3;
        }
        else if (is_dxt1)
        {
            return 0;
        }
    }

    return pixel;
}

static inline u8 dxt5_decode_alphablock(const u8* alpha_block_src, u32 i, u32 j)
{
    const u8 alpha0 = alpha_block_src[0];
    const u8 alpha1 = alpha_block_src[1];

    const u8 bit_pos   = 3 * (j * 4 + i);
    const u8 acodelow  = alpha_block_src[2 + bit_pos / 8];
    const u8 acodehigh = alpha_block_src[3 + bit_pos / 8];

    const u8 code = (acodelow  >> (bit_pos & 7) |
                     acodehigh << (8 - (bit_pos & 7))) & 7;

    if (code == 0)
        return alpha0;

    else if (code == 1)
        return alpha1;

    else if (alpha0 > alpha1)
        return (alpha0 * (8 - code) + alpha1 * (code - 1)) / 7;

    else if (code < 6)
        return (alpha0 * (6 - code) + alpha1 * (code - 1)) / 5;

    else if (code == 6)
        return 0;

    return 255;
}

static inline s8 dxt5_decode_alphablock_signed(const u8* alpha_block_src, u32 i, u32 j)
{
    const s8 alpha0 = alpha_block_src[0];
    const s8 alpha1 = alpha_block_src[1];

    const u8 bit_pos   = 3 * (j * 4 + i);
    const u8 acodelow  = alpha_block_src[2 + bit_pos / 8];
    const u8 acodehigh = alpha_block_src[3 + bit_pos / 8];

    const u8 code = (acodelow  >> (bit_pos & 7) |
                     acodehigh << (8 - (bit_pos & 7))) & 7;

    if (code == 0)
        return alpha0;

    else if (code == 1)
        return alpha1;

    else if (alpha0 > alpha1)
        return (alpha0 * (8 - code) + alpha1 * (code - 1)) / 7;

    else if (code < 6)
        return (alpha0 * (6 - code) + alpha1 * (code - 1)) / 5;

    else if (code == 6)
        return -128;

    return 127;
}

static inline u32 fetch_2d_texel_rgba_dxt1(const u8* pixdata, u32 i, u32 j, u32 pitch)
{
    const u8* blksrc = pixdata + 8 * ((j / 4) * pitch + (i / 4));
    return dxt135_decode_imageblock(blksrc, i & 3, j & 3, true);
}

static inline u32 fetch_2d_texel_rgba_dxt3(const u8* pixdata, u32 i, u32 j, u32 pitch)
{
    const u8* blksrc = pixdata + 16 * ((j / 4) * pitch + (i / 4));
    u32 pixel = dxt135_decode_imageblock(blksrc + 8, i & 3, j & 3, false);
    ((u8*)&pixel)[ACOMP] = EXP4TO8(blksrc[((j & 3) * 4 + (i & 3)) / 2] >> 4 * (i & 1) & 0xF);
    return pixel;
}

static inline u32 fetch_2d_texel_rgba_dxt5(const u8* pixdata, u32 i, u32 j, u32 pitch)
{
    const u8* blksrc = pixdata + 16 * ((j / 4) * pitch + (i / 4));
    u32 pixel = dxt135_decode_imageblock(blksrc + 8, i & 3, j & 3, false);
    ((u8*)&pixel)[ACOMP] = dxt5_decode_alphablock(blksrc, i & 3, j & 3);
    return pixel;
}

static inline u8 fetch_2d_texel_r_bc4(const u8* pixdata, u32 i, u32 j, u32 pitch)
{
    const u8* blksrc = pixdata + 8 * ((j / 4) * pitch + (i / 4));
    return dxt5_decode_alphablock(blksrc, i & 3, j & 3);
}

static inline u8 fetch_2d_texel_r_bc4_signed(const u8* pixdata, u32 i, u32 j, u32 pitch)
{
    const u8* blksrc = pixdata + 8 * ((j / 4) * pitch + (i / 4));
    return dxt5_decode_alphablock_signed(blksrc, i & 3, j & 3) + 128;
}

static inline u16 fetch_2d_texel_rg_bc5(const u8* pixdata, u32 i, u32 j, u32 pitch)
{
    const u8* blksrc = pixdata + 16 * ((j / 4) * pitch + (i / 4));
    u16 pixel;
    ((u8*)&pixel)[RCOMP] = dxt5_decode_alphablock(blksrc, i & 3, j & 3);
    ((u8*)&pixel)[GCOMP] = dxt5_decode_alphablock(blksrc + 8, i & 3, j & 3);
    return pixel;
}

static inline u16 fetch_2d_texel_rg_bc5_signed(const u8* pixdata, u32 i, u32 j, u32 pitch)
{
    const u8* blksrc = pixdata + 16 * ((j / 4) * pitch + (i / 4));
    u16 pixel;
    ((u8*)&pixel)[RCOMP] = dxt5_decode_alphablock_signed(blksrc, i & 3, j & 3) + 128;
    ((u8*)&pixel)[GCOMP] = dxt5_decode_alphablock_signed(blksrc + 8, i & 3, j & 3) + 128;
    return pixel;
}

void BCn_DecompressBC1(u32 width, u32 height, const u8* in_data, u8* out_data)
{
    u32 pitch = (width + 3) >> 2;
    u32* out_rgba_data = (u32*)out_data;

    for (u32 y = 0; y < height; y++)
        for (u32 x = 0; x < width; x++)
            *out_rgba_data++ = fetch_2d_texel_rgba_dxt1(in_data, x, y, pitch);
}

void BCn_DecompressBC2(u32 width, u32 height, const u8* in_data, u8* out_data)
{
    u32 pitch = (width + 3) >> 2;
    u32* out_rgba_data = (u32*)out_data;

    for (u32 y = 0; y < height; y++)
        for (u32 x = 0; x < width; x++)
            *out_rgba_data++ = fetch_2d_texel_rgba_dxt3(in_data, x, y, pitch);
}

void BCn_DecompressBC3(u32 width, u32 height, const u8* in_data, u8* out_data)
{
    u32 pitch = (width + 3) >> 2;
    u32* out_rgba_data = (u32*)out_data;

    for (u32 y = 0; y < height; y++)
        for (u32 x = 0; x < width; x++)
            *out_rgba_data++ = fetch_2d_texel_rgba_dxt5(in_data, x, y, pitch);
}

void BCn_DecompressBC4S(u32 width, u32 height, const u8* in_data, u8* out_data)
{
    u32 pitch = (width + 3) >> 2;

    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            u8 R = fetch_2d_texel_r_bc4_signed(in_data, x, y, pitch);

            out_data[RCOMP] = R;
            out_data[GCOMP] = R;
            out_data[BCOMP] = R;
            out_data[ACOMP] = 255;
            out_data += 4;
        }
    }
}

void BCn_DecompressBC4U(u32 width, u32 height, const u8* in_data, u8* out_data)
{
    u32 pitch = (width + 3) >> 2;

    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            u8 R = fetch_2d_texel_r_bc4(in_data, x, y, pitch);

            out_data[RCOMP] = R;
            out_data[GCOMP] = R;
            out_data[BCOMP] = R;
            out_data[ACOMP] = 255;
            out_data += 4;
        }
    }
}

void BCn_DecompressBC5S(u32 width, u32 height, const u8* in_data, u8* out_data)
{
    u32 pitch = (width + 3) >> 2;

    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            u16 RG = fetch_2d_texel_rg_bc5_signed(in_data, x, y, pitch);

            out_data[RCOMP] = ((u8*)&RG)[RCOMP];
            out_data[GCOMP] = ((u8*)&RG)[GCOMP];
            out_data[BCOMP] = 0;
            out_data[ACOMP] = 255;
            out_data += 4;
        }
    }
}

void BCn_DecompressBC5U(u32 width, u32 height, const u8* in_data, u8* out_data)
{
    u32 pitch = (width + 3) >> 2;

    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            u16 RG = fetch_2d_texel_rg_bc5(in_data, x, y, pitch);

            out_data[RCOMP] = ((u8*)&RG)[RCOMP];
            out_data[GCOMP] = ((u8*)&RG)[GCOMP];
            out_data[BCOMP] = 0;
            out_data[ACOMP] = 255;
            out_data += 4;
        }
    }
}
