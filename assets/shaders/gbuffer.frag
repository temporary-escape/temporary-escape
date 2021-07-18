layout(location = 0) out vec4 o_ColorRoughness;
layout(location = 1) out vec4 o_EmissiveAmbient;
layout(location = 2) out vec4 o_NormalMetallic;
layout(location = 3) out vec2 o_ObjectId;

in vec3 v_normal;
in vec2 v_texCoords;
in mat3 v_TBN;
in vec3 v_worldpos;

// Material
uniform sampler2D baseColorTexture;
uniform sampler2D normalTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D ambientOcclusionTexture;
uniform vec4 objectId;

void main()
{
    vec4 baseColor = texture(baseColorTexture, v_texCoords);
    vec3 normalRaw = vec3(texture(normalTexture, v_texCoords).xy, 0.0) * 2.0 - 1.0;
    normalRaw.z = sqrt(1.0 - dot(normalRaw.xy, normalRaw.xy));
    vec3 emissive = texture(emissiveTexture, v_texCoords).rgb;
    vec3 metallicRoughness = texture(metallicRoughnessTexture, v_texCoords).rgb;
    // float ambient = texture(ambientOcclusionTexture, v_texCoords).r;
    float ambient = 1.0;

    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;

    vec3 normal = normalize(v_TBN * normalRaw); 

    o_ColorRoughness = vec4(baseColor.rgb, roughness);
    o_EmissiveAmbient = vec4(emissive, ambient);
    o_NormalMetallic = vec4(normal, metallic);
    o_ObjectId = vec2(objectId.rg);
}
