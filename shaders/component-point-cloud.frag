#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in GS_OUT {
    vec4 color;
    vec2 texCoords;
} gs_out;

layout(binding = 1) uniform sampler2D colorTexture;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(colorTexture, gs_out.texCoords) * gs_out.color;
    outColor = pow(color, vec4(2.2));
}
