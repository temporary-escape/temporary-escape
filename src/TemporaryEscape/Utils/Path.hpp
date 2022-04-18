#pragma once

#include "Format.hpp"
#include <filesystem>

namespace Engine {
using Path = std::filesystem::path;
namespace Fs = std::filesystem;
} // namespace Engine

template <> struct fmt::formatter<Engine::Path> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Path const& path, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", path.string());
    }
};
