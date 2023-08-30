#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout (std140, binding = 1) uniform DirectionalLights {
    vec4 colors[4];
    vec4 directions[4];
    int count;
} directionalLights;

layout (std140, binding = 2) uniform ShadowsViewProj {
    mat4 lightMat[4];
    vec4 cascadeSplits;
} shadowsViewProj;

layout (binding = 3) uniform sampler2D texDepth;
layout (binding = 4) uniform sampler2D texBaseColorAmbient;
layout (binding = 5) uniform sampler2D texEmissiveRoughness;
layout (binding = 6) uniform sampler2D texNormalMetallic;
layout (binding = 7) uniform samplerCube texIrradiance;
layout (binding = 8) uniform samplerCube texPrefilter;
layout (binding = 9) uniform sampler2D texBrdf;
layout (binding = 10) uniform sampler2D texSsao;
layout (binding = 11) uniform sampler2DArray texShadows;

layout (location = 0) out vec4 outColor;

const float PI = 3.14159265359;
const float gamma = 2.2;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float nom = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float nom = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

// ----------------------------------------------------------------------------
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

// ----------------------------------------------------------------------------
vec3 fresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness)
{
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(1.0 - cosTheta, 5.0);
}

// ----------------------------------------------------------------------------
vec3 vertexPos(float depth, vec2 texCoords) {
    return vec3(texCoords * 2.0 - 1.0, 0.0);
}

// ----------------------------------------------------------------------------
vec4 getClipSpacePos(float depth, vec2 texCoords) {
    // Source: https://stackoverflow.com/questions/22360810/reconstructing-world-coordinates-from-depth-buffer-and-arbitrary-view-projection
    vec4 clipSpaceLocation;
    clipSpaceLocation.xy = texCoords * 2.0f - 1.0f;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;

    return clipSpaceLocation;
}

// ----------------------------------------------------------------------------
vec3 getWorldPos(float depth, vec2 texCoords) {
    // Source: https://stackoverflow.com/questions/22360810/reconstructing-world-coordinates-from-depth-buffer-and-arbitrary-view-projection
    vec4 clipSpaceLocation = getClipSpacePos(depth, texCoords);
    vec4 homogenousLocation = camera.viewProjectionInverseMatrix * clipSpaceLocation;
    vec3 worldPos = homogenousLocation.xyz / homogenousLocation.w;

    return worldPos;
}

// ----------------------------------------------------------------------------
uint getShadowCascade(vec3 viewPos) {
    uint cascadeIndex = 0;
    for (uint i = 0; i < 4 - 1; i++) {
        if (viewPos.z > shadowsViewProj.cascadeSplits[i]) {
            cascadeIndex = i + 1;
        }
    }
    return cascadeIndex;
}

// ----------------------------------------------------------------------------
float textureProj(vec4 shadowCoord, vec2 offset, uint cascadeIndex) {
    float shadow = 1.0;
    float bias = 0.0002;

    if (shadowCoord.z > -1.0 && shadowCoord.z < 1.0) {
        float dist = texture(texShadows, vec3(shadowCoord.st * 0.5 + 0.5 + offset, cascadeIndex)).r;
        if (shadowCoord.w > 0 && dist < shadowCoord.z - bias) {
            shadow = 0.0;
        }
    }
    return shadow;
}

// ----------------------------------------------------------------------------
float filterPCF(vec4 sc, uint cascadeIndex) {
    ivec2 texDim = textureSize(texShadows, 0).xy;
    float scale = 0.75;
    float dx = scale * 1.0 / float(texDim.x);
    float dy = scale * 1.0 / float(texDim.y);

    float shadowFactor = 0.0;
    int count = 0;
    int range = 3;

    for (int x = -range; x <= range; x++) {
        for (int y = -range; y <= range; y++) {
            shadowFactor += textureProj(sc, vec2(dx * x, dy * y), cascadeIndex);
            count++;
        }
    }
    return shadowFactor / count;
}

// ----------------------------------------------------------------------------
vec3 getShadowValue(vec3 viewPos, vec3 worldPos) {
    uint cascadeIndex = getShadowCascade(viewPos);

    vec4 shadowMapPosition = shadowsViewProj.lightMat[cascadeIndex] * vec4(worldPos, 1.0);

    return vec3(filterPCF(shadowMapPosition / shadowMapPosition.w, cascadeIndex));
}

// ----------------------------------------------------------------------------
void main() {
    float depth = texture(texDepth, vs_out.texCoords).r;
    if (depth > 0.99999) {
        discard;
    }

    vec4 baseColorAmbientRaw = texture(texBaseColorAmbient, vs_out.texCoords);
    vec4 emissiveRoughnessRaw = texture(texEmissiveRoughness, vs_out.texCoords);
    vec4 normalMetallicRaw = texture(texNormalMetallic, vs_out.texCoords);
    float ssao = texture(texSsao, vs_out.texCoords).r;

    vec3 albedo = baseColorAmbientRaw.rgb;
    vec3 emissive = emissiveRoughnessRaw.rgb;
    float metallic = normalMetallicRaw.a;
    float roughness = emissiveRoughnessRaw.a;
    float ambientOcclusion = baseColorAmbientRaw.a;
    vec3 N = normalize(normalMetallicRaw.rgb * 2.0 - 1.0);

    // Get world pos from UV and camera projection matrix
    vec3 worldpos = getWorldPos(depth, vs_out.texCoords);

    // input lighting data
    vec3 V = normalize(camera.eyesPos - worldpos);
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    vec3 viewPos = (camera.viewMatrix * vec4(worldpos, 1.0)).xyz;
    vec3 shadowValue = getShadowValue(viewPos, worldpos);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < directionalLights.count; ++i)
    {
        // calculate per-light radiance
        /*vec3 L = normalize(directionalLights.directions[i].xyz - worldpos);
            vec3 H = normalize(V + L);
            float distance = length(directionalLights.directions[i].xyz - worldpos);
            float attenuation = 1.0 / (distance * distance);
            vec3 radiance = directionalLights.colors[i].xyz * attenuation;*/

        vec3 L = normalize(directionalLights.directions[i].xyz);
        if (directionalLights.directions[i].w > 0.5) {
            L = normalize(directionalLights.directions[i].xyz - worldpos);
        }

        vec3 H = normalize(V + L);
        vec3 radiance = directionalLights.colors[i].xyz;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G = GeometrySmith(N, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;// 0.001 to prevent divide by zero.
        vec3 specular = nominator / denominator;

        // kS is equal to Fresnel
        vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
        vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
        kD *= 1.0 - metallic;

        // scale light by NdotL
        float NdotL = max(dot(N, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (we now use IBL as the ambient term)
    // vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 F = F0;

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(texIrradiance, N).rgb * 0.7;
    vec3 diffuse = irradiance * albedo.xyz;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(texPrefilter, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(texBrdf, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ambientOcclusion;

    vec3 color = (ambient + Lo * shadowValue) * ssao + emissive;

    outColor = vec4(color, 1.0);
}
