#version 450
#extension GL_ARB_separate_shader_objects: enable
#include "includes/common.glsl"

layout (std140, binding = 1) uniform Materials {
    SBlockMaterial materials[1024];
};

layout (binding = 2) uniform sampler2DArray baseColorTexture;
layout (binding = 3) uniform sampler2DArray emissiveTexture;
layout (binding = 4) uniform sampler2DArray normalTexture;
layout (binding = 5) uniform sampler2DArray ambientOcclusionTexture;
layout (binding = 6) uniform sampler2DArray metallicRoughnessTexture;
layout (binding = 7) uniform sampler2DArray maskTexture;
layout (binding = 8) uniform sampler1D paletteTexture;

layout (location = 0) in VS_OUT {
    vec3 normal;
    vec2 texCoords;
    vec3 worldPos;
    mat3 TBN;
    float color;
    float materialIndex;
} vs_out;

layout (push_constant) uniform Uniforms {
    mat4 modelMatrix;
    mat3 normalMatrix;
    vec4 entityColor;
} uniforms;

layout (location = 0) out vec4 outColorAmbient;
layout (location = 1) out vec4 outEmissiveRoughness;
layout (location = 2) out vec4 outNormalMetallic;
layout (location = 3) out vec4 outEntity;
layout (location = 4) out vec4 outPosition;

// Source: http://lolengine.net/blog/2013/07/27/rgb-to-hsv-in-glsl
vec3 rgb2hsv(vec3 c) {
    vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
    vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
    vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));

    float d = q.x - min(q.w, q.y);
    float e = 1.0e-10;
    return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 paletteColorForEmissive(vec3 c) {
    vec3 hc = rgb2hsv(c);
    hc.z = 1.0f;
    return hsv2rgb(hc);
}

void main() {
    SBlockMaterial material = materials[int(vs_out.materialIndex)];
    vec3 maskTexCoords = vec3(vs_out.texCoords.xy, material.maskTexture);
    vec3 baseColorTexCoords = vec3(vs_out.texCoords.xy, material.baseColorTexture);
    vec3 normalTexCoords = vec3(vs_out.texCoords.xy, material.normalTexture);
    vec3 emissiveTexCoords = vec3(vs_out.texCoords.xy, material.emissiveTexture);
    vec3 metallicRoughnessTexCoords = vec3(vs_out.texCoords.xy, material.metallicRoughnessTexture);
    vec3 ambientOcclusionTexCoords = vec3(vs_out.texCoords.xy, material.ambientOcclusionTexture);

    float mask = texture(maskTexture, maskTexCoords).r;
    vec4 baseColorRaw = texture(baseColorTexture, baseColorTexCoords);
    vec3 baseColor = pow(baseColorRaw.rgb * material.baseColorFactor.rgb, vec3(2.2));
    vec3 normalRaw = texture(normalTexture, normalTexCoords).xyz;
    normalRaw = vec3(normalRaw.x, normalRaw.y, 0.0) * 2.0 - 1.0;

    normalRaw.z = sqrt(1.0 - normalRaw.x * normalRaw.x - normalRaw.y * normalRaw.y);

    vec3 normal = normalize(vs_out.TBN * normalRaw.xyz);
    vec3 paletteColor = pow(texture(paletteTexture, vs_out.color).rgb, vec3(2.2));

    vec3 emissive = texture(emissiveTexture, emissiveTexCoords).rgb * material.emissiveFactor.rgb;
    vec3 metallicRoughness = texture(metallicRoughnessTexture, metallicRoughnessTexCoords).rgb * material.metallicRoughnessFactor.rgb;
    float ambient = texture(ambientOcclusionTexture, ambientOcclusionTexCoords).r * material.ambientOcclusionFactor.r;

    vec3 emissivePalette = paletteColorForEmissive(paletteColor);

    outColorAmbient = vec4(mix(baseColor.rgb, baseColor.rgb * paletteColor, mask), ambient);
    outEmissiveRoughness = vec4(mix(emissive, emissive * emissivePalette, mask), metallicRoughness.g);
    outNormalMetallic = vec4(normal * 0.5 + 0.5, metallicRoughness.b);
    outEntity = uniforms.entityColor;
    outPosition = vec4(vs_out.worldPos, 1.0);
}
