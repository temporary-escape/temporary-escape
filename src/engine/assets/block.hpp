#pragma once

#include "asset.hpp"
#include "image.hpp"
#include "material.hpp"
#include "texture.hpp"
#include "voxel_shape.hpp"

namespace Engine {
class ENGINE_API Block : public Asset {
public:
    struct Definition {
        std::vector<VoxelShape::Type> shapes;
        std::string category;
        std::string label;

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

        YAML_DEFINE(shapes, category, label, materials);
    };

    explicit Block(std::string name, Path path);
    Block(const Block& other) = delete;
    Block(Block&& other) = default;
    Block& operator=(const Block& other) = delete;
    Block& operator=(Block&& other) = default;

    void load(Registry& registry, VulkanRenderer& vulkan) override;

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

    [[nodiscard]] const std::string& getLabel() const {
        return definition.label;
    }

    [[nodiscard]] const std::string& getCategory() const {
        return definition.category;
    }

    [[nodiscard]] bool isSingular() const {
        return materials.size() == 1;
    }

    [[nodiscard]] const Material& getMaterial() const {
        return *materials.front();
    }

    void setThumbnail(ImagePtr image) {
        thumbnail = std::move(image);
    }

    [[nodiscard]] const ImagePtr& getThumbnail() const {
        return thumbnail;
    }

    static std::shared_ptr<Block> from(const std::string& name);

private:
    Path path;
    Definition definition;
    std::array<Material*, 7> shapeSideToMaterial;
    std::vector<std::unique_ptr<Material>> materials;
    ImagePtr thumbnail;
};

using BlockPtr = std::shared_ptr<Block>;

template <> struct Yaml::Adaptor<BlockPtr> {
    static void convert(const Yaml::Node& node, BlockPtr& value) {
        value = Block::from(node.asString());
    }
    static void pack(Yaml::Node& node, const BlockPtr& value) {
        node.packString(value->getName());
    }
};
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
