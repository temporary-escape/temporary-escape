#include "ComponentModelSkinned.hpp"

using namespace Engine;

ComponentModelSkinned::ComponentModelSkinned(EntityId entity, const ModelPtr& model) : Component{entity} {
    setModel(model);
}

void ComponentModelSkinned::setModel(ModelPtr value) {
    model = std::move(value);

    cache.count = 0;

    if (!model || model->getNodes().empty()) {
        return;
    }

    const auto& node = model->getNodes().front();
    if (!node.skin) {
        return;
    }

    cache.inverseBindMat = node.skin.inverseBindMat;
    cache.jointsLocalMat = node.skin.jointsLocalMat;
    cache.count = node.skin.count;
    for (size_t i = 0; i < cache.count; i++) {
        cache.adjustmentsMat[i] = Matrix4{1.0f};
    }
}

void ComponentModelSkinned::setAdjustment(size_t joint, const Matrix4& value) {
    if (joint >= cache.adjustmentsMat.size()) {
        EXCEPTION("Adjustment joint index out of range");
    }
    cache.adjustmentsMat.at(joint) = value;
}

void ComponentModelSkinned::setUboOffset(const size_t value) {
    uboOffset = value;
}
