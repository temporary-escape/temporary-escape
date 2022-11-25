#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Position;

layout(push_constant) uniform Uniforms {
    mat4 viewMatrix;
    mat4 projectionMatrix;
} uniforms;

layout(location = 0) out VS_OUT {
    vec3 position;
} vs_out;

void main() {
    vec4 worldPos = vec4(in_Position, 1);
    vs_out.position = in_Position;
    gl_Position = uniforms.projectionMatrix * uniforms.viewMatrix * worldPos;
}
