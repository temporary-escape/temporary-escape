#version 450
#extension GL_ARB_separate_shader_objects: enable

// Downsample algorithm based on:
// https://learnopengl.com/code_viewer_gh.php?code=src/8.guest/2022/6.physically_based_bloom/6.new_upsample.fs

layout (location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout (push_constant) uniform Uniforms {
    float bloomStrength;
    float exposure;
    float gamma;
} uniforms;

layout (binding = 0) uniform sampler2D texColor;
layout (binding = 1) uniform sampler2D texBloom;

layout (location = 0) out vec4 outColor;

// Source: https://github.com/dmnsgn/glsl-tone-map/blob/main/aces.glsl
vec3 aces(vec3 x) {
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
}

// Lottes 2016, "Advanced Techniques and Optimization of HDR Color Pipelines"
// Source: https://github.com/dmnsgn/glsl-tone-map/blob/main/lottes.glsl
vec3 lottes(vec3 x) {
    const vec3 a = vec3(1.6);
    const vec3 d = vec3(0.977);
    const vec3 hdrMax = vec3(8.0);
    const vec3 midIn = vec3(0.18);
    const vec3 midOut = vec3(0.267);

    const vec3 b =
    (-pow(midIn, a) + pow(hdrMax, a) * midOut) /
    ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);
    const vec3 c =
    (pow(hdrMax, a * d) * pow(midIn, a) - pow(hdrMax, a) * pow(midIn, a * d) * midOut) /
    ((pow(hdrMax, a * d) - pow(midIn, a * d)) * midOut);

    return pow(x, a) / (pow(x, a * d) * b + c);
}

void main() {
    vec3 color = texture(texColor, vs_out.texCoords).rgb;
    vec3 bloom = texture(texBloom, vs_out.texCoords).rgb;

    vec3 result = color + bloom * uniforms.bloomStrength;

    // tone mapping
    result = lottes(result);

    // also gamma correct while we're at it
    result = pow(result, vec3(1.0 / uniforms.gamma));

    outColor = vec4(result, 1.0);
}
