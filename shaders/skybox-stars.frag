#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in GS_OUT {
    vec2 coords;
    float brightness;
    vec4 color;
} gs_out;

layout(location = 0) out vec4 outColor;

void main() {
    float dist = pow(clamp(1.0 - length(gs_out.coords), 0.0, 1.0), 0.5);
    outColor = gs_out.color * dist * gs_out.brightness;
}
