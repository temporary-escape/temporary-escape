#pragma once

#include <cstdint>
#include <glad/glad.h>

namespace Scissio {
enum class TextureType {
    Generic,
    BaseColor,
    Normals,
    MetallicRoughness,
    AmbientOcclusion,
    Emissive,
};

enum class VertexBufferType : GLenum {
    Array = GL_ARRAY_BUFFER,
    Indices = GL_ELEMENT_ARRAY_BUFFER,
};

enum class VertexBufferUsage : GLenum {
    StaticDraw = GL_STATIC_DRAW,
    DynamicDraw = GL_DYNAMIC_DRAW,
};

/*enum class VertexAttribute : GLint {
    Float = 1,
    Vector2 = 2,
    Vector3 = 3,
    Vector4 = 4,
};*/

enum class PrimitiveType : GLenum {
    Points = GL_POINTS,
    Triangles = GL_TRIANGLES,
    TriangleFan = GL_TRIANGLE_FAN,
    TriangleStrip = GL_TRIANGLE_STRIP,
    Lines = GL_LINES,
    LineLoop = GL_LINE_LOOP,
    LineStrip = GL_LINE_STRIP,
};

enum class IndexType : GLenum {
    None = GL_NONE,
    UnsignedByte = GL_UNSIGNED_BYTE,
    UnsignedShort = GL_UNSIGNED_SHORT,
    UnsignedInt = GL_UNSIGNED_INT,
};

enum class FramebufferAttachment : GLenum {
    Color0 = GL_COLOR_ATTACHMENT0,
    Color1 = GL_COLOR_ATTACHMENT1,
    Color2 = GL_COLOR_ATTACHMENT2,
    Color3 = GL_COLOR_ATTACHMENT3,
    Color4 = GL_COLOR_ATTACHMENT4,
    Color5 = GL_COLOR_ATTACHMENT5,
    Color6 = GL_COLOR_ATTACHMENT6,
    Color7 = GL_COLOR_ATTACHMENT7,
    Color8 = GL_COLOR_ATTACHMENT8,
    Color9 = GL_COLOR_ATTACHMENT9,
    Color10 = GL_COLOR_ATTACHMENT10,
    Color11 = GL_COLOR_ATTACHMENT11,
    Color12 = GL_COLOR_ATTACHMENT12,
    Color13 = GL_COLOR_ATTACHMENT13,
    Color14 = GL_COLOR_ATTACHMENT14,
    Color15 = GL_COLOR_ATTACHMENT15,
    Depth = GL_DEPTH_ATTACHMENT,
    DepthStencil = GL_DEPTH_STENCIL_ATTACHMENT,
    Stencil = GL_STENCIL_ATTACHMENT,
};
} // namespace Scissio
