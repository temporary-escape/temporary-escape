#pragma once

#include "AssetTexture.hpp"
#include "Primitive.hpp"
#include "Shape.hpp"

namespace Engine {
class ENGINE_API AssetBlock : public Asset {
public:
    struct Definition {
        std::string title;
        std::vector<Shape::Type> shapes;

        struct Material {
            std::vector<Shape::Side> faces;

            AssetTexturePtr baseColorTexture{nullptr};
            std::optional<Color4> baseColorFactor{1.0f};
            std::optional<AssetTexturePtr> emissiveTexture{nullptr};
            std::optional<Color4> emissiveFactor{1.0f};
            std::optional<AssetTexturePtr> normalTexture{nullptr};
            std::optional<AssetTexturePtr> ambientOcclusionTexture{nullptr};
            std::optional<AssetTexturePtr> metallicRoughnessTexture{nullptr};
            std::optional<Color4> metallicRoughnessFactor{1.0f};

            YAML_DEFINE(faces, baseColorTexture, baseColorFactor, emissiveTexture, emissiveFactor, normalTexture,
                        ambientOcclusionTexture, metallicRoughnessTexture, metallicRoughnessFactor);
        };

        std::vector<Material> materials;

        YAML_DEFINE(title, shapes, materials);
    };

    static std::shared_ptr<AssetBlock> from(const std::string& name);

    explicit AssetBlock(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetBlock() = default;

    void load(AssetManager& assetManager) override;

    [[nodiscard]] const std::string& getTitle() const {
        return definition.title;
    }

    [[nodiscard]] const Material& getMaterialForSide(const Shape::Side side) const {
        const auto* ptr = shapeSideToMaterial.at(side);
        if (!ptr) {
            EXCEPTION("Block has no associated material for side: {}", int(side));
        }
        return *ptr;
    }

    [[nodiscard]] const std::vector<Shape::Type>& getAllowedTypes() const {
        return definition.shapes;
    }

    bool isSingular() const {
        return materials.size() == 1;
    }

    const Material& getMaterial() const {
        return materials.front();
    }

private:
    Path path;
    Definition definition;
    std::array<Material*, 7> shapeSideToMaterial;
    std::vector<Material> materials;
};

using AssetBlockPtr = std::shared_ptr<AssetBlock>;
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::AssetBlockPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::AssetBlockPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::AssetBlock::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::AssetBlockPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::AssetBlockPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
