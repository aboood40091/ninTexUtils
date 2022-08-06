#ifndef NIN_TEX_UTILS_GX2_SHADERS_H_
#define NIN_TEX_UTILS_GX2_SHADERS_H_

#include "gx2Enum.h"

typedef enum _GX2RResourceFlags
{
    GX2R_RESOURCE_FLAGS_NONE
}
GX2RResourceFlags;
static_assert(sizeof(GX2RResourceFlags) == 4, "GX2RResourceFlags size mismatch");

typedef struct _GX2RBuffer
{
    GX2RResourceFlags resourceFlags;
    u32 elementSize;
    u32 elementCount;
    u32 _c;
}
GX2RBuffer;
static_assert(sizeof(GX2RBuffer) == 0x10, "GX2RBuffer size mismatch");

typedef struct _GX2UniformBlock
{
    const char* name;
    u32 location;
    u32 size;
}
GX2UniformBlock;
static_assert(sizeof(GX2UniformBlock) == 0xC, "GX2UniformBlock size mismatch");

typedef struct _GX2UniformVar
{
    const char* name;
    GX2VarType type;
    u32 arrayCount;
    u32 offset;
    u32 blockIndex;
}
GX2UniformVar;
static_assert(sizeof(GX2UniformVar) == 0x14, "GX2UniformVar size mismatch");

typedef struct _GX2UniformInitialValue
{
    f32 value[4];
    u32 offset;
}
GX2UniformInitialValue;
static_assert(sizeof(GX2UniformInitialValue) == 0x14, "GX2UniformInitialValue size mismatch");

typedef struct _GX2SamplerVar
{
    const char* name;
    GX2SamplerType type;
    u32 location;
}
GX2SamplerVar;
static_assert(sizeof(GX2SamplerVar) == 0xC, "GX2SamplerVar size mismatch");

typedef struct _GX2AttribVar
{
    const char* name;
    GX2VarType type;
    u32 arrayCount;
    u32 location;
}
GX2AttribVar;
static_assert(sizeof(GX2AttribVar) == 0x10, "GX2AttribVar size mismatch");

typedef struct _GX2VertexShader
{
    u32 regs[52];
    u32 shaderSize;
    void* shaderPtr;
    GX2ShaderMode shaderMode;
    u32 numUniformBlocks;
    GX2UniformBlock* uniformBlocks;
    u32 numUniforms;
    GX2UniformVar* uniformVars;
    u32 numInitialValues;
    GX2UniformInitialValue* initialValues;
    u32 numLoops;
    void* loopVars;
    u32 numSamplers;
    GX2SamplerVar* samplerVars;
    u32 numAttribs;
    GX2AttribVar* attribVars;
    u32 ringItemsize;
    BOOL hasStreamOut;
    u32 streamOutVertexStride[4];
    GX2RBuffer shaderProgram;
}
GX2VertexShader;
static_assert(sizeof(GX2VertexShader) == 0x134, "GX2VertexShader size mismatch");

typedef struct _GX2PixelShader
{
    u32 regs[41];
    u32 shaderSize;
    void* shaderPtr;
    GX2ShaderMode shaderMode;
    u32 numUniformBlocks;
    GX2UniformBlock* uniformBlocks;
    u32 numUniforms;
    GX2UniformVar* uniformVars;
    u32 numInitialValues;
    GX2UniformInitialValue* initialValues;
    u32 numLoops;
    void* loopVars;
    u32 numSamplers;
    GX2SamplerVar* samplerVars;
    GX2RBuffer shaderProgram;
}
GX2PixelShader;
static_assert(sizeof(GX2PixelShader) == 0xE8, "GX2PixelShader size mismatch");

typedef struct _GX2GeometryShader
{
    u32 regs[19];
    u32 shaderSize;
    void* shaderPtr;
    u32 copyShaderSize;
    void* copyShaderPtr;
    GX2ShaderMode shaderMode;
    u32 numUniformBlocks;
    GX2UniformBlock* uniformBlocks;
    u32 numUniforms;
    GX2UniformVar* uniformVars;
    u32 numInitialValues;
    GX2UniformInitialValue* initialValues;
    u32 numLoops;
    void* loopVars;
    u32 numSamplers;
    GX2SamplerVar* samplerVars;
    u32 ringItemsize;
    BOOL hasStreamOut;
    u32 streamOutVertexStride[4];
    GX2RBuffer shaderProgram;
    GX2RBuffer copyShaderProgram;
}
GX2GeometryShader;
static_assert(sizeof(GX2GeometryShader) == 0xC0, "GX2GeometryShader size mismatch");

#ifdef __cplusplus
extern "C"
{
#endif

void LoadGX2VertexShader(
    const void* data,
    GX2VertexShader* shader,
#ifdef __cplusplus
    bool        allocate    = true,
    bool        isBigEndian = true
#else
    bool        allocate,
    bool        isBigEndian
#endif
);

void LoadGX2PixelShader(
    const void* data,
    GX2PixelShader* shader,
#ifdef __cplusplus
    bool        allocate    = true,
    bool        isBigEndian = true
#else
    bool        allocate,
    bool        isBigEndian
#endif
);

void LoadGX2GeometryShader(
    const void* data,
    GX2GeometryShader* shader,
#ifdef __cplusplus
    bool        allocate    = true,
    bool        isBigEndian = true
#else
    bool        allocate,
    bool        isBigEndian
#endif
);

// TODO: Saving

#ifdef __cplusplus
}
#endif

#endif
