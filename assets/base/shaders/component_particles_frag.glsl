#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec2 texCoords;
    vec4 color;
} vs_out;

layout (location = 0) out vec4 outColor;

void main() {
    outColor = vs_out.color;
}
