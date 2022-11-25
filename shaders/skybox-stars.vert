#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Position;
layout(location = 1) in float in_Brightness;
layout(location = 2) in vec4 in_Color;

layout(push_constant) uniform Uniforms {
    mat4 viewMatrix;
    mat4 projectionMatrix;
    vec2 particleSize;
} uniforms;

layout(location = 0) out VS_OUT {
    float brightness;
    vec4 color;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vs_out.brightness = in_Brightness;
    vs_out.color = in_Color;
    gl_Position = uniforms.viewMatrix * vec4(in_Position, 1.0);
}
