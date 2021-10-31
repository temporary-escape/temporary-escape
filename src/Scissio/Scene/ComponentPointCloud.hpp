#pragma once

#include "../Assets/AssetTexture.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Library.hpp"
#include "Component.hpp"
#include "PointCloud.hpp"

namespace Scissio {
class SCISSIO_API ComponentPointCloud : public Component, public PointCloud {
public:
    static constexpr ComponentType Type = 4;

    ComponentPointCloud();
    explicit ComponentPointCloud(Object& object, AssetTexturePtr texture);
    virtual ~ComponentPointCloud() = default;

    void render(Shader& shader);

private:
    void rebuildBuffers();

    Mesh mesh;
    AssetTexturePtr texture;
};
} // namespace Scissio
