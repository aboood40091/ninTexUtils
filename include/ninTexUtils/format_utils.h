#ifndef NIN_TEX_UTILS_FORMAT_UTILS_H_
#define NIN_TEX_UTILS_FORMAT_UTILS_H_

#include "types.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum _TexFormatUtilsFormat
{
    TEX_FORMAT_UTILS_FORMAT_L8      = 0,
    TEX_FORMAT_UTILS_FORMAT_LA8     = 1,
    TEX_FORMAT_UTILS_FORMAT_LA4     = 2,
    TEX_FORMAT_UTILS_FORMAT_RGB565  = 3,
    TEX_FORMAT_UTILS_FORMAT_RGB5A1  = 4,
    TEX_FORMAT_UTILS_FORMAT_RGBA4   = 5,
    TEX_FORMAT_UTILS_FORMAT_RGBX8   = 6,
    TEX_FORMAT_UTILS_FORMAT_RGB10A2 = 7,
    TEX_FORMAT_UTILS_FORMAT_RGBA8   = 8

}
TexFormatUtilsFormat;

typedef enum _TexFormatUtilsComponent
{
    TEX_FORMAT_UTILS_COMPONENT_R = 0,
    TEX_FORMAT_UTILS_COMPONENT_G = 1,
    TEX_FORMAT_UTILS_COMPONENT_B = 2,
    TEX_FORMAT_UTILS_COMPONENT_A = 3,
    TEX_FORMAT_UTILS_COMPONENT_0 = 4,
    TEX_FORMAT_UTILS_COMPONENT_1 = 5,

    TEX_FORMAT_UTILS_COMPONENT_RED   = TEX_FORMAT_UTILS_COMPONENT_R,
    TEX_FORMAT_UTILS_COMPONENT_GREEN = TEX_FORMAT_UTILS_COMPONENT_G,
    TEX_FORMAT_UTILS_COMPONENT_BLUE  = TEX_FORMAT_UTILS_COMPONENT_B,
    TEX_FORMAT_UTILS_COMPONENT_ALPHA = TEX_FORMAT_UTILS_COMPONENT_A,
    TEX_FORMAT_UTILS_COMPONENT_ZERO  = TEX_FORMAT_UTILS_COMPONENT_0,
    TEX_FORMAT_UTILS_COMPONENT_ONE   = TEX_FORMAT_UTILS_COMPONENT_1
}
TexFormatUtilsComponent;

inline u32 TexFormatUtils_GetFormatBPP(TexFormatUtilsFormat format)
{
    switch (format)
    {
    case TEX_FORMAT_UTILS_FORMAT_L8:
    case TEX_FORMAT_UTILS_FORMAT_LA4:
        return 1;
    case TEX_FORMAT_UTILS_FORMAT_LA8:
    case TEX_FORMAT_UTILS_FORMAT_RGB565:
    case TEX_FORMAT_UTILS_FORMAT_RGB5A1:
    case TEX_FORMAT_UTILS_FORMAT_RGBA4:
        return 2;
    case TEX_FORMAT_UTILS_FORMAT_RGBX8:
    case TEX_FORMAT_UTILS_FORMAT_RGB10A2:
    case TEX_FORMAT_UTILS_FORMAT_RGBA8:
        return 4;
    default:
        return 0;
    }
}

inline void TexFormatUtils_GetComponentsFromPixel(TexFormatUtilsFormat format,
                                                  u32 pixel,
                                                  u8* comp)
{
    switch (format)
    {
    case TEX_FORMAT_UTILS_FORMAT_L8:
        comp[TEX_FORMAT_UTILS_COMPONENT_R] = pixel;
        break;
    case TEX_FORMAT_UTILS_FORMAT_LA8:
        comp[TEX_FORMAT_UTILS_COMPONENT_R] =  pixel & 0xFF;
        comp[TEX_FORMAT_UTILS_COMPONENT_G] = (pixel & 0xFF00) >> 8;
        break;
    case TEX_FORMAT_UTILS_FORMAT_LA4:
        comp[TEX_FORMAT_UTILS_COMPONENT_R] =  (pixel & 0xF)        * 17;
        comp[TEX_FORMAT_UTILS_COMPONENT_G] = ((pixel & 0xF0) >> 4) * 17;
        break;
    case TEX_FORMAT_UTILS_FORMAT_RGB565:
        comp[TEX_FORMAT_UTILS_COMPONENT_R] =  (pixel & 0x1F)          * 0xFF / 0x1F;
        comp[TEX_FORMAT_UTILS_COMPONENT_G] = ((pixel & 0x7E0) >> 5)   * 0xFF / 0x3F;
        comp[TEX_FORMAT_UTILS_COMPONENT_B] = ((pixel & 0xF800) >> 11) * 0xFF / 0x1F;
        break;
    case TEX_FORMAT_UTILS_FORMAT_RGB5A1:
        comp[TEX_FORMAT_UTILS_COMPONENT_R] =  (pixel & 0x1F)          * 0xFF / 0x1F;
        comp[TEX_FORMAT_UTILS_COMPONENT_G] = ((pixel & 0x3E0) >> 5)   * 0xFF / 0x1F;
        comp[TEX_FORMAT_UTILS_COMPONENT_B] = ((pixel & 0x7C00) >> 10) * 0xFF / 0x1F;
        comp[TEX_FORMAT_UTILS_COMPONENT_A] = ((pixel & 0x8000) >> 15) * 0xFF;
        break;
    case TEX_FORMAT_UTILS_FORMAT_RGBA4:
        comp[TEX_FORMAT_UTILS_COMPONENT_R] =  (pixel & 0xF)           * 17;
        comp[TEX_FORMAT_UTILS_COMPONENT_G] = ((pixel & 0xF0) >> 4)    * 17;
        comp[TEX_FORMAT_UTILS_COMPONENT_B] = ((pixel & 0xF00) >> 8)   * 17;
        comp[TEX_FORMAT_UTILS_COMPONENT_A] = ((pixel & 0xF000) >> 12) * 17;
        break;
    case TEX_FORMAT_UTILS_FORMAT_RGBX8:
        comp[TEX_FORMAT_UTILS_COMPONENT_R] =  pixel & 0xFF;
        comp[TEX_FORMAT_UTILS_COMPONENT_G] = (pixel & 0xFF00) >> 8;
        comp[TEX_FORMAT_UTILS_COMPONENT_B] = (pixel & 0xFF0000) >> 16;
        break;
    case TEX_FORMAT_UTILS_FORMAT_RGB10A2:
        comp[TEX_FORMAT_UTILS_COMPONENT_R] =  (pixel & 0x3FF)             * 0xFF / 0x3FF;
        comp[TEX_FORMAT_UTILS_COMPONENT_G] = ((pixel & 0xFFC00) >> 10)    * 0xFF / 0x3FF;
        comp[TEX_FORMAT_UTILS_COMPONENT_B] = ((pixel & 0x3FF00000) >> 20) * 0xFF / 0x3FF;
        comp[TEX_FORMAT_UTILS_COMPONENT_A] = ((pixel & 0xC0000000) >> 30) * 0xFF / 0x3;
        break;
    case TEX_FORMAT_UTILS_FORMAT_RGBA8:
        comp[TEX_FORMAT_UTILS_COMPONENT_R] =  pixel & 0xFF;
        comp[TEX_FORMAT_UTILS_COMPONENT_G] = (pixel & 0xFF00) >> 8;
        comp[TEX_FORMAT_UTILS_COMPONENT_B] = (pixel & 0xFF0000) >> 16;
        comp[TEX_FORMAT_UTILS_COMPONENT_A] = (pixel & 0xFF000000) >> 24;
    }
}

inline void TexFormatUtils_ToRGBA8(u32 width, u32 height,
                                   const u8* in_data,
                                   u8* out_data,
                                   TexFormatUtilsFormat format,
                                   TexFormatUtilsComponent comp_r_sel,
                                   TexFormatUtilsComponent comp_g_sel,
                                   TexFormatUtilsComponent comp_b_sel,
                                   TexFormatUtilsComponent comp_a_sel)
{
    u8 comp[] = {
        0,   /* TEX_FORMAT_UTILS_COMPONENT_R */
        0,   /* TEX_FORMAT_UTILS_COMPONENT_G */
        0,   /* TEX_FORMAT_UTILS_COMPONENT_B */
        255, /* TEX_FORMAT_UTILS_COMPONENT_A */
        0,   /* TEX_FORMAT_UTILS_COMPONENT_0 */
        255, /* TEX_FORMAT_UTILS_COMPONENT_1 */
    };

    const u32 bpp = TexFormatUtils_GetFormatBPP(format);
    if (bpp == 0)
        return;

    for (u32 y = 0; y < height; y++)
    {
        for (u32 x = 0; x < width; x++)
        {
            const u32 pixel_idx = y * width + x;
            const u32 in_pos = pixel_idx * bpp;
            const u32 out_pos = pixel_idx * 4;

            /* Read pixel data in little endian */

            switch (bpp)
            {
            case 1:
                TexFormatUtils_GetComponentsFromPixel(format,
                                                      in_data[in_pos],
                                                      comp);
                break;
            case 2:
                TexFormatUtils_GetComponentsFromPixel(format,
                                                      (in_data[in_pos] |
                                                       in_data[in_pos + 1] << 8),
                                                      comp);
                break;
            case 4:
                TexFormatUtils_GetComponentsFromPixel(format,
                                                      (in_data[in_pos] |
                                                       in_data[in_pos + 1] << 8 |
                                                       in_data[in_pos + 2] << 16 |
                                                       in_data[in_pos + 3] << 24),
                                                      comp);
            }

            out_data[out_pos + 0] = comp[comp_r_sel];
            out_data[out_pos + 1] = comp[comp_g_sel];
            out_data[out_pos + 2] = comp[comp_b_sel];
            out_data[out_pos + 3] = comp[comp_a_sel];
        }
    }
}

inline void TexFormatUtils_ToRGBA8_CompSelArr(u32 width, u32 height,
                                              const u8* in_data,
                                              u8* out_data,
                                              TexFormatUtilsFormat format,
                                              const TexFormatUtilsComponent comp_sel[])
{
    TexFormatUtils_ToRGBA8(width, height,
                           in_data,
                           out_data,
                           format,
                           comp_sel[0],
                           comp_sel[1],
                           comp_sel[2],
                           comp_sel[3]);
}

inline void TexFormatUtils_ToRGBA8_CompSelBits(u32 width, u32 height,
                                               const u8* in_data,
                                               u8* out_data,
                                               TexFormatUtilsFormat format,
                                               u32 comp_sel)
{
    TexFormatUtils_ToRGBA8(width, height,
                           in_data,
                           out_data,
                           format,
                           (TexFormatUtilsComponent)(comp_sel >> 24 & 0xFF),
                           (TexFormatUtilsComponent)(comp_sel >> 16 & 0xFF),
                           (TexFormatUtilsComponent)(comp_sel >>  8 & 0xFF),
                           (TexFormatUtilsComponent)(comp_sel >>  0 & 0xFF));
}

inline void TexFormatUtils_RGB8ToRGBX8(u32 width, u32 height,
                                       const u8* in_data,
                                       u8* out_data)
{
    for (u32 i = 0; i < width * height; i++)
    {
        out_data[4 * i + 0] = in_data[3 * i + 0];
        out_data[4 * i + 1] = in_data[3 * i + 1];
        out_data[4 * i + 2] = in_data[3 * i + 2];
        out_data[4 * i + 3] = 0xFF;
    }
}

#ifdef __cplusplus
}
#endif

#endif
