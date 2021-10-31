#pragma once

#include "ShaderPbr.hpp"

namespace Scissio {
class SCISSIO_API ShaderModel : public ShaderPbr {
public:
    typedef VertexAttribute<0, Vector3> Position;
    typedef VertexAttribute<1, Vector3> Normal;
    typedef VertexAttribute<2, Vector2> TextureCoordinates;
    typedef VertexAttribute<3, Vector4> Tangent;

    explicit ShaderModel(const Config& config);
    virtual ~ShaderModel() = default;
};
} // namespace Scissio
