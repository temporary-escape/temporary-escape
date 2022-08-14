#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 1) uniform Material {
    vec4 baseColorFactor;
    vec4 emissiveFactor;
    vec4 normalFactor;
    vec4 ambientOcclusionFactor;
    vec4 metallicRoughnessFactor;
} material;

layout(binding = 2) uniform sampler2D baseColorTexture;
layout(binding = 3) uniform sampler2D emissiveTexture;
layout(binding = 4) uniform sampler2D normalTexture;
layout(binding = 5) uniform sampler2D ambientOcclusionTexture;
layout(binding = 6) uniform sampler2D metallicRoughnessTexture;

layout(location = 0) in GS_OUT {
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec3 worldpos;
} gs_out;

layout(location = 1) out vec4 outColor;
layout(location = 2) out vec4 outEmissive;
layout(location = 3) out vec4 outMetallicRoughnessAmbient;
layout(location = 4) out vec4 outNormal;

void main() {
    vec4 baseColor = texture(baseColorTexture, gs_out.texCoords);
    baseColor *= material.baseColorFactor.rgba;
    if (baseColor.a < 0.5) {
        discard;
    }

    vec3 normalRaw = vec3(texture(normalTexture, gs_out.texCoords).xy, 0.0) * 2.0 - 1.0;
    normalRaw *=  vec3(1.0, -1.0, 1.0);

    normalRaw.z = sqrt(1.0 - dot(normalRaw.xy, normalRaw.xy));
    vec3 normal = normalize(gs_out.TBN * normalRaw);
    vec3 emissive = texture(emissiveTexture, gs_out.texCoords).rgb;
    vec3 metallicRoughness = texture(metallicRoughnessTexture, gs_out.texCoords).rgb;
    float ambient = texture(ambientOcclusionTexture, gs_out.texCoords).r;

    outColor = vec4(baseColor.rgb, 1.0f);
    outEmissive = vec4(emissive, 1.0f);
    outMetallicRoughnessAmbient = vec4(metallicRoughness.g, metallicRoughness.b, ambient, 1.0f);
    outNormal = vec4(normal, 1.0f);
}
