#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout(binding = 0) uniform sampler2D texColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = texture(texColor, vs_out.texCoords);
}
