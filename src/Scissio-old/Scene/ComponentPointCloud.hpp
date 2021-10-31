#pragma once

#include "../Assets/BasicTexture.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Library.hpp"
#include "Component.hpp"
#include "PointCloud.hpp"

namespace Scissio {
class ShaderPointCloud;

class SCISSIO_API ComponentPointCloud : public Component, public PointCloud {
public:
    static constexpr ComponentType Type = 4;

    ComponentPointCloud();
    explicit ComponentPointCloud(Object& object, BasicTexturePtr texture);
    virtual ~ComponentPointCloud() = default;

    void render(ShaderPointCloud& shader);

    const Mesh& getMesh() const {
        return mesh;
    }

    const BasicTexturePtr& getTexture() const {
        return texture;
    }

private:
    void rebuildBuffers();

    Mesh mesh;
    BasicTexturePtr texture;
};
} // namespace Scissio
