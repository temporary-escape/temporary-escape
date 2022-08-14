#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout(binding = 0) uniform sampler2D texBaseColor;
layout(binding = 1) uniform sampler2D texEmissive;
layout(binding = 2) uniform sampler2D texMetallicRoughnessAmbient;
layout(binding = 3) uniform sampler2D texNormal;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 baseColorRaw = texture(texBaseColor, vs_out.texCoords);
    vec4 emissiveRaw = texture(texBaseColor, vs_out.texCoords);
    vec4 metallicRoughnessAmbientRaw = texture(texBaseColor, vs_out.texCoords);
    vec4 normalRaw = texture(texBaseColor, vs_out.texCoords);

    float alpha = baseColorRaw.a;
    vec3 albedo = baseColorRaw.rgb;
    vec3 emissive = emissiveRaw.rgb;
    float metallic = metallicRoughnessAmbientRaw.r;
    float roughness = metallicRoughnessAmbientRaw.g;
    float ambientOcclusion = metallicRoughnessAmbientRaw.b;
    vec3 normal = normalRaw.rgb;

    outColor = vec4(albedo, alpha);
}
