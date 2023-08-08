#pragma once

#include "../audio/audio_buffer.hpp"
#include "../utils/path.hpp"
#include "../vulkan/vulkan_shader.hpp"
#include "asset.hpp"

namespace Engine {
class ENGINE_API Sound : public Asset {
public:
    explicit Sound(std::string name, Path path);
    MOVEABLE(Sound);

    void load(AssetsManager& assetsManager, VulkanRenderer& vulkan, AudioContext& audio) override;

    static std::shared_ptr<Sound> from(const std::string& name);

    const AudioBuffer& getAudioBuffer() const {
        return buffer;
    }

private:
    Path path;
    AudioBuffer buffer;
};

using SoundPtr = std::shared_ptr<Sound>;

template <> struct Yaml::Adaptor<SoundPtr> {
    static void convert(const Yaml::Node& node, SoundPtr& value) {
        value = Sound::from(node.asString());
    }
    static void pack(Yaml::Node& node, const SoundPtr& value) {
        node.packString(value->getName());
    }
};
} // namespace Engine

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
    namespace adaptor {

    template <> struct convert<Engine::SoundPtr> {
        msgpack::object const& operator()(msgpack::object const& o, Engine::SoundPtr& v) const {
            if (o.type != msgpack::type::STR)
                throw msgpack::type_error();
            std::string name;
            o.convert(name);
            v = Engine::Sound::from(name);
            return o;
        }
    };

    template <> struct pack<Engine::SoundPtr> {
        template <typename Stream>
        msgpack::packer<Stream>& operator()(msgpack::packer<Stream>& o, Engine::SoundPtr const& v) const {
            o.pack_str(static_cast<uint32_t>(v->getName().size()));
            o.pack_str_body(v->getName().c_str(), static_cast<uint32_t>(v->getName().size()));
            return o;
        }
    };
    } // namespace adaptor
}
} // namespace msgpack
