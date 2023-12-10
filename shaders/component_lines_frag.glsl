#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec4 color;
} vs_out;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = pow(vs_out.color.rgba, vec4(2.2));
}
