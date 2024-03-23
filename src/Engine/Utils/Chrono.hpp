#pragma once

#include "Xml.hpp"
#include <chrono>

namespace Engine {
ENGINE_API std::string timePointToIso(const std::chrono::system_clock::time_point& tp);
ENGINE_API std::chrono::system_clock::time_point isoToTimePoint(const std::string_view& str);

ENGINE_API std::string timePointToLocalString(const std::chrono::system_clock::time_point& tp);

template <> struct Xml::Adaptor<std::chrono::system_clock::time_point> {
    static void pack(Node& node, const std::chrono::system_clock::time_point& value) {
        node.setText(timePointToIso(value));
    }
    static void convert(const Node& node, std::chrono::system_clock::time_point& value) {
        value = isoToTimePoint(node.getText());
    }
};
} // namespace Engine
