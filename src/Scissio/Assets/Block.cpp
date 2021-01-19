#include "Block.hpp"
#include "AssetManager.hpp"

using namespace Scissio;

Block::Block(const Manifest& mod, const Path& path) : Asset(mod, path.stem().string()), path(path) {
}

void Block::load(AssetManager& assetManager) {
    Log::d("Loading block: '{}'", getName());

    Xml::Document xml(path);
    auto root = xml.getRoot();

    root.child("title").convert(title);
    root.child("description").convert(description);
    root.child("tier").convert(tier);
    root.child("group").convert(group);
    model = assetManager.find<Model>(root.child("model").asString());

    thumbnail = assetManager.generateThumbnail(*model);
}

MSGPACK_UNPACK_FUNC(BlockPtr) {
    if (o.type == msgpack::type::STR) {
        const auto name = o.as<std::string>();
        v = AssetManager::singleton().find<Block>(name);
    } else {
        throw msgpack::type_error();
    }

    return o;
}

MSGPACK_PACK_FUNC(BlockPtr) {
    if (v) {
        const auto& name = v->getName();
        o.pack_str(static_cast<uint32_t>(name.size()));
        o.pack_str_body(name.c_str(), static_cast<uint32_t>(name.size()));
    } else {
        o.pack_nil();
    }

    return o;
}
