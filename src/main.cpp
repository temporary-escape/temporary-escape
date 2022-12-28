#include <CLI/CLI.hpp>
#include <engine/client/application.hpp>
#include <engine/utils/exceptions.hpp>
#include <engine/utils/log.hpp>
#include <engine/vg/vg_renderer.hpp>

static const std::string vertCode = R"(#version 450

vec2 positions[3] = vec2[](
    vec2(0.0, -0.5),
    vec2(0.5, 0.5),
    vec2(-0.5, 0.5)
);

vec3 colors[3] = vec3[](
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out VS_OUT {
    vec3 color;
} vs_out;

void main() {
    vs_out.color = colors[gl_VertexIndex];
    gl_Position = vec4(positions[gl_VertexIndex], 0.0, 1.0);
}
)";

static const std::string fragCode = R"(#version 450

layout(location = 0) in VS_OUT {
    vec3 color;
} vs_out;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(vs_out.color, 1.0);
}
)";

using namespace Engine;

class VgApplication : public VgRenderer {
public:
    VgApplication(const Config& config) : VgRenderer{config} {
        auto vert = createShaderModule(vertCode, VK_SHADER_STAGE_VERTEX_BIT);
        auto frag = createShaderModule(fragCode, VK_SHADER_STAGE_FRAGMENT_BIT);

        VgPipeline::CreateInfo pipelineInfo{};
        pipelineInfo.shaderModules = {&vert, &frag};

        pipelineInfo.vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        pipelineInfo.vertexInputInfo.vertexBindingDescriptionCount = 0;
        pipelineInfo.vertexInputInfo.vertexAttributeDescriptionCount = 0;

        pipelineInfo.inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        pipelineInfo.inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        pipelineInfo.inputAssembly.primitiveRestartEnable = VK_FALSE;

        pipelineInfo.viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        pipelineInfo.viewportState.viewportCount = 1;
        pipelineInfo.viewportState.scissorCount = 1;

        pipelineInfo.rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        pipelineInfo.rasterizer.depthClampEnable = VK_FALSE;
        pipelineInfo.rasterizer.rasterizerDiscardEnable = VK_FALSE;
        pipelineInfo.rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
        pipelineInfo.rasterizer.lineWidth = 1.0f;
        pipelineInfo.rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
        pipelineInfo.rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
        pipelineInfo.rasterizer.depthBiasEnable = VK_FALSE;

        pipelineInfo.multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        pipelineInfo.multisampling.sampleShadingEnable = VK_FALSE;
        pipelineInfo.multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;

        VkPipelineColorBlendAttachmentState colorBlendAttachment{};
        colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
        colorBlendAttachment.blendEnable = VK_FALSE;

        pipelineInfo.colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        pipelineInfo.colorBlending.logicOpEnable = VK_FALSE;
        pipelineInfo.colorBlending.logicOp = VK_LOGIC_OP_COPY;
        pipelineInfo.colorBlending.attachmentCount = 1;
        pipelineInfo.colorBlending.pAttachments = &colorBlendAttachment;
        pipelineInfo.colorBlending.blendConstants[0] = 0.0f;
        pipelineInfo.colorBlending.blendConstants[1] = 0.0f;
        pipelineInfo.colorBlending.blendConstants[2] = 0.0f;
        pipelineInfo.colorBlending.blendConstants[3] = 0.0f;

        std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

        pipelineInfo.dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        pipelineInfo.dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
        pipelineInfo.dynamicState.pDynamicStates = dynamicStates.data();

        pipelineInfo.pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineInfo.pipelineLayoutInfo.setLayoutCount = 0;
        pipelineInfo.pipelineLayoutInfo.pushConstantRangeCount = 0;

        pipeline = createPipeline(pipelineInfo);
    }

    void draw(const Vector2i& viewport, float deltaTime) override {
        beginRenderPass(getSwapChainFramebuffer(), viewport);

        bindPipeline(pipeline);
        setViewport({0, 0}, viewport);
        setScissor({0, 0}, viewport);
        drawVertices(3, 1, 0, 0);

        endRenderPass();
    }

    void eventMouseMoved(const Vector2i& pos) override {
    }

    void eventMousePressed(const Vector2i& pos, MouseButton button) override {
    }

    void eventMouseReleased(const Vector2i& pos, MouseButton button) override {
    }

    void eventMouseScroll(int xscroll, int yscroll) override {
    }

    void eventKeyPressed(Key key, Modifiers modifiers) override {
    }

    void eventKeyReleased(Key key, Modifiers modifiers) override {
    }

    void eventWindowResized(const Vector2i& size) override {
    }

    void eventCharTyped(uint32_t code) override {
    }

    void eventWindowBlur() override {
    }

    void eventWindowFocus() override {
    }

    VgPipeline pipeline;
};

int main(int argc, char** argv) {
    const auto defaultRoot = std::filesystem::absolute(getExecutablePath() / ".." / "..");
    const auto defaultUserData = getAppDataPath();

    CLI::App parser{"Temporary Escape"};

    Path rootPath;
    parser.add_option("--root", rootPath, "Root directory")->check(CLI::ExistingDirectory)->default_val(defaultRoot);

    CLI11_PARSE(parser, argc, argv);

    try {
        /*auto args = parser.parse(argc, argv);

        if (args["version"].as<bool>()) {
            std::cout << "v0.0.1" << std::endl;
            return EXIT_SUCCESS;
        }

        const auto userdataPath = std::filesystem::absolute(Path(args["userdata"].as<std::string>()));
        const auto logPath = userdataPath / "debug.log";

        Log::configure(true, logPath);
        Log::i("main", "Temporary Escape main");
        Log::i("main", "Log file location: '{}'", logPath.string());

        const auto rootPath = std::filesystem::absolute(Path(args["root"].as<std::string>()));
        Config config{};
        config.assetsPath = rootPath / "assets";
        config.wrenPaths = {config.assetsPath.string()};
        config.userdataPath = std::filesystem::absolute(Path(args["userdata"].as<std::string>()));
        config.userdataSavesPath = config.userdataPath / "Saves";
        config.shadersPath = rootPath / "shaders";

        if (args.count("save-name")) {
            config.saveFolderName = args["save-name"].as<std::string>();
        }
        config.saveFolderClean = args["save-clean"].as<bool>();
        config.voxelTest = args["voxel-test"].as<bool>();

        std::filesystem::create_directories(config.userdataPath);
        std::filesystem::create_directories(config.userdataSavesPath);*/

        const auto userdataPath = std::filesystem::absolute(Path(defaultUserData));
        const auto logPath = userdataPath / "debug.log";

        // Log::configure(true, logPath);
        Log::i("main", "Temporary Escape main");
        Log::i("main", "Log file location: '{}'", logPath.string());

        rootPath = std::filesystem::absolute(rootPath);
        Config config{};
        config.assetsPath = rootPath / "assets";
        config.userdataPath = std::filesystem::absolute(defaultUserData);
        config.userdataSavesPath = config.userdataPath / "Saves";
        config.shaderCachePath = config.userdataPath / "Shaders";
        config.shadersPath = rootPath / "shaders";
        config.fontsPath = rootPath / "fonts";
        config.shapesPath = rootPath / "shapes";

        std::filesystem::create_directories(config.userdataPath);
        std::filesystem::create_directories(config.userdataSavesPath);
        std::filesystem::create_directories(config.shaderCachePath);

        {
            VgApplication window(config);
            window.run();
        }
        Log::i("main", "Exit success");
        return EXIT_SUCCESS;

    } catch (const std::exception& e) {
        BACKTRACE("Main", e, "fatal error");
        return EXIT_FAILURE;
    }
}
