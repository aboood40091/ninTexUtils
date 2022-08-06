#include <ninTexUtils/gfd/gfdStruct.h>

#include <cassert>
#include <cstring>

extern "C"
{

void LoadGFDHeader(const void* data, GFDHeader* header, bool serialized, bool isBigEndian)
{
    const GFDHeader* src = (const GFDHeader*)data;
    GFDHeader* dst = header;

    assert(src != NULL);
    assert(dst != NULL);

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        dst->magic        =                __builtin_bswap32(     src->magic);
        dst->size         =                __builtin_bswap32(     src->size);
        dst->majorVersion =                __builtin_bswap32(     src->majorVersion);
        dst->minorVersion =                __builtin_bswap32(     src->minorVersion);
        dst->gpuVersion   = (GFDGPUVersion)__builtin_bswap32((u32)src->gpuVersion);
        dst->alignMode    =  (GFDAlignMode)__builtin_bswap32((u32)src->alignMode);
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GFDHeader));
    }

    if (serialized)
    {
        assert(dst->magic        == 0x47667832u); // Gfx2
        assert(dst->size         == sizeof(GFDHeader));
        assert(dst->majorVersion == 6 ||
               dst->majorVersion == 7);
        assert(dst->gpuVersion   == GFD_GPU_VERSION_GPU7);

        if (dst->majorVersion == 6 && dst->minorVersion == 0)
            dst->alignMode = GFD_ALIGN_MODE_UNDEF;
    }
}

void LoadGFDBlockHeader(const void* data, GFDBlockHeader* block, bool serialized, bool isBigEndian)
{
    const GFDBlockHeader* src = (const GFDBlockHeader*)data;
    GFDBlockHeader* dst = block;

    assert(src != NULL);
    assert(dst != NULL);

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        dst->magic        =               __builtin_bswap32(     src->magic);
        dst->size         =               __builtin_bswap32(     src->size);
        dst->majorVersion =               __builtin_bswap32(     src->majorVersion);
        dst->minorVersion =               __builtin_bswap32(     src->minorVersion);
        dst->type         = (GFDBlockType)__builtin_bswap32((u32)src->type);
        dst->dataSize     =               __builtin_bswap32(     src->dataSize);
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GFDBlockHeader));
    }

    if (serialized)
    {
        assert(dst->magic        == 0x424C4B7Bu); // BLK{
        assert(dst->size         == sizeof(GFDBlockHeader));
        assert(dst->majorVersion == 0 ||
               dst->majorVersion == 1);
        assert(dst->type         != GFD_BLOCK_TYPE_INVALID);

        if (dst->type == GFD_BLOCK_TYPE_END)
        {
            assert(dst->dataSize == 0);
        }
    }
}

}

size_t GFDFile::load(const void* data)
{
    // Re-initialize the file
    destroy();

    const u8* data_u8 = (const u8*)data;

    LoadGFDHeader(data_u8, &mHeader);
    data_u8 += sizeof(GFDHeader);

    bool searchAlignmentBlock = mHeader.majorVersion == 6 && mHeader.minorVersion == 0;

    GX2Texture*        currentTexture        = NULL;
    GX2VertexShader*   currentVertexShader   = NULL;
    GX2PixelShader*    currentPixelShader    = NULL;
    GX2GeometryShader* currentGeometryShader = NULL;
  //GX2ComputeShader*  currentComputeShader  = NULL;

    GFDBlockHeader blockHeader;

    while (true)
    {
        LoadGFDBlockHeader(data_u8, &blockHeader);
        data_u8 += sizeof(GFDBlockHeader);

        const u32            blockVersion  = blockHeader.majorVersion;
        const GFDBlockType   blockType     = blockHeader.type;
        const GFDBlockTypeV0 blockTypeV0   = blockHeader.typeV0;
        const GFDBlockTypeV1 blockTypeV1   = blockHeader.typeV1;
        const u32            blockDataSize = blockHeader.dataSize;

        if (blockType == GFD_BLOCK_TYPE_END)
        {
            //assert(blockDataSize == 0);
            data_u8 += blockDataSize;
            break;
        }
        else if (blockType == GFD_BLOCK_TYPE_PAD)
        {
            if (searchAlignmentBlock)
            {
                mHeader.alignMode = GFD_ALIGN_MODE_ENABLE;
                searchAlignmentBlock = false;
            }
        }
        else if (blockType == GFD_BLOCK_TYPE_GX2_VS_HEADER)
        {
            assert(blockDataSize >= sizeof(GX2VertexShader));
            mVertexShaders.push_back(GX2VertexShader());
            currentVertexShader = &mVertexShaders.back();
            LoadGX2VertexShader(data_u8, currentVertexShader);
        }
        else if (blockType == GFD_BLOCK_TYPE_GX2_VS_PROGRAM)
        {
            assert(currentVertexShader != NULL && currentVertexShader->shaderPtr == NULL);
            assert(blockDataSize == currentVertexShader->shaderSize);
            currentVertexShader->shaderPtr = new u8[blockDataSize];
            std::memcpy(currentVertexShader->shaderPtr, data_u8, blockDataSize);
        }
        else if (blockType == GFD_BLOCK_TYPE_GX2_PS_HEADER)
        {
            assert(blockDataSize >= sizeof(GX2PixelShader));
            mPixelShaders.push_back(GX2PixelShader());
            currentPixelShader = &mPixelShaders.back();
            LoadGX2PixelShader(data_u8, currentPixelShader);
        }
        else if (blockType == GFD_BLOCK_TYPE_GX2_PS_PROGRAM)
        {
            assert(currentPixelShader != NULL && currentPixelShader->shaderPtr == NULL);
            assert(blockDataSize == currentPixelShader->shaderSize);
            currentPixelShader->shaderPtr = new u8[blockDataSize];
            std::memcpy(currentPixelShader->shaderPtr, data_u8, blockDataSize);
        }
        else if (blockType == GFD_BLOCK_TYPE_GX2_GS_HEADER)
        {
            assert(blockDataSize >= sizeof(GX2GeometryShader));
            mGeometryShaders.push_back(GX2GeometryShader());
            currentGeometryShader = &mGeometryShaders.back();
            LoadGX2GeometryShader(data_u8, currentGeometryShader);
        }
        else if (blockType == GFD_BLOCK_TYPE_GX2_GS_PROGRAM)
        {
            assert(currentGeometryShader != NULL && currentGeometryShader->shaderPtr == NULL);
            assert(blockDataSize == currentGeometryShader->shaderSize);
            currentGeometryShader->shaderPtr = new u8[blockDataSize];
            std::memcpy(currentGeometryShader->shaderPtr, data_u8, blockDataSize);
        }
        else if ((blockVersion == 0 && blockTypeV0 == GFD_BLOCK_TYPE_V0_GX2_GS_COPY_PROGRAM) ||
                 (blockVersion == 1 && blockTypeV1 == GFD_BLOCK_TYPE_V1_GX2_GS_COPY_PROGRAM))
        {
            assert(currentGeometryShader != NULL && currentGeometryShader->copyShaderPtr == NULL);
            assert(blockDataSize == currentGeometryShader->copyShaderSize);
            currentGeometryShader->copyShaderPtr = new u8[blockDataSize];
            std::memcpy(currentGeometryShader->copyShaderPtr, data_u8, blockDataSize);
        }
        else if ((blockVersion == 0 && blockTypeV0 == GFD_BLOCK_TYPE_V0_GX2_TEX_HEADER) ||
                 (blockVersion == 1 && blockTypeV1 == GFD_BLOCK_TYPE_V1_GX2_TEX_HEADER))
        {
            assert(blockDataSize == sizeof(GX2Texture));
            mTextures.push_back(GX2Texture());
            currentTexture = &mTextures.back();
            LoadGX2Texture(data_u8, currentTexture);
        }
        else if ((blockVersion == 0 && blockTypeV0 == GFD_BLOCK_TYPE_V0_GX2_TEX_IMAGE_DATA) ||
                 (blockVersion == 1 && blockTypeV1 == GFD_BLOCK_TYPE_V1_GX2_TEX_IMAGE_DATA))
        {
            assert(currentTexture != NULL && currentTexture->surface.imagePtr == NULL);
            assert(blockDataSize == currentTexture->surface.imageSize);
            currentTexture->surface.imagePtr = new u8[blockDataSize];
            std::memcpy(currentTexture->surface.imagePtr, data_u8, blockDataSize);
        }
        else if ((blockVersion == 0 && blockTypeV0 == GFD_BLOCK_TYPE_V0_GX2_TEX_MIP_DATA) ||
                 (blockVersion == 1 && blockTypeV1 == GFD_BLOCK_TYPE_V1_GX2_TEX_MIP_DATA))
        {
            assert(currentTexture != NULL && currentTexture->surface.mipPtr == NULL);
            assert(blockDataSize == currentTexture->surface.mipSize);
            currentTexture->surface.mipPtr = new u8[blockDataSize];
            std::memcpy(currentTexture->surface.mipPtr, data_u8, blockDataSize);
        }

        data_u8 += blockDataSize;
    }

    if (searchAlignmentBlock)
        mHeader.alignMode = GFD_ALIGN_MODE_DISABLE;

    return (uintptr_t)data_u8 - (uintptr_t)data;
}

void GFDFile::destroy()
{
    for (u32 i = 0; i < mTextures.size(); i++)
    {
        GX2Texture& texture = mTextures[i];
        if (texture.surface.imagePtr)
            delete[] (u8*)texture.surface.imagePtr;
        if (texture.surface.mipPtr)
            delete[] (u8*)texture.surface.mipPtr;
    }

    mTextures.clear();

    for (u32 i = 0; i < mVertexShaders.size(); i++)
    {
        GX2VertexShader& shader = mVertexShaders[i];

        if (shader.shaderPtr)
            delete[] (u8*)shader.shaderPtr;

        if (shader.uniformBlocks)
        {
            for (u32 j = 0; j < shader.numUniformBlocks; j++)
                delete[] shader.uniformBlocks[j].name;

            delete[] shader.uniformBlocks;
        }

        if (shader.uniformVars)
        {
            for (u32 j = 0; j < shader.numUniforms; j++)
                delete[] shader.uniformVars[j].name;

            delete[] shader.uniformVars;
        }

        if (shader.initialValues)
            delete[] shader.initialValues;

        if (shader.loopVars)
            delete[] (u32*)shader.loopVars;

        if (shader.samplerVars)
        {
            for (u32 j = 0; j < shader.numSamplers; j++)
                delete[] shader.samplerVars[j].name;

            delete[] shader.samplerVars;
        }

        if (shader.attribVars)
        {
            for (u32 j = 0; j < shader.numAttribs; j++)
                delete[] shader.attribVars[j].name;

            delete[] shader.attribVars;
        }
    }

    mVertexShaders.clear();

    for (u32 i = 0; i < mPixelShaders.size(); i++)
    {
        GX2PixelShader& shader = mPixelShaders[i];

        if (shader.shaderPtr)
            delete[] (u8*)shader.shaderPtr;

        if (shader.uniformBlocks)
        {
            for (u32 j = 0; j < shader.numUniformBlocks; j++)
                delete[] shader.uniformBlocks[j].name;

            delete[] shader.uniformBlocks;
        }

        if (shader.uniformVars)
        {
            for (u32 j = 0; j < shader.numUniforms; j++)
                delete[] shader.uniformVars[j].name;

            delete[] shader.uniformVars;
        }

        if (shader.initialValues)
            delete[] shader.initialValues;

        if (shader.loopVars)
            delete[] (u32*)shader.loopVars;

        if (shader.samplerVars)
        {
            for (u32 j = 0; j < shader.numSamplers; j++)
                delete[] shader.samplerVars[j].name;

            delete[] shader.samplerVars;
        }
    }

    mPixelShaders.clear();

    for (u32 i = 0; i < mGeometryShaders.size(); i++)
    {
        GX2GeometryShader& shader = mGeometryShaders[i];

        if (shader.shaderPtr)
            delete[] (u8*)shader.shaderPtr;

        if (shader.copyShaderPtr)
            delete[] (u8*)shader.copyShaderPtr;

        if (shader.uniformBlocks)
        {
            for (u32 j = 0; j < shader.numUniformBlocks; j++)
                delete[] shader.uniformBlocks[j].name;

            delete[] shader.uniformBlocks;
        }

        if (shader.uniformVars)
        {
            for (u32 j = 0; j < shader.numUniforms; j++)
                delete[] shader.uniformVars[j].name;

            delete[] shader.uniformVars;
        }

        if (shader.initialValues)
            delete[] shader.initialValues;

        if (shader.loopVars)
            delete[] (u32*)shader.loopVars;

        if (shader.samplerVars)
        {
            for (u32 j = 0; j < shader.numSamplers; j++)
                delete[] shader.samplerVars[j].name;

            delete[] shader.samplerVars;
        }
    }

    mGeometryShaders.clear();

  //mComputeShaders.clear();
}
