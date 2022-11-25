#pragma once

#include "component.hpp"

namespace Engine {
class ENGINE_API Entity;

class ENGINE_API ComponentScript : public Component {
public:
    struct Delta {
        MSGPACK_DEFINE_ARRAY();
    };

    ComponentScript();
    explicit ComponentScript(Object& object);
    virtual ~ComponentScript();

    Delta getDelta() {
        return {};
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    /*void setScript(wrenbind17::Variable value) {
        script = std::move(value);
    }*/

    /*wrenbind17::Variable& getScript() {
        return script;
    }*/

    /*template <typename... Args> wrenbind17::Any call(const std::string& signature, Args&&... args) {
        auto func = script.func(signature);
        if (!func) {
            throw std::out_of_range("No such function");
        }
        return func(std::forward<Args>(args)...);
    }*/

    /*void clear() {
        script.reset();
    }*/

private:
    // wrenbind17::Variable script;

public:
    MSGPACK_DEFINE_ARRAY();
};
} // namespace Engine
