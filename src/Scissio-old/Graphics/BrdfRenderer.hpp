#pragma once

#include "../Config.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Graphics/Texture2D.hpp"
#include "../Library.hpp"
#include "../Shaders/ShaderBrdf.hpp"

namespace Scissio {
class SCISSIO_API BrdfRenderer {
public:
    explicit BrdfRenderer(const Config& config);
    virtual ~BrdfRenderer();

    Texture2D render();

private:
    const Config& config;
    ShaderBrdf shader;
    Mesh mesh;
};
} // namespace Scissio