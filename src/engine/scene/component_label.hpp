#pragma once

#include "component.hpp"

namespace Engine {
class ENGINE_API ComponentLabel : public Component {
public:
    struct Delta {
        std::string label;

        MSGPACK_DEFINE_ARRAY(label);
    };

    ComponentLabel() = default;
    explicit ComponentLabel(Object& object, std::string label) : Component(object), label{std::move(label)} {
    }

    virtual ~ComponentLabel() = default;

    Delta getDelta() {
        Delta delta{};
        delta.label = label;
        // delta.image = image;
        return delta;
    }

    void applyDelta(Delta& delta) {
        (void)delta;
    }

    const std::string& getLabel() const {
        return label;
    }

    void setLabel(std::string value) {
        label = std::move(value);
        setDirty(true);
    }

    /*const AssetImagePtr& getImage() const {
        return image;
    }

    void setImage(AssetImagePtr value) {
        image = std::move(value);
        setDirty(true);
    }*/

private:
    std::string label;
    // AssetImagePtr image;
    Color4 color;

public:
    MSGPACK_DEFINE_ARRAY(label);
};
} // namespace Engine
