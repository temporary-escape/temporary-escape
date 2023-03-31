#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in GS_OUT {
    vec2 texCoords;
} gs_out;

layout(push_constant) uniform Uniforms {
    mat4 modelMatrix;
    vec4 color;
} uniforms;

layout(binding = 1) uniform sampler2D colorTexture;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(colorTexture, gs_out.texCoords).rrrr * uniforms.color;
    outColor = pow(color, vec4(2.2));
}
