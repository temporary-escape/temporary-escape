#pragma once

#include "AssetModel.hpp"
#include "AssetParticles.hpp"

namespace Engine {
class ENGINE_API AssetBlock : public Asset {
public:
    struct Definition {
        std::string title;
        AssetModelPtr model;

        struct Particles {
            AssetParticlesPtr asset;
            Vector3 offset;
        };

        std::optional<Particles> particles;
    };

    static std::shared_ptr<AssetBlock> from(const std::string& name);

    explicit AssetBlock(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetBlock() = default;

    void load(AssetManager& assetManager) override;

    const std::string& getTitle() const {
        return definition.title;
    }

    const AssetModelPtr& getModel() const {
        return definition.model;
    }

    const std::optional<Definition::Particles>& getParticles() const {
        return definition.particles;
    }

private:
    Path path;
    Definition definition;
};

using AssetBlockPtr = std::shared_ptr<AssetBlock>;

namespace Xml {
template <> struct Adaptor<AssetBlock::Definition> {
    static void convert(const Xml::Node& n, AssetBlock::Definition& v);
};

template <> struct Adaptor<AssetBlock::Definition::Particles> {
    static void convert(const Xml::Node& n, AssetBlock::Definition::Particles& v);
};
} // namespace Xml
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
