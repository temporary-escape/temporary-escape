#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) out vec4 outColor;

layout(location = 0) in VS_OUT {
    vec4 color;
} vs_out;

void main() {
    outColor = pow(vs_out.color, vec4(2.2));
}
