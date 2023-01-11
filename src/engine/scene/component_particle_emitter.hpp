#pragma once

#include "component.hpp"

namespace Engine {
class ComponentParticleEmitter : public Component {
public:
    ComponentParticleEmitter() = default;
    virtual ~ComponentParticleEmitter() = default; // NOLINT(modernize-use-override)
    COMPONENT_DEFAULTS(ComponentParticleEmitter);

    /*void setOffset(const Vector3& value) {
        uniform.offset = value;
    }

    const Vector3& getOffset() const {
        return uniform.offset;
    }

    void setStartRadius(const float value) {
        uniform.startRadius = value;
    }

    float getStartRadius() const {
        return uniform.startRadius;
    }

    void setEndRadius(const float value) {
        uniform.endRadius = value;
    }

    float getEndRadius() const {
        return uniform.endRadius;
    }

    void setForce(const Vector3& value) {
        uniform.force = value;
    }

    const Vector3& getForce() const {
        return uniform.force;
    }

    void setCount(const int value) {
        uniform.count = value;
    }

    int getCount() const {
        return uniform.count;
    }

    void setDuration(const float value) {
        uniform.duration = value;
    }

    float getDuration() const {
        return uniform.duration;
    }

    void setStartSize(const float value) {
        uniform.startSize = value;
    }

    float getStartSize() const {
        return uniform.startSize;
    }

    void setEndSize(const float value) {
        uniform.endSize = value;
    }

    float getEndSize() const {
        return uniform.endSize;
    }

    void setStartColor(const Color4& value) {
        uniform.startColor = value;
    }

    const Color4& getStartColor() const {
        return uniform.startColor;
    }

    void setEndColor(const Color4& value) {
        uniform.endColor = value;
    }

    const Color4& getEndColor() const {
        return uniform.endColor;
    }*/

    /*const AssetTexturePtr& getTexture() const {
        return asset->getTexture();
    }*/

    /*void rebuild() {
        if (!shouldRebuild) {
            return;
        }

        uniform.startColor = asset->getStartColor();
        uniform.endColor = asset->getEndColor();
        uniform.startRadius = asset->getStartRadius();
        uniform.endRadius = asset->getEndRadius();
        uniform.startColor = asset->getStartColor();
        uniform.endColor = asset->getEndColor();
        uniform.count = asset->getCount();
        uniform.duration = asset->getDuration();
        uniform.force = asset->getForce();
        uniform.startSize = asset->getStartSize();
        uniform.endSize = asset->getEndSize();

        if (!ubo) {
            ubo = VertexBuffer(VertexBufferType::Uniform);
            ubo.bufferData(&uniform, sizeof(Uniform), VertexBufferUsage::DynamicDraw);
        } else {
            ubo.bufferSubData(&uniform, sizeof(Uniform), 0);
        }

        shouldRebuild = false;
    }*/

    /*const VertexBuffer& getUbo() const {
        return ubo;
    }*/

private:
    /*struct Uniform { // glsl layout(std140)
        Color4 startColor{1.0f};
        Color4 endColor{1.0f};
        Vector3 offset{0.0f};
        char padding0{0};
        Vector3 force{0.0f};
        float startRadius{1.0f};
        float endRadius{1.0f};
        int count{100};
        float duration{1.0f};
        float startSize{1.0f};
        float endSize{1.0f};
        char padding3[4] = {0};

        MSGPACK_DEFINE_ARRAY(offset);
    } uniform;

    AssetParticlesPtr asset{nullptr};
    VertexBuffer ubo{NO_CREATE};
    bool shouldRebuild{true};*/
};
} // namespace Engine
