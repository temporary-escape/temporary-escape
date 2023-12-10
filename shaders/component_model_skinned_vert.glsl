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

layout (std140, binding = 2) uniform Armature {
    mat4 joints[16];
    int count;
} armature;

layout (push_constant) uniform Uniforms {
    mat4 modelMatrix;
    mat3 normalMatrix;
    vec4 entityColor;
} uniforms;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Normal;
layout (location = 2) in vec2 in_TexCoords;
layout (location = 3) in vec4 in_Tangent;
layout (location = 4) in vec4 in_Joints;
layout (location = 5) in vec4 in_Weights;

layout (location = 0) out VS_OUT {
    vec3 normal;
    vec2 texCoords;
    vec3 worldpos;
    mat3 TBN;
    vec4 entityColor;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    mat4 skinMat =
    in_Weights.x * armature.joints[int(in_Joints.x)] +
    in_Weights.y * armature.joints[int(in_Joints.y)] +
    in_Weights.z * armature.joints[int(in_Joints.z)] +
    in_Weights.w * armature.joints[int(in_Joints.w)];

    vec4 worldPos = uniforms.modelMatrix * skinMat * vec4(in_Position, 1.0);
    vec3 N = normalize(uniforms.normalMatrix * mat3(skinMat) * in_Normal);
    vec3 T = normalize(uniforms.normalMatrix * mat3(skinMat) * in_Tangent.xyz);

    vs_out.normal = N;
    vs_out.worldpos = worldPos.xyz;
    vs_out.texCoords = in_TexCoords;
    vs_out.entityColor = uniforms.entityColor;

    vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
