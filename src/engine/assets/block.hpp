#pragma once

#include "asset.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "voxel_shape.hpp"

namespace Engine {
class ENGINE_API Block : public Asset {
public:
    struct Definition {
        std::string title;
        int level;
        std::string category;
        std::vector<VoxelShape::Type> shapes;

        struct TextureDefinition {
            TexturePtr texture;
            std::optional<Color4> factor{1.0f};

            YAML_DEFINE(texture, factor);
        };

        struct MaterialDefinition {
            std::vector<VoxelShape::Face> faces;

            std::optional<TextureDefinition> baseColor{};
            std::optional<TextureDefinition> emissive{};
            std::optional<TextureDefinition> normal{};
            std::optional<TextureDefinition> ambientOcclusion{};
            std::optional<TextureDefinition> metallicRoughness{};

            YAML_DEFINE(faces, baseColor, emissive, normal, ambientOcclusion, metallicRoughness);
        };

        std::vector<MaterialDefinition> materials;

        YAML_DEFINE(title, level, category, shapes, materials);
    };

    explicit Block(std::string name, Path path);
    void load(Registry& registry, VulkanDevice& vulkan) override;

    [[nodiscard]] const std::string& getTitle() const {
        return definition.title;
    }

    [[nodiscard]] const std::string& getCategory() const {
        return definition.category;
    }

    [[nodiscard]] const Material& getMaterialForSide(const VoxelShape::Face side) const {
        const auto* ptr = shapeSideToMaterial.at(side);
        if (!ptr) {
            EXCEPTION("Block has no associated material for side: {}", int(side));
        }
        return *ptr;
    }

    [[nodiscard]] const std::vector<VoxelShape::Type>& getAllowedTypes() const {
        return definition.shapes;
    }

    [[nodiscard]] bool isSingular() const {
        return materials.size() == 1;
    }

    [[nodiscard]] const Material& getMaterial() const {
        return materials.front();
    }

    static std::shared_ptr<Block> from(const std::string& name);

private:
    Path path;
    Definition definition;
    std::array<Material*, 7> shapeSideToMaterial;
    std::vector<Material> materials;
};

using BlockPtr = std::shared_ptr<Block>;
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::BlockPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::BlockPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::Block::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::BlockPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::BlockPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
