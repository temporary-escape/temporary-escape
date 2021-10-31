#pragma once

#include "ShaderPbr.hpp"

namespace Scissio {
class SCISSIO_API ShaderGrid : public ShaderPbr {
public:
    typedef VertexAttribute<0, Vector3> Position;
    typedef VertexAttribute<1, Vector3> Normal;
    typedef VertexAttribute<2, Vector2> TextureCoordinates;
    typedef VertexAttribute<3, Vector4> Tangent;
    typedef VertexAttribute<4, Matrix4> Instances;

    explicit ShaderGrid(const Config& config);
    virtual ~ShaderGrid() = default;
};
} // namespace Scissio
