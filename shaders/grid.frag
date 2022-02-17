in VS_OUT {
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec3 worldpos;
} vsOut;

layout(location = 0) out vec3 fsOut_baseColor;
layout(location = 1) out vec3 fsOut_emissive;
layout(location = 2) out vec3 fsOut_metallicRoughnessAmbient;
layout(location = 3) out vec3 fsOut_normal;

uniform Material {
    vec4 baseColorFactor;
    vec4 emissiveFactor;
    vec4 metallicRoughnessFactor;
} material;

uniform sampler2D baseColorTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D normalTexture;
uniform sampler2D ambientOcclusionTexture;
uniform sampler2D metallicRoughnessTexture;

void main() {
    vec4 baseColor = texture(baseColorTexture, vsOut.texCoords);
    if (baseColor.a < 0.5) {
        discard;
    }

    vec3 normalRaw = vec3(texture(normalTexture, vsOut.texCoords).xy, 0.0) * 2.0 - 1.0;
    normalRaw.z = sqrt(1.0 - dot(normalRaw.xy, normalRaw.xy));
    vec3 emissive = texture(emissiveTexture, vsOut.texCoords).rgb;
    vec3 metallicRoughness = texture(metallicRoughnessTexture, vsOut.texCoords).rgb;
    float ambient = texture(ambientOcclusionTexture, vsOut.texCoords).r;

    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;

    vec3 normal = normalize(vsOut.TBN * normalRaw);

    fsOut_baseColor = baseColor.rgb;
    fsOut_emissive = emissive;
    fsOut_metallicRoughnessAmbient = vec3(metallic, roughness, ambient);
    fsOut_normal = normal.rgb;
}
