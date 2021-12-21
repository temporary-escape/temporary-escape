in FS_IN {
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec3 worldpos;
} fs_in;

out FS_OUT {
    vec4 colorRoughness;
    vec4 emissiveAmbient;
    vec4 normalMetallic;
    vec2 objectId;
} fs_out;

// Material
uniform sampler2D baseColorTexture;
uniform sampler2D normalTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D metallicRoughnessTexture;
uniform sampler2D ambientOcclusionTexture;
uniform vec4 objectId;

void main()
{
    vec4 baseColor = texture(baseColorTexture, fs_in.texCoords);
    vec3 normalRaw = vec3(texture(normalTexture, fs_in.texCoords).xy, 0.0) * 2.0 - 1.0;
    normalRaw.z = sqrt(1.0 - dot(normalRaw.xy, normalRaw.xy));
    vec3 emissive = texture(emissiveTexture, fs_in.texCoords).rgb;
    vec3 metallicRoughness = texture(metallicRoughnessTexture, fs_in.texCoords).rgb;
    // float ambient = texture(ambientOcclusionTexture, v_texCoords).r;
    float ambient = 1.0;

    float metallic = metallicRoughness.b;
    float roughness = metallicRoughness.g;

    vec3 normal = normalize(v_TBN * normalRaw);

    fs_out.colorRoughness = vec4(baseColor.rgb, roughness);
    fs_out.emissiveAmbient = vec4(emissive, ambient);
    fs_out.normalMetallic = vec4(normal, metallic);
    fs_out.objectId = vec2(objectId.rg);
}
