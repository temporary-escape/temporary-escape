#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout(push_constant) uniform Uniforms {
    mat4 modelMatrix;
} uniforms;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec4 in_Color;

layout(location = 0) out VS_OUT {
    vec4 color;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 worldPos = uniforms.modelMatrix * vec4(in_Position.xyz, 1.0);
    vs_out.color = in_Color;

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
