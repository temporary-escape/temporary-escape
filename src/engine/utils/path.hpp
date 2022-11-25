#pragma once

#include "../library.hpp"
#include "format.hpp"
#include <filesystem>
#include <set>
#include <string>

namespace Engine {
using Path = std::filesystem::path;
namespace Fs = std::filesystem;

template <typename Fn>
inline ENGINE_API void iterateDir(const Path& dir, const std::set<std::string>& exts, const Fn& fn) {
    if (Fs::exists(dir) && Fs::is_directory(dir)) {
        for (const auto& it : Fs::recursive_directory_iterator(dir)) {
            const auto ext = it.path().extension().string();
            if (it.is_regular_file() && exts.find(ext) != exts.end()) {
                const auto path = Fs::absolute(it.path());
                fn(path);
            }
        }
    }
}

ENGINE_API Path getAppDataPath();
ENGINE_API Path getExecutablePath();
} // namespace Engine

template <> struct fmt::formatter<Engine::Path> {
    template <typename ParseContext> constexpr auto parse(ParseContext& ctx) {
        return ctx.begin();
    }

    template <typename FormatContext> auto format(Engine::Path const& path, FormatContext& ctx) {
        return fmt::format_to(ctx.out(), "{}", path.string());
    }
};
