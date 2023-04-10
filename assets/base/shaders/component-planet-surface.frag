#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec3 normal;
    vec3 texCoords;
    vec3 worldPos;
    mat3 TBN;
} vs_out;

layout(location = 0) out vec4 outColor;

layout (std140, binding = 1) uniform DirectionalLights {
    vec4 colors[4];
    vec4 directions[4];
    int count;
} directionalLights;

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout (std140, binding = 2) uniform Atmosphere {
    vec4 start;
    vec4 end;
    float strength;
} atmosphere;

layout (binding = 3) uniform samplerCube albedoTexture;
layout (binding = 4) uniform samplerCube normalTexture;
layout (binding = 5) uniform samplerCube metallicRoughnessTexture;
layout (binding = 6) uniform samplerCube irradianceTexture;
layout (binding = 7) uniform samplerCube prefilterTexture;
layout (binding = 8) uniform sampler2D brdfTexture;

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

vec3 getColor(vec3 worldPos, vec3 albedo, vec3 normal, float metallic, float roughness) {
    // input lighting data
    vec3 V = normalize(camera.eyesPos - worldPos);
    vec3 R = reflect(-V, normal);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

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
        vec3 H = normalize(V + L);
        vec3 radiance = directionalLights.colors[i].xyz;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(normal, H, roughness);
        float G = GeometrySmith(normal, V, L, roughness);
        vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator = NDF * G * F;
        float denominator = 4 * max(dot(normal, V), 0.0) * max(dot(normal, L), 0.0) + 0.001;// 0.001 to prevent divide by zero.
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
        float NdotL = max(dot(normal, L), 0.0);

        // add to outgoing radiance Lo
        Lo += (kD * albedo / PI + specular) * radiance * NdotL;// note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again
    }

    // ambient lighting (we now use IBL as the ambient term)
    // vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    vec3 F = F0;

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceTexture, normal).rgb * 0.1;
    vec3 diffuse = irradiance * albedo.xyz;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterTexture, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf = texture(brdfTexture, vec2(max(dot(normal, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y) * 0.5;

    vec3 ambient = kD * diffuse + specular;

    return ambient + Lo;
}

// ---------------------------------------------------------------------------------------------------------------------
vec4 atmosphereColor() {
    // Light scattering
    vec3 light = normalize(directionalLights.directions[0].xyz);
    vec3 radiance = directionalLights.colors[0].xyz;

    float d = clamp(dot(vs_out.normal, light), 0.0, 1.0);
    // d = pow(d, 1.0);

    vec3 eyesDir = normalize(camera.eyesPos - vs_out.worldPos);

    float f = 1.0 - clamp(dot(vs_out.normal, eyesDir), 0.0, 1.0);
    f = pow(f, 4.5) + 0.05;

    float factor = clamp(f * d, 0.0, 1.0);

    vec4 atmosphereColor = mix(pow(atmosphere.start, vec4(2.2)), pow(atmosphere.end, vec4(2.2)), pow(f, 1.5));
    // atmosphereColor *= vec4(radiance, 1.0);

    return atmosphereColor * vec4(factor * 1.0) * atmosphere.strength;
}

// ---------------------------------------------------------------------------------------------------------------------
void main() {
    vec3 albedo = pow(texture(albedoTexture, vs_out.texCoords).rgb, vec3(2.2));
    vec3 normalRaw = texture(normalTexture, vs_out.texCoords).xyz;
    normalRaw = vec3(normalRaw.x, 1.0 - normalRaw.y, normalRaw.z) * 2.0 - 1.0;
    vec3 normal = normalize(vs_out.TBN * normalRaw.xyz);

    vec2 metallicRoughness = texture(metallicRoughnessTexture, vs_out.texCoords).rg;
    float metallic = metallicRoughness.r;
    float roughness = metallicRoughness.g;

    vec3 color = getColor(vs_out.worldPos, albedo, normal, metallic, roughness) + atmosphereColor().rgb;
    outColor = vec4(color, 1.0);
}
