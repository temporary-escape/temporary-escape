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
            std::optional<TextureDefinition> mask{};

            YAML_DEFINE(faces, baseColor, emissive, normal, ambientOcclusion, metallicRoughness, mask);
        };

        std::vector<MaterialDefinition> materials;

        YAML_DEFINE(shapes, category, label, materials);
    };

    explicit Block(std::string name, Path path);
    MOVEABLE(Block);

    void load(AssetsManager& assetsManager, VulkanRenderer& vulkan, AudioContext& audio) override;

    [[nodiscard]] const Material& getMaterialForSide(const VoxelShape::Face side) const {
        const auto* ptr = shapeSideToMaterial.at(side);
        if (!ptr) {
            EXCEPTION("Block has no associated material for side: {}", int(side));
        }
        return *ptr;
    }

    [[nodiscard]] const std::vector<VoxelShape::Type>& getShapes() const {
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

    void setThumbnail(const VoxelShape::Type shape, ImagePtr image) {
        thumbnails[shape] = std::move(image);
    }

    [[nodiscard]] const ImagePtr& getThumbnail(const VoxelShape::Type shape) const {
        return thumbnails[shape];
    }

    static std::shared_ptr<Block> from(const std::string& name);

    static void bind(Lua& lua);

private:
    Path path;
    Definition definition;
    std::array<Material*, 7> shapeSideToMaterial;
    std::vector<std::unique_ptr<Material>> materials;
    std::array<ImagePtr, VoxelShape::numOfShapes> thumbnails;
};

using BlockPtr = std::shared_ptr<Block>;

template <> struct Yaml::Adaptor<BlockPtr> {
    static void convert(const Yaml::Node& node, BlockPtr& value) {
        if (node.isNull()) {
            value = nullptr;
        } else {
            value = Block::from(node.asString());
        }
    }
    static void pack(Yaml::Node& node, const BlockPtr& value) {
        if (value) {
            node.packString(value->getName());
        } else {
            node.packNull();
        }
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
