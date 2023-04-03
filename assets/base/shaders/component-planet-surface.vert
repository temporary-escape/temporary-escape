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
    mat3 normalMatrix;
} uniforms;

layout(location = 0) in vec3 in_position;
layout(location = 1) in vec3 in_normal;

layout(location = 0) out VS_OUT {
    vec3 normal;
    vec3 texCoords;
    vec3 worldPos;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vs_out.texCoords = normalize(in_position);

    vec4 worldPos = uniforms.modelMatrix * vec4(in_position.xyz, 1.0);
    vec3 N = normalize(uniforms.normalMatrix * in_normal);

    vs_out.worldPos = worldPos.xyz;
    vs_out.normal = in_normal;

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
