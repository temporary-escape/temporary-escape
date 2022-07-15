#include "ClientWindow.hpp"

using namespace Engine;

struct Vertex {
    float x, y, z;
    float nx, ny, nz;
    float u, v;
};

static const std::string fragmentShader = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec2 texCoord;
} ps_in;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = vec4(ps_in.texCoord, 1.0, 1.0);
}
)";

static const std::string vertexShader = R"(#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject
{
    mat4 model;
    mat4 view;
    mat4 proj;
};

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec2 in_TexCoord;

layout(location = 0) out VS_OUT
{
    vec3 pos;
    vec3 normal;
    vec2 texCoord;
} vs_out;

out gl_PerVertex{
    vec4 gl_Position;
};

void main()
{
    vs_out.pos = in_Position;
    vs_out.normal = in_Normal;
    vs_out.texCoord = in_TexCoord;
    gl_Position = proj * view * model * vec4(in_Position, 1.0f);
}
)";

ClientWindow::ClientWindow(const std::string& name, const Vector2i& size) : VulkanWindow(name, size) {
}

ClientWindow::~ClientWindow() {
}

void ClientWindow::initialize() {
    // A single quad with positions, normals and uvs.
    Vertex vertices[] = {{-1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f},
                         {1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f},
                         {1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f},
                         {-1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f}};

    vbo = createBuffer(VulkanBuffer::Type::Vertex, VulkanBuffer::Usage::Static, sizeof(vertices));
    vbo.subData(vertices, 0, sizeof(vertices));

    // A single quad with positions, normals and uvs.
    uint32_t indices[] = {
        0, 1, 2, 0, 2, 3,
    };

    ibo = createBuffer(VulkanBuffer::Type::Index, VulkanBuffer::Usage::Static, sizeof(indices));
    ibo.subData(indices, 0, sizeof(indices));

    ubo = createBuffer(VulkanBuffer::Type::Uniform, VulkanBuffer::Usage::Dynamic, sizeof(CameraUniformBuffer));

    shader = createPipeline({
        {vertexShader, ShaderType::Vertex},
        {fragmentShader, ShaderType::Fragment},
    });
}

void ClientWindow::cleanup() {
    vbo.reset();
    ibo.reset();
    ubo.reset();
    shader.reset();
}

void ClientWindow::update(float timeElapsed) {
    // Get the current window dimensions.
    int width, height;
    AppBase::GetWindowSize(&width, &height);

    // Calculate elapsed time.
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float elapsedTime =
        std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime).count() / 1000.0f;

    // Calculate appropriate matrices for the current frame.
    CameraUniformBuffer ub = {};
    ub.model = glm::rotate(glm::mat4(1.0f), elapsedTime * glm::radians(0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
    ub.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ub.projection = glm::perspective(glm::radians(45.0f), width / static_cast<float>(height), 0.1f, 10.0f);
    ub.projection[1][1] *= -1.0f;

    auto* data = ubo.map<CameraUniformBuffer>();
    *data = ub;
    ubo.unmap();

    /*// Update the memory via map and unmap since it was created as HOST_VISIBLE (i.e. VK_MEMORY_USAGE_CPU_TO_GPU).
    void* data = nullptr;
    auto result = vezMapBuffer(getDevice(), ubo.getHandle(), 0, sizeof(CameraUniformBuffer), &data);
    if (result != VK_SUCCESS) {
        EXCEPTION("vezMapBuffer failed");
    }

    memcpy(data, &ub, sizeof(CameraUniformBuffer));
    vezUnmapBuffer(getDevice(), ubo.getHandle());*/
}

void ClientWindow::render(const Vector2i& viewport) {
    startCommandBuffer();

    // Set the viewport state and dimensions.
    int width, height;
    AppBase::GetWindowSize(&width, &height);
    VkViewport view = {0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height), 0.0f, 1.0f};
    VkRect2D scissor = {{0, 0}, {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}};
    vezCmdSetViewport(0, 1, &view);
    vezCmdSetScissor(0, 1, &scissor);
    vezCmdSetViewportState(1);

    // Define clear values for the swapchain's color and depth attachments.
    std::array<VezAttachmentReference, 2> attachmentReferences = {};
    attachmentReferences[0].clearValue.color = {0.3f, 0.3f, 0.3f, 0.0f};
    attachmentReferences[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentReferences[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachmentReferences[1].clearValue.depthStencil.depth = 1.0f;
    attachmentReferences[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachmentReferences[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;

    // Begin a render pass.
    VezRenderPassBeginInfo beginInfo = {};
    beginInfo.framebuffer = AppBase::GetFramebuffer();
    beginInfo.attachmentCount = static_cast<uint32_t>(attachmentReferences.size());
    beginInfo.pAttachments = attachmentReferences.data();
    vezCmdBeginRenderPass(&beginInfo);

    // Bind the pipeline and associated resources.
    bindPipeline(shader);
    bindUniformBuffer(ubo);
    // vezCmdBindImageView(m_imageView, m_sampler, 0, 1, 0);

    // Set depth stencil state.
    VezPipelineDepthStencilState depthStencilState = {};
    depthStencilState.depthTestEnable = VK_TRUE;
    depthStencilState.depthWriteEnable = VK_TRUE;
    depthStencilState.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
    vezCmdSetDepthStencilState(&depthStencilState);

    // Bind the vertex buffer and index buffers.
    bindVertexBuffer(vbo);
    bindIndexBuffer(ibo);

    // Draw the quad.
    vezCmdDrawIndexed(6, 1, 0, 0, 0);

    // End the render pass.
    vezCmdEndRenderPass();

    endCommandBuffer();
    submitQueue();
}

void ClientWindow::eventResized(const Vector2i& size) {
}

void ClientWindow::eventMouseMoved(const Vector2i& pos) {
}

void ClientWindow::eventMousePressed(const Vector2i& pos, MouseButton button) {
}

void ClientWindow::eventMouseReleased(const Vector2i& pos, MouseButton button) {
}

void ClientWindow::eventMouseScroll(int xscroll, int yscroll) {
}

void ClientWindow::eventKeyPressed(Key key, Modifiers modifiers) {
}

void ClientWindow::eventKeyReleased(Key key, Modifiers modifiers) {
}
