#pragma once

// This file is included inside gfdStruct.h
//#include "gfdStruct.h"

#include <ninTexUtils/gx2/gx2Shaders.h>
#include <ninTexUtils/gx2/gx2Texture.h>

#include <vector>

//typedef struct _GX2ComputeShader  GX2ComputeShader;

class GFDFile
{
public:
    GFDFile()
        : mHeader() // Zero-initialize
    {
        mHeader.magic = 0x47667832u; // Gfx2
        mHeader.size = sizeof(GFDHeader);
        mHeader.majorVersion = 7;
        mHeader.minorVersion = 1;
        mHeader.gpuVersion = GFD_GPU_VERSION_GPU7;
        mHeader.alignMode = GFD_ALIGN_MODE_ENABLE;
    }

    ~GFDFile()
    {
        destroy();
    }

    bool setVersion(u32 majorVersion, u32 minorVersion, u32* pSurfMode = NULL, u32* pPerfModulation = NULL)
    {
        if (majorVersion != 6 && majorVersion != 7)
            return false;

        mHeader.majorVersion = majorVersion;
        mHeader.minorVersion = minorVersion;

        /*
        u32 surfMode       = (majorVersion == 6) ? 1 : 0;
        u32 perfModulation = (majorVersion == 6) ? 0 : 7;

        for (u32 i = 0; i < mTextures.size(); i++)
            GX2InitTextureRegs(&mTextures[i], surfMode, perfModulation);

        if (pSurfMode)
            *pSurfMode = surfMode;

        if (pPerfModulation)
            *pPerfModulation = perfModulation;
        */

        return true;
    }

    size_t load(const void* data);
    void destroy();

public:
    GFDHeader mHeader;
    std::vector<GX2Texture> mTextures;
    std::vector<GX2VertexShader> mVertexShaders;
    std::vector<GX2PixelShader> mPixelShaders;
    std::vector<GX2GeometryShader> mGeometryShaders;
    //std::vector<GX2ComputeShader> mComputeShaders;
};
