#pragma once

#include "../Audio/AudioBuffer.hpp"
#include "../Utils/Path.hpp"
#include "../Utils/Xml.hpp"
#include "../Vulkan/VulkanShader.hpp"
#include "Asset.hpp"

namespace Engine {
class ENGINE_API Sound : public Asset {
public:
    explicit Sound(std::string name, Path path);
    MOVEABLE(Sound);

    void load(AssetsManager& assetsManager, VulkanRenderer* vulkan, AudioContext* audio) override;

    static std::shared_ptr<Sound> from(const std::string& name);

    const AudioBuffer& getAudioBuffer() const {
        return buffer;
    }

    static void bind(Lua& lua);

private:
    Path path;
    AudioBuffer buffer;
};

using SoundPtr = std::shared_ptr<Sound>;

template <> struct Xml::Adaptor<SoundPtr> {
    static void convert(const Xml::Node& node, SoundPtr& value) {
        if (node && !node.empty()) {
            value = Sound::from(std::string{node.getText()});
        }
    }
    static void pack(Xml::Node& node, const SoundPtr& value) {
        if (value) {
            node.setText(value->getName());
        }
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
