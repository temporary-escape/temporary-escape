#pragma once

#include "AssetModel.hpp"
#include "AssetParticles.hpp"

namespace wrenbind17 {
class VM;
class Variable;
} // namespace wrenbind17

namespace Engine {
class ENGINE_API AssetEntity : public Asset {
public:
    static std::shared_ptr<AssetEntity> from(const std::string& name);

    explicit AssetEntity(const Manifest& mod, std::string name, const Path& path);
    virtual ~AssetEntity() = default;

    void load(AssetManager& assetManager) override;
    void run(wrenbind17::VM& vm);
    const std::string& getModuleName() const {
        return moduleName;
    }
    const std::string& getClassName() const {
        return className;
    }

private:
    Path path;
    std::string moduleName;
    std::string className;
};

using AssetEntityPtr = std::shared_ptr<AssetEntity>;
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::AssetEntityPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::AssetEntityPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::AssetEntity::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::AssetEntityPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::AssetEntityPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
