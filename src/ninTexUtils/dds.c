#include <ninTexUtils/dds.h>
#include <string.h>

static DDSHeader temp_header = { 0 };

bool DDSReadFile(const void* file, DDSHeader* out_header)
{
    memcpy(&temp_header, file, sizeof(DDSHeader));

    /* TODO: Reverse endian */

    if (temp_header.magic[0] != 'D' || temp_header.magic[1] != 'D' ||
        temp_header.magic[2] != 'S' || temp_header.magic[3] != ' ')
        return false;

    if (temp_header.size != sizeof(DDSHeader) - 4)
        return false;

    /* Minimum required flags */
    const DDSFlags minFlags = DDS_FLAGS_CAPS | DDS_FLAGS_HEIGHT |
                              DDS_FLAGS_WIDTH | DDS_FLAGS_PIXEL_FORMAT;
    if ((temp_header.flags & minFlags) != minFlags)
        return false;

    /* Can't have these enabled together */
    if ((temp_header.flags & (DDS_FLAGS_PITCH | DDS_FLAGS_LINEAR_SIZE)) ==
                              (DDS_FLAGS_PITCH | DDS_FLAGS_LINEAR_SIZE))
        return false;

    if (temp_header.width == 0 || temp_header.height == 0)
        return false;

    if (!(temp_header.caps & DDS_CAPS_TEXTURE))
        return false;

    if (temp_header.flags & (DDS_FLAGS_PITCH | DDS_FLAGS_LINEAR_SIZE))
    {
        if (temp_header.pitchOrLinearSize == 0)
            return false;
    }
    else
    {
        temp_header.pitchOrLinearSize = 0;
    }

    if (temp_header.flags & DDS_FLAGS_DEPTH)
    {
        if (temp_header.depth == 0)
            return false;
    }
    else
    {
        temp_header.depth = 0;
    }

    if (temp_header.flags & DDS_FLAGS_MIP_MAP_COUNT)
    {
        if (1 > temp_header.mipMapCount || temp_header.mipMapCount > 14)
            return false;
    }
    else
    {
        temp_header.mipMapCount = 1;
    }

    if (temp_header.pixelFormat.size != sizeof(DDSPixelFormat))
        return false;

    const DDSPixelFormatFlags flags = temp_header.pixelFormat.flags & (
        DDS_PIXEL_FORMAT_FLAGS_ALPHA |
        DDS_PIXEL_FORMAT_FLAGS_FOUR_CC |
        DDS_PIXEL_FORMAT_FLAGS_RGB |
        DDS_PIXEL_FORMAT_FLAGS_YUV |
        DDS_PIXEL_FORMAT_FLAGS_LUMINANCE
    );

    /* Make sure one and only one of them is enabled */
    if (!(flags == DDS_PIXEL_FORMAT_FLAGS_ALPHA ||
          flags == DDS_PIXEL_FORMAT_FLAGS_FOUR_CC ||
          flags == DDS_PIXEL_FORMAT_FLAGS_RGB ||
          flags == DDS_PIXEL_FORMAT_FLAGS_YUV ||
          flags == DDS_PIXEL_FORMAT_FLAGS_LUMINANCE))
        return false;

    if (flags & DDS_PIXEL_FORMAT_FLAGS_FOUR_CC)
    {
        if (*(u32*)&temp_header.pixelFormat.fourCC == 0)
            return false;
    }
    else
    {
        *(u32*)&temp_header.pixelFormat.fourCC = 0;
    }

    memcpy(out_header, &temp_header, sizeof(DDSHeader));
    return true;
}
