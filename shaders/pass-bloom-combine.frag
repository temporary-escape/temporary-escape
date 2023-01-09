#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout(binding = 0) uniform sampler2D texForwardColor;
layout(binding = 1) uniform sampler2D texBloomColor;

layout(push_constant) uniform Uniforms {
    float bloomStrength;
    float bloomPower;
    float exposure;
    float gamma;
    float contrast;
} uniforms;

layout(location = 0) out vec4 outColor;

void main() {
    vec4 forwardColor = texture(texForwardColor, vs_out.texCoords);
    vec3 bloomColor = pow(texture(texBloomColor, vs_out.texCoords).rgb, vec3(uniforms.bloomPower));

    vec3 color = forwardColor.rgb + bloomColor.rgb * uniforms.bloomStrength;

    // tone mapping
    vec3 result = vec3(1.0) - exp(-color * uniforms.exposure);

    // also gamma correct while we're at it
    result = pow(result, vec3(1.0 / uniforms.gamma));

    // Contrast
    result = ((result.rgb - 0.5f) * max(uniforms.contrast, 0)) + 0.5f;

    outColor = vec4(result, 1.0);
}
