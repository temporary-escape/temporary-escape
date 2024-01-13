#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec3 normal;
    vec3 texCoords;
    vec3 worldPos;
    mat3 TBN;
} vs_out;

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout (std140, binding = 1) uniform Atmosphere {
    vec4 start;
    vec4 end;
    float strength;
} atmosphere;

layout (binding = 2) uniform samplerCube albedoTexture;
layout (binding = 3) uniform samplerCube normalTexture;
layout (binding = 4) uniform samplerCube metallicRoughnessTexture;

layout (location = 0) out vec4 outColorAmbient;
layout (location = 1) out vec4 outEmissiveRoughness;
layout (location = 2) out vec4 outNormalMetallic;
layout (location = 3) out vec4 outEntity;
layout (location = 4) out vec4 outPosition;

// ---------------------------------------------------------------------------------------------------------------------
vec4 atmosphereColor() {
    // Light scattering
    // TODO: Get system star color
    vec3 light = normalize(-vs_out.worldPos);
    vec3 radiance = vec3(1.0, 1.0, 1.0);

    float d = clamp(dot(vs_out.normal, light), 0.0, 1.0);

    // vec3 eyesDir = normalize(camera.eyesPos - vs_out.worldPos);
    // TODO: Get forward vector if using orthographic camera
    vec3 eyesDir = vec3(0.0, -1.0, 0.0);

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
    normalRaw.z = sqrt(1.0 - normalRaw.x * normalRaw.x - normalRaw.y * normalRaw.y);
    normalRaw = vec3(normalRaw.x, 1.0 - normalRaw.y, normalRaw.z) * 2.0 - 1.0;

    vec3 normal = normalize(vs_out.TBN * normalRaw.xyz);

    vec2 metallicRoughness = texture(metallicRoughnessTexture, vs_out.texCoords).rg;
    float metallic = metallicRoughness.r;
    float roughness = metallicRoughness.g;

    vec3 emissive = atmosphereColor().rgb;

    outColorAmbient = vec4(albedo, 1.0);
    outEmissiveRoughness = vec4(emissive * 0.1, roughness);
    outNormalMetallic = vec4(normal * 0.5 + 0.5, metallic);
    outEntity = vec4(1.0, 1.0, 1.0, 1.0);
    outPosition = vec4(vs_out.worldPos, 1.0);
}
