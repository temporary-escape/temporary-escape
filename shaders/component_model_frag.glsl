#version 450
#extension GL_ARB_separate_shader_objects: enable
#include "includes/common.glsl"

layout (std140, set = 1, binding = 0) uniform Material {
    vec4 baseColorFactor;
    vec4 emissiveFactor;
    vec4 normalFactor;
    vec4 ambientOcclusionFactor;
    vec4 metallicRoughnessFactor;
} material;

layout (set = 1, binding = 1) uniform sampler2D baseColorTexture;
layout (set = 1, binding = 2) uniform sampler2D emissiveTexture;
layout (set = 1, binding = 3) uniform sampler2D normalTexture;
layout (set = 1, binding = 4) uniform sampler2D ambientOcclusionTexture;
layout (set = 1, binding = 5) uniform sampler2D metallicRoughnessTexture;

layout (location = 0) in VS_OUT {
    vec3 normal;
    vec2 texCoords;
    vec3 worldPos;
    mat3 TBN;
    vec4 entityColor;
} vs_out;

layout (location = 0) out vec4 outColorAmbient;
layout (location = 1) out vec4 outEmissiveRoughness;
layout (location = 2) out vec4 outNormalMetallic;
layout (location = 3) out vec4 outEntity;
layout (location = 4) out vec4 outPosition;

void main() {
    vec3 baseColor = pow(texture(baseColorTexture, vs_out.texCoords).rgb * material.baseColorFactor.rgb, vec3(2.2));
    vec3 normalRaw = texture(normalTexture, vs_out.texCoords).xyz;
    normalRaw.z = sqrt(1.0 - normalRaw.x * normalRaw.x - normalRaw.y * normalRaw.y);
    normalRaw = vec3(normalRaw.x, 1.0 - normalRaw.y, normalRaw.z) * 2.0 - 1.0;
    vec3 normal = normalize(vs_out.TBN * normalRaw.xyz);

    vec3 emissive = texture(emissiveTexture, vs_out.texCoords).rgb * material.baseColorFactor.rgb;
    vec3 metallicRoughness = texture(metallicRoughnessTexture, vs_out.texCoords).rgb * material.metallicRoughnessFactor.rgb;
    float ambient = texture(ambientOcclusionTexture, vs_out.texCoords).r * material.ambientOcclusionFactor.r;

    outColorAmbient = vec4(baseColor.rgb, ambient);
    outEmissiveRoughness = vec4(emissive, metallicRoughness.g);
    outNormalMetallic = vec4(normal * 0.5 + 0.5, metallicRoughness.b);
    outEntity = vs_out.entityColor;
    outPosition = vec4(vs_out.worldPos, 1.0);
}
