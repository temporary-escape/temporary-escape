#pragma once

#include "AssetImage.hpp"
#include "AssetTexture.hpp"
#include "Primitive.hpp"
#include "Shape.hpp"

namespace Engine {
class ENGINE_API AssetBlock : public Asset {
public:
    struct Definition {
        std::string title;
        int level;
        std::string category;
        std::vector<Shape::Type> shapes;

        struct TextureDefinition {
            AssetTexturePtr texture;
            std::optional<Color4> factor{1.0f};

            YAML_DEFINE(texture, factor);
        };

        struct Material {
            std::vector<Shape::Side> faces;

            TextureDefinition baseColor{nullptr};
            std::optional<TextureDefinition> emissive{};
            std::optional<TextureDefinition> normal{};
            std::optional<TextureDefinition> ambientOcclusion{};
            std::optional<TextureDefinition> metallicRoughness{};

            YAML_DEFINE(faces, baseColor, emissive, normal, ambientOcclusion, metallicRoughness);
        };

        std::vector<Material> materials;

        YAML_DEFINE(title, level, category, shapes, materials);
    };

    static std::shared_ptr<AssetBlock> from(const std::string& name);

    explicit AssetBlock(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetBlock() = default;

    void load(AssetManager& assetManager, bool noGraphics) override;

    [[nodiscard]] const std::string& getTitle() const {
        return definition.title;
    }

    [[nodiscard]] const std::string& getCategory() const {
        return definition.category;
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

    const AssetImagePtr& getImageForShape(const Shape::Type type) const {
        return images.at(type);
    }

    void setImageForShape(const Shape::Type type, const AssetImagePtr& image) {
        images[type] = image;
    }

private:
    Path path;
    Definition definition;
    std::array<Material*, 7> shapeSideToMaterial;
    std::vector<Material> materials;
    std::unordered_map<Shape::Type, AssetImagePtr> images;
};

using AssetBlockPtr = std::shared_ptr<AssetBlock>;

struct AssetBlockShape {
    AssetBlockPtr block;
    Shape::Type shape;
};
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
