#pragma once

#include "../scene/scene.hpp"
#include "render_pipeline.hpp"
#include "render_resources.hpp"

namespace Engine {
class ENGINE_API RenderPass;

class ENGINE_API RenderSubpass : public NonCopyable {
public:
    RenderSubpass() = default;
    virtual ~RenderSubpass() = default;
    NON_MOVEABLE(RenderSubpass);

    void init(VulkanRenderPass& renderPass, uint32_t subpass);
    void init();

    const std::vector<uint32_t>& getAttachments() const {
        return attachments;
    }

    const std::vector<uint32_t>& getInputs() const {
        return inputs;
    }

protected:
    void setAttachments(std::vector<uint32_t> value) {
        attachments = std::move(value);
    }
    void setInputs(std::vector<uint32_t> value) {
        inputs = std::move(value);
    }
    void addPipeline(RenderPipeline& pipeline) {
        pipelines.push_back(&pipeline);
    }

private:
    std::vector<uint32_t> attachments;
    std::vector<uint32_t> inputs;
    std::list<RenderPipeline*> pipelines;
};
} // namespace Engine
