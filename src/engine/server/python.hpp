#pragma once

#include "../utils/path.hpp"
#include <functional>
#include <list>
#include <vector>

namespace pybind11 {
class gil_scoped_release;
class scoped_interpreter;
} // namespace pybind11

namespace Engine {
class Python {
public:
    explicit Python(const Path& home, const std::vector<Path>& paths);
    ~Python();

    void importModule(const std::string& name);
    // void guard(const std::function<void()>& fn);

private:
    struct Module;
    std::list<std::unique_ptr<Module>> modules;
    std::unique_ptr<pybind11::scoped_interpreter> interpreter;
};
} // namespace Engine
