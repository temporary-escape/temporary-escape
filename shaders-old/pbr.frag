in VS_OUT {
    vec2 texCoords;
} vsOut;

layout(location = 0) out vec4 o_color;

uniform sampler2D depthTexture;
uniform sampler2D baseColorTexture;
uniform sampler2D emissiveTexture;
uniform sampler2D metallicRoughnessAmbientTexture;
uniform sampler2D normalTexture;
uniform samplerCube irradianceTexture;
uniform samplerCube prefilterTexture;
uniform sampler2D brdfTexture;
uniform sampler2D ssaoTexture;

layout (std140) uniform DirectionalLights {
    vec4 colors[4];
    vec4 directions[4];
    int count;
} directionalLights;

layout (std140) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

const float PI = 3.14159265359;

// ----------------------------------------------------------------------------
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

// ----------------------------------------------------------------------------
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
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
    clipSpaceLocation.z = depth * 2.0f - 1.0f;
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
void main() {
    float depth = texture(depthTexture, vsOut.texCoords).r;
    if (depth > 0.9999) {
        discard;
    }

    vec3 albedo = texture(baseColorTexture, vsOut.texCoords).rgb;
    albedo = pow(albedo, vec3(2.2));
    vec3 metallicRoughnessAmbient = texture(metallicRoughnessAmbientTexture, vsOut.texCoords).rgb;
    vec3 N = texture(normalTexture, vsOut.texCoords).rgb;
    float metallic = metallicRoughnessAmbient.r;
    float roughness = metallicRoughnessAmbient.g;
    vec3 emissive = texture(emissiveTexture, vsOut.texCoords).rgb * 1.0;
    float ao = metallicRoughnessAmbient.b;
    float ssao = texture(ssaoTexture, vsOut.texCoords).r;

    // Get world pos from UV and camera projection matrix
    vec3 worldpos = getWorldPos(depth, vsOut.texCoords);

    // Screen space normals
    vec3 SSN = (camera.viewMatrix * vec4(N, 0.0)).xyz;

    // input lighting data
    vec3 V = normalize(camera.eyesPos - worldpos);
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);

    // reflectance equation
    vec3 Lo = vec3(0.0);
    for (int i = 0; i < directionalLights.count; ++i)
    {
        // calculate per-light radiance
        /*vec3 L = normalize(lightPositions[i] - worldpos);
        vec3 H = normalize(V + L);
        float distance = length(lightPositions[i] - worldpos);
        float attenuation = 1.0 / (distance * distance);
        vec3 radiance = lightColors[i] * attenuation;*/

        vec3 L = normalize(directionalLights.directions[i].xyz);
        vec3 H = normalize(V + L);
        vec3 radiance = directionalLights.colors[i].xyz;

        // Cook-Torrance BRDF
        float NDF = DistributionGGX(N, H, roughness);
        float G   = GeometrySmith(N, V, L, roughness);
        vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);

        vec3 nominator    = NDF * G * F;
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
    vec3 F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);

    vec3 kS = F;
    vec3 kD = 1.0 - kS;
    kD *= 1.0 - metallic;

    vec3 irradiance = texture(irradianceTexture, N).rgb;
    vec3 diffuse = irradiance * albedo.xyz;

    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterTexture, R, roughness * MAX_REFLECTION_LOD).rgb;
    vec2 brdf  = texture(brdfTexture, vec2(max(dot(N, V), 0.0), roughness)).rg;
    vec3 specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular) * ao;

    vec3 color = (ambient + Lo) * pow(ssao, 1.5) + emissive;

    // HDR tonemapping
    //color = color / (color + vec3(1.0));

    // exposure tone mapping
    color = vec3(1.0) - exp(-color * 1.25);

    // gamma correct
    color = pow(color, vec3(1.0/2.2));

    o_color = vec4(color, 1.0);
}