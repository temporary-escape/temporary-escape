#pragma once

#include "Asset.hpp"
#include "Image.hpp"
#include "Material.hpp"
#include "ParticlesType.hpp"
#include "Texture.hpp"
#include "VoxelShape.hpp"

namespace Engine {
class ENGINE_API Block : public Asset {
public:
    enum class RotationMode {
        None,
        Limited,
        Full,
    };

    struct Definition {
        struct TextureDefinition {
            TexturePtr texture;
            std::optional<Color4> factor{1.0f};

            void convert(const Xml::Node& xml) {
                xml.convert("texture", texture);
                xml.convert("factor", factor);
            }

            void pack(Xml::Node& xml) const {
                xml.pack("texture", texture);
                xml.pack("factor", factor);
            }
        };

        struct ParticlesInfo {
            Vector3 offset;
            ParticlesTypePtr type;

            void convert(const Xml::Node& xml) {
                xml.convert("offset", offset);
                xml.convert("type", type);
            }

            void pack(Xml::Node& xml) const {
                xml.pack("offset", offset);
                xml.pack("type", type);
            }
        };

        struct MaterialDefinition {
            std::vector<VoxelShape::Face> faces;

            std::optional<TextureDefinition> baseColor{};
            std::optional<TextureDefinition> emissive{};
            std::optional<TextureDefinition> normal{};
            std::optional<TextureDefinition> ambientOcclusion{};
            std::optional<TextureDefinition> metallicRoughness{};
            std::optional<TextureDefinition> mask{};

            void convert(const Xml::Node& xml) {
                xml.convert("face", faces);
                xml.convert("baseColor", baseColor);
                xml.convert("emissive", emissive);
                xml.convert("normal", normal);
                xml.convert("ambientOcclusion", ambientOcclusion);
                xml.convert("metallicRoughness", metallicRoughness);
                xml.convert("mask", mask);
            }

            void pack(Xml::Node& xml) const {
                xml.pack("face", faces);
                xml.pack("baseColor", baseColor);
                xml.pack("emissive", emissive);
                xml.pack("normal", normal);
                xml.pack("ambientOcclusion", ambientOcclusion);
                xml.pack("metallicRoughness", metallicRoughness);
                xml.pack("mask", mask);
            }
        };

        std::vector<VoxelShape::Type> shapes;
        std::string category;
        std::string label;
        std::optional<ParticlesInfo> particles;
        std::vector<MaterialDefinition> materials;
        RotationMode rotation{RotationMode::Full};

        void convert(const Xml::Node& xml) {
            xml.convert("shape", shapes);
            xml.convert("category", category);
            xml.convert("label", label);
            xml.convert("particles", particles);
            xml.convert("material", materials);
            xml.convert("rotation", rotation);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("shape", shapes);
            xml.pack("category", category);
            xml.pack("label", label);
            xml.pack("particles", particles);
            xml.pack("material", materials);
            xml.pack("rotation", rotation);
        }
    };

    explicit Block(std::string name, Path path);
    MOVEABLE(Block);

    void load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) override;

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

    [[nodiscard]] RotationMode getRotationMode() const {
        return definition.rotation;
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

    const std::optional<Definition::ParticlesInfo>& getParticlesInfo() const {
        return definition.particles;
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

XML_DEFINE(Block::Definition, "block");
XML_DEFINE_ENUM(Block::RotationMode, None, Limited, Full);

using BlockPtr = std::shared_ptr<Block>;

template <> struct Xml::Adaptor<BlockPtr> {
    static void convert(const Xml::Node& node, BlockPtr& value) {
        if (node && !node.empty()) {
            value = Block::from(std::string{node.getText()});
        }
    }
    static void pack(Xml::Node& node, const BlockPtr& value) {
        if (value) {
            node.setText(value->getName());
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
