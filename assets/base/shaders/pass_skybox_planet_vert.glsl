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
layout(location = 2) in vec2 in_texCoords;
layout(location = 3) in vec4 in_tangent;

layout(location = 0) out VS_OUT {
    vec3 normal;
    vec3 texCoords;
    vec3 worldPos;
    mat3 TBN;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 worldPos = uniforms.modelMatrix * vec4(in_position, 1.0);
    vec3 N = normalize(uniforms.modelMatrix * vec4(in_normal, 0.0)).xyz;
    vec3 T = normalize(uniforms.modelMatrix * vec4(in_tangent.xyz, 0.0)).xyz;

    vs_out.normal = N;
    vs_out.worldPos = worldPos.xyz;
    vs_out.texCoords = normalize(in_position);

    vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
