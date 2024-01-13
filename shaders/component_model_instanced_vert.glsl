#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;
layout (location = 3) in vec4 in_Tangent;
layout (location = 4) in vec4 in_EntityColor;
layout (location = 5) in mat4 in_ModelMatrix;

layout (location = 0) out VS_OUT {
    vec3 normal;
    vec2 texCoords;
    vec3 worldPos;
    mat3 TBN;
    vec4 entityColor;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 worldPos = in_ModelMatrix * vec4(in_Position, 1.0);
    vec3 N = normalize(in_ModelMatrix * vec4(in_Normal, 0.0)).xyz;
    vec3 T = normalize(in_ModelMatrix * vec4(in_Tangent.xyz, 0.0)).xyz;

    vs_out.normal = N;
    vs_out.worldPos = worldPos.xyz;
    vs_out.texCoords = in_TexCoords;
    vs_out.entityColor = in_EntityColor;

    vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
