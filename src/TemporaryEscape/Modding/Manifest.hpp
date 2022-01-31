#pragma once
#include "../Utils/Xml.hpp"
#include <string>

namespace Engine {
struct Manifest {
    std::string name;
    std::string description;
    std::string author;
};

namespace Xml {
template <> struct Adaptor<Manifest> {
    static void convert(const Xml::Node& n, Manifest& v) {
        n.child("name").convert(v.name);
        n.child("description").convert(v.description);
        n.child("author").convert(v.author);
    }
};
} // namespace Xml
} // namespace Engine
