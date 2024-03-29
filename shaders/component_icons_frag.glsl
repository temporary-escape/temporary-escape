#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec4 color;
    vec2 texCoords;
} vs_out;

layout (binding = 1) uniform sampler2D colorTexture;

layout (location = 0) out vec4 outColor;

void main() {
    vec4 color = pow(texture(colorTexture, vs_out.texCoords), vec4(2.2)) * vs_out.color;
    outColor = color;
}
