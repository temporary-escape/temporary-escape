in GS_OUT {
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec3 worldpos;
} gsOut;

layout(location = 0) out vec3 fsOut_baseColor;
layout(location = 1) out vec3 fsOut_emissive;
layout(location = 2) out vec3 fsOut_metallicRoughnessAmbient;
layout(location = 3) out vec3 fsOut_normal;
layout(location = 4) out vec2 fsOut_objectId;

uniform Material {
    vec4 baseColorFactor;
    vec4 emissiveFactor;
    vec4 metallicRoughnessFactor;
} material;

uniform vec2 objectId;
uniform sampler2D baseColorTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D normalTexture;
uniform sampler2D ambientOcclusionTexture;
uniform sampler2D metallicRoughnessTexture;

void main() {
    vec4 baseColor = texture(baseColorTexture, gsOut.texCoords);
    baseColor *= material.baseColorFactor.rgba;
    if (baseColor.a < 0.5) {
        discard;
    }

    vec3 normalRaw = vec3(texture(normalTexture, gsOut.texCoords).xy, 0.0) * 2.0 - 1.0;
    normalRaw.z = sqrt(1.0 - dot(normalRaw.xy, normalRaw.xy));
    vec3 emissive = texture(emissiveTexture, gsOut.texCoords).rgb * vec3(1.0, 0.9, 0.6);
    vec3 metallicRoughness = texture(metallicRoughnessTexture, gsOut.texCoords).rgb;
    float ambient = texture(ambientOcclusionTexture, gsOut.texCoords).r;

    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;
    emissive *= material.emissiveFactor.rgb;
    metallic *= material.metallicRoughnessFactor.b;
    roughness *= material.metallicRoughnessFactor.g;

    vec3 normal = normalize(gsOut.TBN * normalRaw);
    //vec3 normal = gsOut.normal;

    fsOut_baseColor = baseColor.rgb;
    fsOut_emissive = emissive;
    fsOut_metallicRoughnessAmbient = vec3(metallic, roughness, ambient);
    fsOut_normal = normal.rgb;
    fsOut_objectId = objectId;
}
