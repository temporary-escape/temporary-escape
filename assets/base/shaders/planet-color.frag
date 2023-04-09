#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (input_attachment_index = 0, binding = 0) uniform subpassInput samplerHeightmap;
layout (input_attachment_index = 1, binding = 1) uniform subpassInput samplerMoisture;

layout(location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout(location = 0) out vec4 outColor;
layout(location = 1) out vec4 outMetallicRoughness;

layout(binding = 2) uniform sampler2D textureBiome;
layout(binding = 3) uniform sampler2D textureRoughness;

void main() {
    float height = subpassLoad(samplerHeightmap).r;
    float moisture = subpassLoad(samplerMoisture).r;

    vec3 biome = texture(textureBiome, vec2(moisture, height)).rgb;
    float roughness = texture(textureRoughness, vec2(moisture, height)).r;

    outColor = vec4(biome, 1.0);
    outMetallicRoughness = vec4(0.1, roughness, 0.0, 1.0);
}
