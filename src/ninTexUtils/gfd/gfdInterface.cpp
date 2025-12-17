#include <ninTexUtils/gfd/gfdStruct.h>

#include <cassert>
#include <cstring>

extern "C"
{

void GFDHeaderVerifyForSerialization(const GFDHeader* header)
{
    assert(header->magic        == 0x47667832u); // Gfx2
    assert(header->size         == sizeof(GFDHeader));
    assert(header->majorVersion == 6 ||
           header->majorVersion == 7);
    assert(header->gpuVersion   == GFD_GPU_VERSION_GPU7);
}

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
        GFDHeaderVerifyForSerialization(dst);

        if (dst->majorVersion == 6 && dst->minorVersion == 0)
            dst->alignMode = GFD_ALIGN_MODE_UNDEF;
    }
}

void GFDBlockHeaderVerifyForSerialization(const GFDBlockHeader* block)
{
    assert(block->magic        == 0x424C4B7Bu); // BLK{
    assert(block->size         == sizeof(GFDBlockHeader));
    assert(block->majorVersion == 0 ||
           block->majorVersion == 1);
    assert(block->type         != GFD_BLOCK_TYPE_INVALID);
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
        GFDBlockHeaderVerifyForSerialization(dst);

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
            assert(currentVertexShader != NULL && currentVertexShader->shaderPtr.getOffset() == 0);
            assert(blockDataSize == currentVertexShader->shaderSize);
            currentVertexShader->shaderPtr.set(new u8[blockDataSize]);
            std::memcpy(currentVertexShader->shaderPtr.get(), data_u8, blockDataSize);
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
            assert(currentPixelShader != NULL && currentPixelShader->shaderPtr.getOffset() == 0);
            assert(blockDataSize == currentPixelShader->shaderSize);
            currentPixelShader->shaderPtr.set(new u8[blockDataSize]);
            std::memcpy(currentPixelShader->shaderPtr.get(), data_u8, blockDataSize);
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
            assert(currentGeometryShader != NULL && currentGeometryShader->shaderPtr.getOffset() == 0);
            assert(blockDataSize == currentGeometryShader->shaderSize);
            currentGeometryShader->shaderPtr.set(new u8[blockDataSize]);
            std::memcpy(currentGeometryShader->shaderPtr.get(), data_u8, blockDataSize);
        }
        else if ((blockVersion == 0 && blockTypeV0 == GFD_BLOCK_TYPE_V0_GX2_GS_COPY_PROGRAM) ||
                 (blockVersion == 1 && blockTypeV1 == GFD_BLOCK_TYPE_V1_GX2_GS_COPY_PROGRAM))
        {
            assert(currentGeometryShader != NULL && currentGeometryShader->copyShaderPtr.getOffset() == 0);
            assert(blockDataSize == currentGeometryShader->copyShaderSize);
            currentGeometryShader->copyShaderPtr.set(new u8[blockDataSize]);
            std::memcpy(currentGeometryShader->copyShaderPtr.get(), data_u8, blockDataSize);
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
            assert(currentTexture != NULL && currentTexture->surface.imagePtr.getOffset() == 0);
            assert(blockDataSize == currentTexture->surface.imageSize);
            currentTexture->surface.imagePtr.set(new u8[blockDataSize]);
            std::memcpy(currentTexture->surface.imagePtr.get(), data_u8, blockDataSize);
        }
        else if ((blockVersion == 0 && blockTypeV0 == GFD_BLOCK_TYPE_V0_GX2_TEX_MIP_DATA) ||
                 (blockVersion == 1 && blockTypeV1 == GFD_BLOCK_TYPE_V1_GX2_TEX_MIP_DATA))
        {
            assert(currentTexture != NULL && currentTexture->surface.mipPtr.getOffset() == 0);
            assert(blockDataSize == currentTexture->surface.mipSize);
            currentTexture->surface.mipPtr.set(new u8[blockDataSize]);
            std::memcpy(currentTexture->surface.mipPtr.get(), data_u8, blockDataSize);
        }

        data_u8 += blockDataSize;
    }

    if (searchAlignmentBlock)
        mHeader.alignMode = GFD_ALIGN_MODE_DISABLE;

    return (uintptr_t)data_u8 - (uintptr_t)data;
}

static inline void BufferAppend_GFDHeader(std::vector<u8>& buffer, const GFDHeader& header)
{
    size_t pos = buffer.size();
    buffer.resize(pos + sizeof(GFDHeader));
    SaveGFDHeader((GFDHeader*)(buffer.data() + pos), &header);
}

static inline void BufferAppend_GFDBlockHeader(std::vector<u8>& buffer, const GFDBlockHeader& blockHeader)
{
    size_t pos = buffer.size();
    buffer.resize(pos + sizeof(GFDBlockHeader));
    SaveGFDBlockHeader((GFDBlockHeader*)(buffer.data() + pos), &blockHeader);
}

static inline void BufferAppend_GX2Texture(std::vector<u8>& buffer, const GX2Texture& texture)
{
    size_t pos = buffer.size();
    buffer.resize(pos + sizeof(GX2Texture));
    SaveGX2Texture((GX2Texture*)(buffer.data() + pos), &texture);
}

static inline void BufferAppend_Span(std::vector<u8>& buffer, const void* ptr, size_t size)
{
    size_t pos = buffer.size();
    buffer.resize(pos + size);
    std::memcpy(buffer.data() + pos, ptr, size);;
}

static inline size_t RoundUpSize(size_t x, size_t y)
{
    return ((x - 1) | (y - 1)) + 1;
}

static inline void BufferAppend_GFDBlockHeader_Pad(std::vector<u8>& buffer, GFDBlockHeader& blockHeader, u32 alignment)
{
    //   Calculate the needed pad
    const size_t dataPos = buffer.size() + sizeof(GFDBlockHeader) * 2;
    const size_t padSize = RoundUpSize(dataPos, alignment) - dataPos;

    blockHeader.type = GFD_BLOCK_TYPE_PAD;
    blockHeader.dataSize = padSize;

    BufferAppend_GFDBlockHeader(buffer, blockHeader);
    buffer.resize(buffer.size() + padSize);
}

std::vector<u8> GFDFile::saveGTX() const
{
    GFDBlockHeader blockHeader;
    blockHeader.magic = 0x424C4B7Bu; // BLK{
    blockHeader.size = sizeof(GFDBlockHeader);
    // Determine the usual block header version from the file version
    if (mHeader.majorVersion == 6 && mHeader.minorVersion == 0)
    {
        blockHeader.majorVersion = 0;
        blockHeader.minorVersion = 1;
    }
    else
    {
        blockHeader.majorVersion = 1;
        blockHeader.minorVersion = 0;
    }

    // Check alignment
    assert(mHeader.alignMode != GFD_ALIGN_MODE_UNDEF && "Please choose an alignment mode before saving.");
    const bool align = mHeader.alignMode == GFD_ALIGN_MODE_ENABLE;

    std::vector<u8> outBuffer;

    BufferAppend_GFDHeader(outBuffer, mHeader);

    for (const GX2Texture& texture : mTextures)
    {
        // Write GX2Texture Header block
        if (blockHeader.majorVersion == 1)
            blockHeader.typeV1 = GFD_BLOCK_TYPE_V1_GX2_TEX_HEADER;
        else
            blockHeader.typeV0 = GFD_BLOCK_TYPE_V0_GX2_TEX_HEADER;
        blockHeader.dataSize = sizeof(GX2Texture);

        BufferAppend_GFDBlockHeader(outBuffer, blockHeader);
        BufferAppend_GX2Texture(outBuffer, texture);

        // Write Pad block for the image data
        if (align)
            BufferAppend_GFDBlockHeader_Pad(outBuffer, blockHeader, texture.surface.alignment);

        if (blockHeader.majorVersion == 1)
            blockHeader.typeV1 = GFD_BLOCK_TYPE_V1_GX2_TEX_IMAGE_DATA;
        else
            blockHeader.typeV0 = GFD_BLOCK_TYPE_V0_GX2_TEX_IMAGE_DATA;
        blockHeader.dataSize = texture.surface.imageSize;

        BufferAppend_GFDBlockHeader(outBuffer, blockHeader);
        BufferAppend_Span(outBuffer, texture.surface.imagePtr.get(), texture.surface.imageSize);

        if (texture.surface.mipPtr.get())
        {
            // Write Pad block for the mipmap data
            if (align)
                BufferAppend_GFDBlockHeader_Pad(outBuffer, blockHeader, texture.surface.alignment);

            if (blockHeader.majorVersion == 1)
                blockHeader.typeV1 = GFD_BLOCK_TYPE_V1_GX2_TEX_MIP_DATA;
            else
                blockHeader.typeV0 = GFD_BLOCK_TYPE_V0_GX2_TEX_MIP_DATA;
            blockHeader.dataSize = texture.surface.mipSize;

            BufferAppend_GFDBlockHeader(outBuffer, blockHeader);
            BufferAppend_Span(outBuffer, texture.surface.mipPtr.get(), texture.surface.mipSize);
        }
    }

    blockHeader.type = GFD_BLOCK_TYPE_END;
    blockHeader.dataSize = 0;

    BufferAppend_GFDBlockHeader(outBuffer, blockHeader);

    return outBuffer;
}

void GFDFile::destroy()
{
    for (u32 i = 0; i < mTextures.size(); i++)
    {
        GX2Texture& texture = mTextures[i];
        if (texture.surface.imagePtr.get())
            delete[] (u8*)texture.surface.imagePtr.get();
        if (texture.surface.mipPtr.get())
            delete[] (u8*)texture.surface.mipPtr.get();
    }

    mTextures.clear();

    for (u32 i = 0; i < mVertexShaders.size(); i++)
    {
        GX2VertexShader& shader = mVertexShaders[i];

        if (shader.shaderPtr.get())
            delete[] (u8*)shader.shaderPtr.get();

        if (shader.uniformBlocks.get())
        {
            for (u32 j = 0; j < shader.numUniformBlocks; j++)
                delete[] shader.uniformBlocks.getIndexed(j)->name.get();

            delete[] shader.uniformBlocks.get();
        }

        if (shader.uniformVars.get())
        {
            for (u32 j = 0; j < shader.numUniforms; j++)
                delete[] shader.uniformVars.getIndexed(j)->name.get();

            delete[] shader.uniformVars.get();
        }

        if (shader.initialValues.get())
            delete[] shader.initialValues.get();

        if (shader._loopVars.get())
            delete[] (u32*)shader._loopVars.get();

        if (shader.samplerVars.get())
        {
            for (u32 j = 0; j < shader.numSamplers; j++)
                delete[] shader.samplerVars.getIndexed(j)->name.get();

            delete[] shader.samplerVars.get();
        }

        if (shader.attribVars.get())
        {
            for (u32 j = 0; j < shader.numAttribs; j++)
                delete[] shader.attribVars.getIndexed(j)->name.get();

            delete[] shader.attribVars.get();
        }
    }

    mVertexShaders.clear();

    for (u32 i = 0; i < mPixelShaders.size(); i++)
    {
        GX2PixelShader& shader = mPixelShaders[i];

        if (shader.shaderPtr.get())
            delete[] (u8*)shader.shaderPtr.get();

        if (shader.uniformBlocks.get())
        {
            for (u32 j = 0; j < shader.numUniformBlocks; j++)
                delete[] shader.uniformBlocks.getIndexed(j)->name.get();

            delete[] shader.uniformBlocks.get();
        }

        if (shader.uniformVars.get())
        {
            for (u32 j = 0; j < shader.numUniforms; j++)
                delete[] shader.uniformVars.getIndexed(j)->name.get();

            delete[] shader.uniformVars.get();
        }

        if (shader.initialValues.get())
            delete[] shader.initialValues.get();

        if (shader._loopVars.get())
            delete[] (u32*)shader._loopVars.get();

        if (shader.samplerVars.get())
        {
            for (u32 j = 0; j < shader.numSamplers; j++)
                delete[] shader.samplerVars.getIndexed(j)->name.get();

            delete[] shader.samplerVars.get();
        }
    }

    mPixelShaders.clear();

    for (u32 i = 0; i < mGeometryShaders.size(); i++)
    {
        GX2GeometryShader& shader = mGeometryShaders[i];

        if (shader.shaderPtr.get())
            delete[] (u8*)shader.shaderPtr.get();

        if (shader.copyShaderPtr.get())
            delete[] (u8*)shader.copyShaderPtr.get();

        if (shader.uniformBlocks.get())
        {
            for (u32 j = 0; j < shader.numUniformBlocks; j++)
                delete[] shader.uniformBlocks.getIndexed(j)->name.get();

            delete[] shader.uniformBlocks.get();
        }

        if (shader.uniformVars.get())
        {
            for (u32 j = 0; j < shader.numUniforms; j++)
                delete[] shader.uniformVars.getIndexed(j)->name.get();

            delete[] shader.uniformVars.get();
        }

        if (shader.initialValues.get())
            delete[] shader.initialValues.get();

        if (shader._loopVars.get())
            delete[] (u32*)shader._loopVars.get();

        if (shader.samplerVars.get())
        {
            for (u32 j = 0; j < shader.numSamplers; j++)
                delete[] shader.samplerVars.getIndexed(j)->name.get();

            delete[] shader.samplerVars.get();
        }
    }

    mGeometryShaders.clear();

  //mComputeShaders.clear();
}
