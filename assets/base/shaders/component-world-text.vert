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
    vec4 color;
} uniforms;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec2 in_Offset;
layout(location = 2) in vec2 in_Size;
layout(location = 3) in vec2 in_Uv;
layout(location = 4) in vec2 in_St;

layout(location = 0) out VS_OUT {
    vec2 size;
    vec2 offset;
    vec2 uv;
    vec2 st;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vs_out.size = in_Size;
    vs_out.offset = in_Offset;
    vs_out.uv = in_Uv;
    vs_out.st = in_St;
    vec4 worldPos = uniforms.modelMatrix * vec4(in_Position, 1.0);
    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
