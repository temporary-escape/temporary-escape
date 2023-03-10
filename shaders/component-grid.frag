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
layout(binding = 7) uniform sampler1D paletteTexture;

layout(location = 0) in VS_OUT {
    vec3 normal;
    vec2 texCoords;
    vec3 worldpos;
    mat3 TBN;
    float color;
} vs_out;

layout(location = 0) out vec4 outColorAmbient;
layout(location = 1) out vec4 outEmissiveRoughness;
layout(location = 2) out vec4 outNormalMetallic;

void main() {
    vec3 baseColor = pow(texture(baseColorTexture, vs_out.texCoords).rgb * material.baseColorFactor.rgb, vec3(2.2));
    vec3 normalRaw = texture(normalTexture, vs_out.texCoords).xyz;
    normalRaw = vec3(normalRaw.x, 1.0 - normalRaw.y, normalRaw.z) * 2.0 - 1.0;
    vec3 normal = normalize(vs_out.TBN * normalRaw.xyz);
    vec3 paletteColor = texture(paletteTexture, vs_out.color).rgb;

    vec3 emissive = texture(emissiveTexture, vs_out.texCoords).rgb * material.baseColorFactor.rgb;
    vec3 metallicRoughness = texture(metallicRoughnessTexture, vs_out.texCoords).rgb * material.metallicRoughnessFactor.rgb;
    float ambient = texture(ambientOcclusionTexture, vs_out.texCoords).r * material.ambientOcclusionFactor.r;

    outColorAmbient = vec4(baseColor.rgb * paletteColor, ambient);
    outEmissiveRoughness = vec4(emissive, metallicRoughness.g);
    outNormalMetallic = vec4(normal * 0.5 + 0.5, metallicRoughness.b);
}
