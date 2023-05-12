#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Position;

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout(location = 0) out VS_OUT {
    vec3 position;
} vs_out;

void main() {
    vec4 worldPos = vec4(in_Position, 1);
    vs_out.position = in_Position;
    gl_Position = camera.projectionMatrix * camera.viewMatrix * worldPos;
}
