#include <ninTexUtils/gx2/gx2Shaders.h>

#include <cassert>
#include <cstring>

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstrict-aliasing"

static inline void Swap32(void* dst, const void* src)
{
    *(u32*)dst = __builtin_bswap32(*(u32*)src);
}

static inline void Swap32(void* dst, const void* src, u32 count)
{
    for (u32 i = 0; i < count; i++)
        Swap32((u32*)dst + i, (u32*)src + i);
}

static void LoadGX2UniformBlock(const void* data, const void* baseSrc, GX2UniformBlock* uniformBlocks, const void* baseDst, u32 count, bool allocate, bool isBigEndian)
{
    const GX2UniformBlock* src = (const GX2UniformBlock*)data;
    GX2UniformBlock* dst = uniformBlocks;

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        for (u32 i = 0; i < count; i++)
        {
            Swap32(&dst[i].name,     &src[i].name);
            Swap32(&dst[i].location, &src[i].location);
            Swap32(&dst[i].size,     &src[i].size);
        }
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GX2UniformBlock) * count);
    }

    for (u32 i = 0; i < count; i++)
    {
        const char* srcName;
        *(uintptr_t*)&srcName = dst[i].name.getOffset() != 0 ? (uintptr_t)baseSrc + (dst[i].name.getOffset() & 0xFFFFFu)
                                                             : (uintptr_t)NULL;

        size_t nameLen = srcName != NULL ? std::strlen(srcName) : 0;

        if (nameLen == 0)
            dst[i].name.set(NULL);

        else if (allocate)
            dst[i].name.set(new char[nameLen + 1]);

        else
            dst[i].name.resolveRelativePtr(baseDst, 0xFFFFFu);

        char* dstName = (char*)dst[i].name.get();

        if (srcName != dstName && nameLen != 0)
        {
            std::memmove(dstName, srcName, nameLen);
            dstName[nameLen] = '\0';
        }
    }
}

static void LoadGX2UniformVar(const void* data, const void* baseSrc, GX2UniformVar* uniformVars, const void* baseDst, u32 count, bool allocate, bool isBigEndian)
{
    const GX2UniformVar* src = (const GX2UniformVar*)data;
    GX2UniformVar* dst = uniformVars;

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        for (u32 i = 0; i < count; i++)
        {
            Swap32(&dst[i].name,       &src[i].name);
            Swap32(&dst[i].type,       &src[i].type);
            Swap32(&dst[i].arrayCount, &src[i].arrayCount);
            Swap32(&dst[i].offset,     &src[i].offset);
            Swap32(&dst[i].blockIndex, &src[i].blockIndex);
        }
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GX2UniformVar) * count);
    }

    for (u32 i = 0; i < count; i++)
    {
        const char* srcName;
        *(uintptr_t*)&srcName = dst[i].name.getOffset() != 0 ? (uintptr_t)baseSrc + (dst[i].name.getOffset() & 0xFFFFFu)
                                                             : (uintptr_t)NULL;

        size_t nameLen = srcName != NULL ? std::strlen(srcName) : 0;

        if (nameLen == 0)
            dst[i].name.set(NULL);

        else if (allocate)
            dst[i].name.set(new char[nameLen + 1]);

        else
            dst[i].name.resolveRelativePtr(baseDst, 0xFFFFFu);

        char* dstName = (char*)dst[i].name.get();

        if (srcName != dstName && nameLen != 0)
        {
            std::memmove(dstName, srcName, nameLen);
            dstName[nameLen] = '\0';
        }
    }
}

static void LoadGX2UniformInitialValue(const void* data, const void* baseSrc, GX2UniformInitialValue* initialValues, const void* baseDst, u32 count, bool allocate, bool isBigEndian)
{
    const GX2UniformInitialValue* src = (const GX2UniformInitialValue*)data;
    GX2UniformInitialValue* dst = initialValues;

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        for (u32 i = 0; i < count; i++)
        {
            Swap32(&dst[i].value,  &src[i].value,   4);
            Swap32(&dst[i].offset, &src[i].offset);
        }
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GX2UniformInitialValue) * count);
    }
}

static void LoadGX2LoopVar(const void* data, const void* baseSrc, void* loopVars, const void* baseDst, u32 count, bool allocate, bool isBigEndian)
{
    const u32* src = (const u32*)data;
    u32* dst = (u32*)loopVars;

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        for (u32 i = 0; i < count; i++)
        {
            Swap32(&dst[i * 2], &src[i * 2], 2);
        }
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(u32) * 2 * count);
    }
}

static void LoadGX2SamplerVar(const void* data, const void* baseSrc, GX2SamplerVar* samplerVars, const void* baseDst, u32 count, bool allocate, bool isBigEndian)
{
    const GX2SamplerVar* src = (const GX2SamplerVar*)data;
    GX2SamplerVar* dst = samplerVars;

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        for (u32 i = 0; i < count; i++)
        {
            Swap32(&dst[i].name,     &src[i].name);
            Swap32(&dst[i].type,     &src[i].type);
            Swap32(&dst[i].location, &src[i].location);
        }
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GX2SamplerVar) * count);
    }

    for (u32 i = 0; i < count; i++)
    {
        const char* srcName;
        *(uintptr_t*)&srcName = dst[i].name.getOffset() != 0 ? (uintptr_t)baseSrc + (dst[i].name.getOffset() & 0xFFFFFu)
                                                             : (uintptr_t)NULL;

        size_t nameLen = srcName != NULL ? std::strlen(srcName) : 0;

        if (nameLen == 0)
            dst[i].name.set(NULL);

        else if (allocate)
            dst[i].name.set(new char[nameLen + 1]);

        else
            dst[i].name.resolveRelativePtr(baseDst, 0xFFFFFu);

        char* dstName = (char*)dst[i].name.get();

        if (srcName != dstName && nameLen != 0)
        {
            std::memmove(dstName, srcName, nameLen);
            dstName[nameLen] = '\0';
        }
    }
}

static void LoadGX2AttribVar(const void* data, const void* baseSrc, GX2AttribVar* attribVars, const void* baseDst, u32 count, bool allocate, bool isBigEndian)
{
    const GX2AttribVar* src = (const GX2AttribVar*)data;
    GX2AttribVar* dst = attribVars;

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        for (u32 i = 0; i < count; i++)
        {
            Swap32(&dst[i].name,       &src[i].name);
            Swap32(&dst[i].type,       &src[i].type);
            Swap32(&dst[i].arrayCount, &src[i].arrayCount);
            Swap32(&dst[i].location,   &src[i].location);
        }
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GX2AttribVar) * count);
    }

    for (u32 i = 0; i < count; i++)
    {
        const char* srcName;
        *(uintptr_t*)&srcName = dst[i].name.getOffset() != 0 ? (uintptr_t)baseSrc + (dst[i].name.getOffset() & 0xFFFFFu)
                                                             : (uintptr_t)NULL;

        size_t nameLen = srcName != NULL ? std::strlen(srcName) : 0;

        if (nameLen == 0)
            dst[i].name.set(NULL);

        else if (allocate)
            dst[i].name.set(new char[nameLen + 1]);

        else
            dst[i].name.resolveRelativePtr(baseDst, 0xFFFFFu);

        char* dstName = (char*)dst[i].name.get();

        if (srcName != dstName && nameLen != 0)
        {
            std::memmove(dstName, srcName, nameLen);
            dstName[nameLen] = '\0';
        }
    }
}

extern "C"
{

void LoadGX2VertexShader(const void* data, GX2VertexShader* shader, bool allocate, bool isBigEndian)
{
    const GX2VertexShader* src = (const GX2VertexShader*)data;
    GX2VertexShader* dst = shader;

    assert(src != NULL);
    assert(dst != NULL);

    allocate = allocate && src != dst;

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        Swap32(&dst->_regs,                       &src->_regs,                       52);
        Swap32(&dst->shaderSize,                  &src->shaderSize);
        Swap32(&dst->shaderPtr,                   &src->shaderPtr);
        Swap32(&dst->shaderMode,                  &src->shaderMode);
        Swap32(&dst->numUniformBlocks,            &src->numUniformBlocks);
        Swap32(&dst->uniformBlocks,               &src->uniformBlocks);
        Swap32(&dst->numUniforms,                 &src->numUniforms);
        Swap32(&dst->uniformVars,                 &src->uniformVars);
        Swap32(&dst->numInitialValues,            &src->numInitialValues);
        Swap32(&dst->initialValues,               &src->initialValues);
        Swap32(&dst->_numLoops,                   &src->_numLoops);
        Swap32(&dst->_loopVars,                   &src->_loopVars);
        Swap32(&dst->numSamplers,                 &src->numSamplers);
        Swap32(&dst->samplerVars,                 &src->samplerVars);
        Swap32(&dst->numAttribs,                  &src->numAttribs);
        Swap32(&dst->attribVars,                  &src->attribVars);
        Swap32(&dst->ringItemsize,                &src->ringItemsize);
        Swap32(&dst->hasStreamOut,                &src->hasStreamOut);
        Swap32(&dst->streamOutVertexStride,       &src->streamOutVertexStride,        4);
        Swap32(&dst->shaderProgram.resourceFlags, &src->shaderProgram.resourceFlags);
        Swap32(&dst->shaderProgram.elementSize,   &src->shaderProgram.elementSize);
        Swap32(&dst->shaderProgram.elementCount,  &src->shaderProgram.elementCount);
        Swap32(&dst->shaderProgram._c,            &src->shaderProgram._c);
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GX2VertexShader));
    }

    if (dst->numUniformBlocks != 0)
        assert(dst->uniformBlocks.getOffset() != 0);
    else
        assert(dst->uniformBlocks.getOffset() == 0);

    if (dst->numUniforms != 0)
        assert(dst->uniformVars.getOffset() != 0);
    else
        assert(dst->uniformVars.getOffset() == 0);

    if (dst->numInitialValues != 0)
        assert(dst->initialValues.getOffset() != 0);
    else
        assert(dst->initialValues.getOffset() == 0);

    if (dst->_numLoops != 0)
        assert(dst->_loopVars.getOffset() != 0);
    else
        assert(dst->_loopVars.getOffset() == 0);

    if (dst->numSamplers != 0)
        assert(dst->samplerVars.getOffset() != 0);
    else
        assert(dst->samplerVars.getOffset() == 0);

    if (dst->numAttribs != 0)
        assert(dst->attribVars.getOffset() != 0);
    else
        assert(dst->attribVars.getOffset() == 0);

    GX2UniformBlock* srcUniformBlocks;
    GX2UniformVar* srcUniformVars;
    GX2UniformInitialValue* srcInitialValues;
    void* srcLoopVars;
    GX2SamplerVar* srcSamplerVars;
    GX2AttribVar* srcAttribVars;

    *(uintptr_t*)&srcUniformBlocks = dst->numUniformBlocks != 0 ? (uintptr_t)src + (dst->uniformBlocks.getOffset() & 0xFFFFFu)
                                                                : (uintptr_t)NULL;

    *(uintptr_t*)&srcUniformVars = dst->numUniforms != 0 ? (uintptr_t)src + (dst->uniformVars.getOffset() & 0xFFFFFu)
                                                         : (uintptr_t)NULL;

    *(uintptr_t*)&srcInitialValues = dst->numInitialValues != 0 ? (uintptr_t)src + (dst->initialValues.getOffset() & 0xFFFFFu)
                                                                : (uintptr_t)NULL;

    *(uintptr_t*)&srcLoopVars = dst->_numLoops != 0 ? (uintptr_t)src + (dst->_loopVars.getOffset() & 0xFFFFFu)
                                                    : (uintptr_t)NULL;

    *(uintptr_t*)&srcSamplerVars = dst->numSamplers != 0 ? (uintptr_t)src + (dst->samplerVars.getOffset() & 0xFFFFFu)
                                                         : (uintptr_t)NULL;

    *(uintptr_t*)&srcAttribVars = dst->numAttribs != 0 ? (uintptr_t)src + (dst->attribVars.getOffset() & 0xFFFFFu)
                                                       : (uintptr_t)NULL;

    if (allocate)
    {
        if (dst->numUniformBlocks != 0)
            dst->uniformBlocks.set(new GX2UniformBlock[dst->numUniformBlocks]);

        if (dst->numUniforms != 0)
            dst->uniformVars.set(new GX2UniformVar[dst->numUniforms]);

        if (dst->numInitialValues != 0)
            dst->initialValues.set(new GX2UniformInitialValue[dst->numInitialValues]);

        if (dst->_numLoops != 0)
            dst->_loopVars.set(new u32[2 * dst->_numLoops]);

        if (dst->numSamplers != 0)
            dst->samplerVars.set(new GX2SamplerVar[dst->numSamplers]);

        if (dst->numAttribs != 0)
            dst->attribVars.set(new GX2AttribVar[dst->numAttribs]);
    }
    else
    {
        if (dst->uniformBlocks.getOffset() != 0)
            dst->uniformBlocks.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->uniformVars.getOffset() != 0)
            dst->uniformVars.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->initialValues.getOffset() != 0)
            dst->initialValues.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->_loopVars.getOffset() != 0)
            dst->_loopVars.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->samplerVars.getOffset() != 0)
            dst->samplerVars.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->attribVars.getOffset() != 0)
            dst->attribVars.resolveRelativePtr(dst, 0xFFFFFu);
    }

    if (dst->numUniformBlocks != 0)
        LoadGX2UniformBlock(srcUniformBlocks, src, dst->uniformBlocks.get(), dst, dst->numUniformBlocks, allocate, isBigEndian);
    if (dst->numUniforms != 0)
        LoadGX2UniformVar(srcUniformVars, src, dst->uniformVars.get(), dst, dst->numUniforms, allocate, isBigEndian);
    if (dst->numInitialValues != 0)
        LoadGX2UniformInitialValue(srcInitialValues, src, dst->initialValues.get(), dst, dst->numInitialValues, allocate, isBigEndian);
    if (dst->_numLoops != 0)
        LoadGX2LoopVar(srcLoopVars, src, dst->_loopVars.get(), dst, dst->_numLoops, allocate, isBigEndian);
    if (dst->numSamplers != 0)
        LoadGX2SamplerVar(srcSamplerVars, src, dst->samplerVars.get(), dst, dst->numSamplers, allocate, isBigEndian);
    if (dst->numAttribs != 0)
        LoadGX2AttribVar(srcAttribVars, src, dst->attribVars.get(), dst, dst->numAttribs, allocate, isBigEndian);
}

void LoadGX2PixelShader(const void* data, GX2PixelShader* shader, bool allocate, bool isBigEndian)
{
    const GX2PixelShader* src = (const GX2PixelShader*)data;
    GX2PixelShader* dst = shader;

    assert(src != NULL);
    assert(dst != NULL);

    allocate = allocate && src != dst;

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        Swap32(&dst->_regs,                       &src->_regs,                       41);
        Swap32(&dst->shaderSize,                  &src->shaderSize);
        Swap32(&dst->shaderPtr,                   &src->shaderPtr);
        Swap32(&dst->shaderMode,                  &src->shaderMode);
        Swap32(&dst->numUniformBlocks,            &src->numUniformBlocks);
        Swap32(&dst->uniformBlocks,               &src->uniformBlocks);
        Swap32(&dst->numUniforms,                 &src->numUniforms);
        Swap32(&dst->uniformVars,                 &src->uniformVars);
        Swap32(&dst->numInitialValues,            &src->numInitialValues);
        Swap32(&dst->initialValues,               &src->initialValues);
        Swap32(&dst->_numLoops,                   &src->_numLoops);
        Swap32(&dst->_loopVars,                   &src->_loopVars);
        Swap32(&dst->numSamplers,                 &src->numSamplers);
        Swap32(&dst->samplerVars,                 &src->samplerVars);
        Swap32(&dst->shaderProgram.resourceFlags, &src->shaderProgram.resourceFlags);
        Swap32(&dst->shaderProgram.elementSize,   &src->shaderProgram.elementSize);
        Swap32(&dst->shaderProgram.elementCount,  &src->shaderProgram.elementCount);
        Swap32(&dst->shaderProgram._c,            &src->shaderProgram._c);
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GX2PixelShader));
    }

    if (dst->numUniformBlocks != 0)
        assert(dst->uniformBlocks.getOffset() != 0);
    else
        assert(dst->uniformBlocks.getOffset() == 0);

    if (dst->numUniforms != 0)
        assert(dst->uniformVars.getOffset() != 0);
    else
        assert(dst->uniformVars.getOffset() == 0);

    if (dst->numInitialValues != 0)
        assert(dst->initialValues.getOffset() != 0);
    else
        assert(dst->initialValues.getOffset() == 0);

    if (dst->_numLoops != 0)
        assert(dst->_loopVars.getOffset() != 0);
    else
        assert(dst->_loopVars.getOffset() == 0);

    if (dst->numSamplers != 0)
        assert(dst->samplerVars.getOffset() != 0);
    else
        assert(dst->samplerVars.getOffset() == 0);

    GX2UniformBlock* srcUniformBlocks;
    GX2UniformVar* srcUniformVars;
    GX2UniformInitialValue* srcInitialValues;
    void* srcLoopVars;
    GX2SamplerVar* srcSamplerVars;

    *(uintptr_t*)&srcUniformBlocks = dst->numUniformBlocks != 0 ? (uintptr_t)src + (dst->uniformBlocks.getOffset() & 0xFFFFFu)
                                                                : (uintptr_t)NULL;

    *(uintptr_t*)&srcUniformVars = dst->numUniforms != 0 ? (uintptr_t)src + (dst->uniformVars.getOffset() & 0xFFFFFu)
                                                         : (uintptr_t)NULL;

    *(uintptr_t*)&srcInitialValues = dst->numInitialValues != 0 ? (uintptr_t)src + (dst->initialValues.getOffset() & 0xFFFFFu)
                                                                : (uintptr_t)NULL;

    *(uintptr_t*)&srcLoopVars = dst->_numLoops != 0 ? (uintptr_t)src + (dst->_loopVars.getOffset() & 0xFFFFFu)
                                                    : (uintptr_t)NULL;

    *(uintptr_t*)&srcSamplerVars = dst->numSamplers != 0 ? (uintptr_t)src + (dst->samplerVars.getOffset() & 0xFFFFFu)
                                                         : (uintptr_t)NULL;

    if (allocate)
    {
        if (dst->numUniformBlocks != 0)
            dst->uniformBlocks.set(new GX2UniformBlock[dst->numUniformBlocks]);

        if (dst->numUniforms != 0)
            dst->uniformVars.set(new GX2UniformVar[dst->numUniforms]);

        if (dst->numInitialValues != 0)
            dst->initialValues.set(new GX2UniformInitialValue[dst->numInitialValues]);

        if (dst->_numLoops != 0)
            dst->_loopVars.set(new u32[2 * dst->_numLoops]);

        if (dst->numSamplers != 0)
            dst->samplerVars.set(new GX2SamplerVar[dst->numSamplers]);
    }
    else
    {
        if (dst->uniformBlocks.getOffset() != 0)
            dst->uniformBlocks.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->uniformVars.getOffset() != 0)
            dst->uniformVars.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->initialValues.getOffset() != 0)
            dst->initialValues.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->_loopVars.getOffset() != 0)
            dst->_loopVars.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->samplerVars.getOffset() != 0)
            dst->samplerVars.resolveRelativePtr(dst, 0xFFFFFu);
    }

    if (dst->numUniformBlocks != 0)
        LoadGX2UniformBlock(srcUniformBlocks, src, dst->uniformBlocks.get(), dst, dst->numUniformBlocks, allocate, isBigEndian);
    if (dst->numUniforms != 0)
        LoadGX2UniformVar(srcUniformVars, src, dst->uniformVars.get(), dst, dst->numUniforms, allocate, isBigEndian);
    if (dst->numInitialValues != 0)
        LoadGX2UniformInitialValue(srcInitialValues, src, dst->initialValues.get(), dst, dst->numInitialValues, allocate, isBigEndian);
    if (dst->_numLoops != 0)
        LoadGX2LoopVar(srcLoopVars, src, dst->_loopVars.get(), dst, dst->_numLoops, allocate, isBigEndian);
    if (dst->numSamplers != 0)
        LoadGX2SamplerVar(srcSamplerVars, src, dst->samplerVars.get(), dst, dst->numSamplers, allocate, isBigEndian);
}

void LoadGX2GeometryShader(const void* data, GX2GeometryShader* shader, bool allocate, bool isBigEndian)
{
    const GX2GeometryShader* src = (const GX2GeometryShader*)data;
    GX2GeometryShader* dst = shader;

    assert(src != NULL);
    assert(dst != NULL);

    allocate = allocate && src != dst;

#if (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
    if (isBigEndian)
#else
    if (!isBigEndian)
#endif
    {
        Swap32(&dst->_regs,                           &src->_regs,                           19);
        Swap32(&dst->shaderSize,                      &src->shaderSize);
        Swap32(&dst->shaderPtr,                       &src->shaderPtr);
        Swap32(&dst->copyShaderSize,                  &src->copyShaderSize);
        Swap32(&dst->copyShaderPtr,                   &src->copyShaderPtr);
        Swap32(&dst->shaderMode,                      &src->shaderMode);
        Swap32(&dst->numUniformBlocks,                &src->numUniformBlocks);
        Swap32(&dst->uniformBlocks,                   &src->uniformBlocks);
        Swap32(&dst->numUniforms,                     &src->numUniforms);
        Swap32(&dst->uniformVars,                     &src->uniformVars);
        Swap32(&dst->numInitialValues,                &src->numInitialValues);
        Swap32(&dst->initialValues,                   &src->initialValues);
        Swap32(&dst->_numLoops,                       &src->_numLoops);
        Swap32(&dst->_loopVars,                       &src->_loopVars);
        Swap32(&dst->numSamplers,                     &src->numSamplers);
        Swap32(&dst->samplerVars,                     &src->samplerVars);
        Swap32(&dst->ringItemsize,                    &src->ringItemsize);
        Swap32(&dst->hasStreamOut,                    &src->hasStreamOut);
        Swap32(&dst->streamOutVertexStride,           &src->streamOutVertexStride,            4);
        Swap32(&dst->shaderProgram.resourceFlags,     &src->shaderProgram.resourceFlags);
        Swap32(&dst->shaderProgram.elementSize,       &src->shaderProgram.elementSize);
        Swap32(&dst->shaderProgram.elementCount,      &src->shaderProgram.elementCount);
        Swap32(&dst->shaderProgram._c,                &src->shaderProgram._c);
        Swap32(&dst->copyShaderProgram.resourceFlags, &src->copyShaderProgram.resourceFlags);
        Swap32(&dst->copyShaderProgram.elementSize,   &src->copyShaderProgram.elementSize);
        Swap32(&dst->copyShaderProgram.elementCount,  &src->copyShaderProgram.elementCount);
        Swap32(&dst->copyShaderProgram._c,            &src->copyShaderProgram._c);
    }
    else if (src != dst)
    {
        std::memmove(dst, src, sizeof(GX2GeometryShader));
    }

    if (dst->numUniformBlocks != 0)
        assert(dst->uniformBlocks.getOffset() != 0);
    else
        assert(dst->uniformBlocks.getOffset() == 0);

    if (dst->numUniforms != 0)
        assert(dst->uniformVars.getOffset() != 0);
    else
        assert(dst->uniformVars.getOffset() == 0);

    if (dst->numInitialValues != 0)
        assert(dst->initialValues.getOffset() != 0);
    else
        assert(dst->initialValues.getOffset() == 0);

    if (dst->_numLoops != 0)
        assert(dst->_loopVars.getOffset() != 0);
    else
        assert(dst->_loopVars.getOffset() == 0);

    if (dst->numSamplers != 0)
        assert(dst->samplerVars.getOffset() != 0);
    else
        assert(dst->samplerVars.getOffset() == 0);

    GX2UniformBlock* srcUniformBlocks;
    GX2UniformVar* srcUniformVars;
    GX2UniformInitialValue* srcInitialValues;
    void* srcLoopVars;
    GX2SamplerVar* srcSamplerVars;

    *(uintptr_t*)&srcUniformBlocks = dst->numUniformBlocks != 0 ? (uintptr_t)src + (dst->uniformBlocks.getOffset() & 0xFFFFFu)
                                                                : (uintptr_t)NULL;

    *(uintptr_t*)&srcUniformVars = dst->numUniforms != 0 ? (uintptr_t)src + (dst->uniformVars.getOffset() & 0xFFFFFu)
                                                         : (uintptr_t)NULL;

    *(uintptr_t*)&srcInitialValues = dst->numInitialValues != 0 ? (uintptr_t)src + (dst->initialValues.getOffset() & 0xFFFFFu)
                                                                : (uintptr_t)NULL;

    *(uintptr_t*)&srcLoopVars = dst->_numLoops != 0 ? (uintptr_t)src + (dst->_loopVars.getOffset() & 0xFFFFFu)
                                                    : (uintptr_t)NULL;

    *(uintptr_t*)&srcSamplerVars = dst->numSamplers != 0 ? (uintptr_t)src + (dst->samplerVars.getOffset() & 0xFFFFFu)
                                                         : (uintptr_t)NULL;

    if (allocate)
    {
        if (dst->numUniformBlocks != 0)
            dst->uniformBlocks.set(new GX2UniformBlock[dst->numUniformBlocks]);

        if (dst->numUniforms != 0)
            dst->uniformVars.set(new GX2UniformVar[dst->numUniforms]);

        if (dst->numInitialValues != 0)
            dst->initialValues.set(new GX2UniformInitialValue[dst->numInitialValues]);

        if (dst->_numLoops != 0)
            dst->_loopVars.set(new u32[2 * dst->_numLoops]);

        if (dst->numSamplers != 0)
            dst->samplerVars.set(new GX2SamplerVar[dst->numSamplers]);
    }
    else
    {
        if (dst->uniformBlocks.getOffset() != 0)
            dst->uniformBlocks.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->uniformVars.getOffset() != 0)
            dst->uniformVars.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->initialValues.getOffset() != 0)
            dst->initialValues.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->_loopVars.getOffset() != 0)
            dst->_loopVars.resolveRelativePtr(dst, 0xFFFFFu);
        if (dst->samplerVars.getOffset() != 0)
            dst->samplerVars.resolveRelativePtr(dst, 0xFFFFFu);
    }

    if (dst->numUniformBlocks != 0)
        LoadGX2UniformBlock(srcUniformBlocks, src, dst->uniformBlocks.get(), dst, dst->numUniformBlocks, allocate, isBigEndian);
    if (dst->numUniforms != 0)
        LoadGX2UniformVar(srcUniformVars, src, dst->uniformVars.get(), dst, dst->numUniforms, allocate, isBigEndian);
    if (dst->numInitialValues != 0)
        LoadGX2UniformInitialValue(srcInitialValues, src, dst->initialValues.get(), dst, dst->numInitialValues, allocate, isBigEndian);
    if (dst->_numLoops != 0)
        LoadGX2LoopVar(srcLoopVars, src, dst->_loopVars.get(), dst, dst->_numLoops, allocate, isBigEndian);
    if (dst->numSamplers != 0)
        LoadGX2SamplerVar(srcSamplerVars, src, dst->samplerVars.get(), dst, dst->numSamplers, allocate, isBigEndian);
}

}

#pragma GCC diagnostic pop
