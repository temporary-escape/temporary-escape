#pragma once

#include "../Utils/Xml.hpp"

namespace Engine {
struct PlayerLocalProfile {
    std::string name;
    uint64_t secret;
};

namespace Xml {
template <> struct Adaptor<PlayerLocalProfile> {
    static inline void convert(const Xml::Node& n, PlayerLocalProfile& v) {
        n.child("name").convert(v.name);
        n.child("secret").convert(v.secret);
    }
};

template <> struct Writer<PlayerLocalProfile> {
    static inline void read(Xml::NewNode& n, const PlayerLocalProfile& v) {
        n.child("name").write(v.name);
        n.child("secret").write(v.secret);
    }
};
}
}
