#version 450
#extension GL_ARB_separate_shader_objects: enable

// Downsample algorithm based on:
// https://learnopengl.com/code_viewer_gh.php?code=src/8.guest/2022/6.physically_based_bloom/6.new_downsample.fs
// Modified for Vulkan
// Originally taken from Call Of Duty method, presented at ACM Siggraph 2014.

layout (location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout (push_constant) uniform Uniforms {
    vec2 texColorSize;
    int mipLevel;
} uniforms;

layout (binding = 0) uniform sampler2D texColor;

layout (location = 0) out vec4 outColor;

vec3 PowVec3(vec3 v, float p)
{
    return vec3(pow(v.x, p), pow(v.y, p), pow(v.z, p));
}

const float invGamma = 1.0 / 2.2;
vec3 ToSRGB(vec3 v) { return PowVec3(v, invGamma); }

float sRGBToLuma(vec3 col)
{
    //return dot(col, vec3(0.2126f, 0.7152f, 0.0722f));
    return dot(col, vec3(0.299f, 0.587f, 0.114f));
}

float KarisAverage(vec3 col)
{
    // Formula is 1 / (1 + luma)
    float luma = sRGBToLuma(ToSRGB(col)) * 0.25f;
    return 1.0f / (1.0f + luma);
}


void main() {
    vec2 srcTexelSize = 1.0 / uniforms.texColorSize;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(texColor, vec2(vs_out.texCoords.x - 2 * x, vs_out.texCoords.y + 2 * y)).rgb;
    vec3 b = texture(texColor, vec2(vs_out.texCoords.x, vs_out.texCoords.y + 2 * y)).rgb;
    vec3 c = texture(texColor, vec2(vs_out.texCoords.x + 2 * x, vs_out.texCoords.y + 2 * y)).rgb;

    vec3 d = texture(texColor, vec2(vs_out.texCoords.x - 2 * x, vs_out.texCoords.y)).rgb;
    vec3 e = texture(texColor, vec2(vs_out.texCoords.x, vs_out.texCoords.y)).rgb;
    vec3 f = texture(texColor, vec2(vs_out.texCoords.x + 2 * x, vs_out.texCoords.y)).rgb;

    vec3 g = texture(texColor, vec2(vs_out.texCoords.x - 2 * x, vs_out.texCoords.y - 2 * y)).rgb;
    vec3 h = texture(texColor, vec2(vs_out.texCoords.x, vs_out.texCoords.y - 2 * y)).rgb;
    vec3 i = texture(texColor, vec2(vs_out.texCoords.x + 2 * x, vs_out.texCoords.y - 2 * y)).rgb;

    vec3 j = texture(texColor, vec2(vs_out.texCoords.x - x, vs_out.texCoords.y + y)).rgb;
    vec3 k = texture(texColor, vec2(vs_out.texCoords.x + x, vs_out.texCoords.y + y)).rgb;
    vec3 l = texture(texColor, vec2(vs_out.texCoords.x - x, vs_out.texCoords.y - y)).rgb;
    vec3 m = texture(texColor, vec2(vs_out.texCoords.x + x, vs_out.texCoords.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving downsample we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1

    // Check if we need to perform Karis average on each block of 4 samples
    vec3 downsample = vec3(0.0, 0.0, 0.0);
    vec3 groups[5];
    if (uniforms.mipLevel == 0) {
        // We are writing to mip 0, so we need to apply Karis average to each block
        // of 4 samples to prevent fireflies (very bright subpixels, leads to pulsating
        // artifacts).
        groups[0] = vec3(a + b + d + e) * vec3(0.125f / 4.0f);
        groups[1] = vec3(b + c + e + f) * vec3(0.125f / 4.0f);
        groups[2] = vec3(d + e + g + h) * vec3(0.125f / 4.0f);
        groups[3] = vec3(e + f + h + i) * vec3(0.125f / 4.0f);
        groups[4] = vec3(j + k + l + m) * vec3(0.5f / 4.0f);
        groups[0] *= KarisAverage(groups[0]);
        groups[1] *= KarisAverage(groups[1]);
        groups[2] *= KarisAverage(groups[2]);
        groups[3] *= KarisAverage(groups[3]);
        groups[4] *= KarisAverage(groups[4]);
        downsample = groups[0] + groups[1] + groups[2] + groups[3] + groups[4];
        downsample = max(downsample, 0.0001f);
    } else {
        downsample = e * 0.125;                // ok
        downsample += (a + c + g + i) * 0.03125;     // ok
        downsample += (b + d + f + h) * 0.0625;      // ok
        downsample += (j + k + l + m) * 0.125;       // ok
    }

    outColor = vec4(downsample, 1.0);
}
