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

ClientWindow::ClientWindow(const Config& config)
    : VulkanWindow(config.windowName, {config.windowWidth, config.windowHeight}), Game{config, *this} {
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

ClientWindow::~ClientWindow() {
    vbo.reset();
    ibo.reset();
    ubo.reset();
    shader.reset();
}

void ClientWindow::update(float deltaTime) {
    this->lastDeltaTime = deltaTime;
    const auto windowSize = getWindowSize();

    // Calculate elapsed time.
    static auto startTime = std::chrono::high_resolution_clock::now();
    auto currentTime = std::chrono::high_resolution_clock::now();
    float elapsedTime =
        std::chrono::duration_cast<std::chrono::microseconds>(currentTime - startTime).count() / 1000.0f;

    // Calculate appropriate matrices for the current frame.
    CameraUniformBuffer ub = {};
    ub.model = glm::rotate(glm::mat4(1.0f), elapsedTime * glm::radians(0.01f), glm::vec3(0.0f, 0.0f, 1.0f));
    ub.view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    ub.projection = glm::perspective(glm::radians(45.0f), windowSize.x / static_cast<float>(windowSize.y), 0.1f, 10.0f);
    ub.projection[1][1] *= -1.0f;

    auto* data = ubo.map<CameraUniformBuffer>();
    *data = ub;
    ubo.unmap();
}

void ClientWindow::render(const Vector2i& viewport) {
    startCommandBuffer();

    const auto windowSize = getWindowSize();

    // Set the viewport state and dimensions.
    setViewport({0, 0}, windowSize);
    setScissor({0, 0}, windowSize);
    setViewportState();

    VulkanFramebuffer::AttachmentReference colorAttachment{};
    VulkanFramebuffer::AttachmentReference depthStencilAttachment{};
    colorAttachment.clearValue.color = {0.3f, 0.3f, 0.3f, 0.0f};
    depthStencilAttachment.clearValue.depthStencil.depth = 1.0f;
    depthStencilAttachment.clearValue.depthStencil.stencil = 0;

    beginRenderPass(getDefaultFramebuffer(), {colorAttachment, depthStencilAttachment});

    /*// Bind the pipeline and associated resources.
    bindPipeline(shader);
    bindUniformBuffer(ubo);
    // vezCmdBindImageView(m_imageView, m_sampler, 0, 1, 0);

    // Set depth stencil state.
    setDepthStencilState(true, true);

    // Bind the vertex buffer and index buffers.
    bindVertexBuffer(vbo);
    bindIndexBuffer(ibo);

    // Draw the quad.
    drawIndexed(6, 1, 0, 0, 0);*/

    Game::render(windowSize, lastDeltaTime);

    // End the render pass.
    endRenderPass();

    endCommandBuffer();
    submitQueue();
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

void ClientWindow::eventWindowResized(const Vector2i& size) {
}
