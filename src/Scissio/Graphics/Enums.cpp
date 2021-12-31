#include "Enums.hpp"
#include <fmt/format.h>

using namespace Scissio;

void Xml::Adaptor<TextureFiltering>::convert(const Xml::Node& n, TextureFiltering& v) {
    const auto s = n.asString();
    if (s == "Linear") {
        v = TextureFiltering::Linear;
    } else if (s == "Nearest") {
        v = TextureFiltering::Nearest;
    } else if (s == "LinearMipMapLinear") {
        v = TextureFiltering::LinearMipMapLinear;
    } else if (s == "LinearMipMapNearest") {
        v = TextureFiltering::LinearMipMapNearest;
    } else if (s == "NearestMipMapLinear") {
        v = TextureFiltering::NearestMipMapLinear;
    } else if (s == "NearestMipMapNearest") {
        v = TextureFiltering::NearestMipMapNearest;
    } else {
        throw std::runtime_error(fmt::format("Xml value of {} is not a valid TextureFiltering type", s));
    }
}

void Xml::Adaptor<TextureWrapping>::convert(const Xml::Node& n, TextureWrapping& v) {
    const auto s = n.asString();
    if (s == "Repeat") {
        v = TextureWrapping::Repeat;
    } else if (s == "ClampToEdge") {
        v = TextureWrapping::ClampToEdge;
    } else {
        throw std::runtime_error(fmt::format("Xml value of {} is not a valid TextureWrapping type", s));
    }
}
