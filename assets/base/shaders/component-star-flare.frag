#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in GS_OUT {
    vec2 coords;
} gs_out;

layout (push_constant) uniform Uniforms {
    mat4 modelMatrix;
    vec2 size;
    float temp;
} uniforms;

layout(binding = 1) uniform sampler2D colorTexture;
layout(binding = 2) uniform sampler1D spectrumLowTexture;
layout(binding = 3) uniform sampler1D spectrumHighTexture;

layout (location = 0) out vec4 outColor;

void main() {
    vec4 baseColor = texture(colorTexture, gs_out.coords);

    vec3 colorLow = texture(spectrumLowTexture, uniforms.temp).rgb;
    vec3 colorHigh = texture(spectrumHighTexture, uniforms.temp).rgb;

    vec4 color = vec4(mix(colorLow, colorHigh, pow(baseColor.a, 2.0)), baseColor.a);

    outColor = pow(color, vec4(2.2));
}
