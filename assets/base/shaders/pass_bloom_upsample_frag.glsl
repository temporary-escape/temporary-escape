#version 450
#extension GL_ARB_separate_shader_objects: enable

// Upsample algorithm based on:
// https://learnopengl.com/code_viewer_gh.php?code=src/8.guest/2022/6.physically_based_bloom/6.new_upsample.fs
// Modified for Vulkan
// Originally taken from Call Of Duty method, presented at ACM Siggraph 2014.

layout (location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout (push_constant) uniform Uniforms {
    float filterRadius;
} uniforms;

layout (binding = 0) uniform sampler2D texColor;

layout (location = 0) out vec4 outColor;


void main() {
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = uniforms.filterRadius;
    float y = uniforms.filterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(texColor, vec2(vs_out.texCoords.x - x, vs_out.texCoords.y + y)).rgb;
    vec3 b = texture(texColor, vec2(vs_out.texCoords.x, vs_out.texCoords.y + y)).rgb;
    vec3 c = texture(texColor, vec2(vs_out.texCoords.x + x, vs_out.texCoords.y + y)).rgb;

    vec3 d = texture(texColor, vec2(vs_out.texCoords.x - x, vs_out.texCoords.y)).rgb;
    vec3 e = texture(texColor, vec2(vs_out.texCoords.x, vs_out.texCoords.y)).rgb;
    vec3 f = texture(texColor, vec2(vs_out.texCoords.x + x, vs_out.texCoords.y)).rgb;

    vec3 g = texture(texColor, vec2(vs_out.texCoords.x - x, vs_out.texCoords.y - y)).rgb;
    vec3 h = texture(texColor, vec2(vs_out.texCoords.x, vs_out.texCoords.y - y)).rgb;
    vec3 i = texture(texColor, vec2(vs_out.texCoords.x + x, vs_out.texCoords.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    vec3 upsample = e * 4.0;
    upsample += (b + d + f + h) * 2.0;
    upsample += (a + c + g + i);
    upsample *= 1.0 / 16.0;

    outColor = vec4(upsample, 1.0);
}
