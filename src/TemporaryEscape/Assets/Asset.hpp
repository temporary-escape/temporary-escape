#pragma once

#include "../Config.hpp"
#include "../Modding/Manifest.hpp"
#include <memory>

namespace Engine {
class AssetManager;

class ENGINE_API Asset {
public:
    explicit Asset(const Manifest& mod, std::string name) : mod(mod), name(std::move(name)) {
    }
    virtual ~Asset() = default;

    virtual void load(AssetManager& assetManager) = 0;

    const Manifest& getMod() const {
        return mod;
    }

    const std::string& getName() const {
        return name;
    }

private:
    const Manifest& mod;
    std::string name;
};

using AssetPtr = std::shared_ptr<Asset>;
} // namespace Engine

#define MSGPACK_DEFINE_ASSET(Type)                                                                                     \
    namespace msgpack {                                                                                                \
    MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {                                                            \
        namespace adaptor {                                                                                            \
        template <> struct convert<std::shared_ptr<Engine::Type>> {                                                    \
            msgpack::object const& operator()(msgpack::object const& o, std::shared_ptr<Engine::Type>& v) const {      \
                if (o.type != msgpack::type::STR)                                                                      \
                    throw msgpack::type_error();                                                                       \
                std::string name;                                                                                      \
                o.convert(name);                                                                                       \
                v = Engine::Type::from(name);                                                                          \
                return o;                                                                                              \
            }                                                                                                          \
        };                                                                                                             \
        template <> struct pack<std::shared_ptr<Engine::Type>> {                                                       \
            template <typename Stream>                                                                                 \
            msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o,                                            \
                                                std::shared_ptr<Engine::Type> const& v) const {                        \
                o.pack_str(static_cast<uint32_t>(v->getName().size()));                                                \
                o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));                     \
                return o;                                                                                              \
            }                                                                                                          \
        };                                                                                                             \
        }                                                                                                              \
    }                                                                                                                  \
    }
