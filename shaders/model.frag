in VS_OUT {
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec3 worldpos;
} vsOut;

layout(location = 0) out vec4 fsOut_colorRoughness;
layout(location = 1) out vec4 fsOut_emissiveAmbient;
layout(location = 2) out vec4 fsOut_normalMetallic;
layout(location = 3) out vec2 fsOut_objectId;

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

uniform vec4 objectId;

void main() {
    vec4 baseColor = texture(baseColorTexture, vsOut.texCoords);
    vec3 normalRaw = vec3(texture(normalTexture, vsOut.texCoords).xy, 0.0) * 2.0 - 1.0;
    normalRaw.z = sqrt(1.0 - dot(normalRaw.xy, normalRaw.xy));
    vec3 emissive = texture(emissiveTexture, vsOut.texCoords).rgb;
    vec3 metallicRoughness = texture(metallicRoughnessTexture, vsOut.texCoords).rgb;
    float ambient = texture(ambientOcclusionTexture, vsOut.texCoords).r;
    // float ambient = 1.0;

    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;

    vec3 normal = normalize(vsOut.TBN * normalRaw);

    fsOut_colorRoughness = vec4(baseColor.rgb, roughness);
    fsOut_emissiveAmbient = vec4(emissive, ambient);
    fsOut_normalMetallic = vec4(normal, metallic);
    fsOut_objectId = vec2(objectId.rg);
}
