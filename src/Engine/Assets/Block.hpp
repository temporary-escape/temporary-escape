#pragma once

#include "Asset.hpp"
#include "Image.hpp"
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

    enum class Type {
        Regular,
        Engine,
    };

    struct Material {
        TexturePtr baseColorTexture;
        TexturePtr emissiveTexture;
        TexturePtr normalTexture;
        TexturePtr ambientOcclusionTexture;
        TexturePtr metallicRoughnessTexture;
        TexturePtr maskTexture;

        Vector4 baseColorFactor;
        Vector4 emissiveFactor;
        Vector4 normalFactor;
        Vector4 ambientOcclusionFactor;
        Vector4 metallicRoughnessFactor;
    };

    struct MaterialUniform {
        int baseColorTexture;
        int emissiveTexture;
        int normalTexture;
        int ambientOcclusionTexture;
        int metallicRoughnessTexture;
        int maskTexture;

        Vector4 baseColorFactor;
        Vector4 emissiveFactor;
        Vector4 normalFactor;
        Vector4 ambientOcclusionFactor;
        Vector4 metallicRoughnessFactor;

        char padding[8]; // Uniform padding
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

        struct ThrustInfo {
            ParticlesTypePtr particles;

            void convert(const Xml::Node& xml) {
                xml.convert("particles", particles);
            }

            void pack(Xml::Node& xml) const {
                xml.pack("particles", particles);
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
        Type type{Type::Regular};
        ThrustInfo thrust{};
        std::vector<MaterialDefinition> materials;
        RotationMode rotation{RotationMode::Full};

        void convert(const Xml::Node& xml) {
            xml.convert("shape", shapes);
            xml.convert("category", category);
            xml.convert("label", label);
            xml.convert("type", type, false);
            xml.convert("thrust", thrust, false);
            xml.convert("material", materials);
            xml.convert("rotation", rotation);
        }

        void pack(Xml::Node& xml) const {
            xml.pack("shape", shapes);
            xml.pack("category", category);
            xml.pack("label", label);
            xml.pack("type", type);
            xml.pack("thrust", thrust);
            xml.pack("material", materials);
            xml.pack("rotation", rotation);
        }
    };

    explicit Block(std::string name, Path path);
    MOVEABLE(Block);

    void load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) override;
    void allocateMaterials(AssetsManager& assetsManager);

    [[nodiscard]] int getMaterialForSide(const VoxelShape::Face side) const {
        const auto index = shapeSideToMaterial.at(side);
        if (index < 0) {
            EXCEPTION("Block has no associated material for side: {}", int(side));
        }
        return index;
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
        return singular;
    }

    [[nodiscard]] int getMaterial() const {
        return shapeSideToMaterial[0];
    }

    void setThumbnail(const VoxelShape::Type shape, ImagePtr image) {
        thumbnails[shape] = std::move(image);
    }

    [[nodiscard]] const ImagePtr& getThumbnail(const VoxelShape::Type shape) const {
        return thumbnails[shape];
    }

    [[nodiscard]] Type getType() const {
        return definition.type;
    }

    [[nodiscard]] const Definition::ThrustInfo& getThrustInfo() const {
        return definition.thrust;
    }

    static std::shared_ptr<Block> from(const std::string& name);

private:
    Path path;
    Definition definition;
    std::array<int, 7> shapeSideToMaterial{0};
    std::array<ImagePtr, VoxelShape::numOfShapes> thumbnails{nullptr};
    std::vector<Material> materials;
    bool singular{false};
};

XML_DEFINE(Block::Definition, "block");
XML_DEFINE_ENUM(Block::RotationMode, None, Limited, Full);
XML_DEFINE_ENUM(Block::Type, Regular, Engine);

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
