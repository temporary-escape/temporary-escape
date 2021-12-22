#pragma once

#include "../Assets/AssetTexture.hpp"
#include "../Graphics/Mesh.hpp"
#include "../Library.hpp"
#include "Component.hpp"
#include "PointCloud.hpp"

namespace Scissio {
class SCISSIO_API ComponentPointCloud : public Component, public PointCloud {
public:
    ComponentPointCloud() = default;
    explicit ComponentPointCloud(Object& object, AssetTexturePtr texture);
    virtual ~ComponentPointCloud() = default;

private:
    void rebuildBuffers();

    Mesh mesh{NO_CREATE};
    AssetTexturePtr texture{nullptr};
};
} // namespace Scissio
