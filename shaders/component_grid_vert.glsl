#version 450
#extension GL_ARB_separate_shader_objects: enable
#include "includes/common.glsl"

layout (std140, binding = 0) uniform Camera {
    SCamera camera;
};

layout (push_constant) uniform Uniforms {
    mat4 modelMatrix;
    mat3 normalMatrix;
    vec4 entityColor;
} uniforms;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec4 in_TexCoords;
layout (location = 3) in vec4 in_Tangent;

layout (location = 0) out VS_OUT {
    vec3 normal;
    vec2 texCoords;
    vec3 worldPos;
    mat3 TBN;
    float color;
    float materialIndex;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 worldPos = uniforms.modelMatrix * vec4(in_Position, 1.0);
    vec3 N = normalize(uniforms.modelMatrix * vec4(in_Normal, 0.0)).xyz;
    vec3 T = normalize(uniforms.modelMatrix * vec4(in_Tangent.xyz, 0.0)).xyz;

    vs_out.normal = N;
    vs_out.worldPos = worldPos.xyz;
    vs_out.texCoords = in_TexCoords.xy;
    vs_out.materialIndex = in_TexCoords.z;
    vs_out.color = in_TexCoords.w;

    vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
