#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout(binding = 0) uniform sampler2D texForwardColor;
layout(binding = 1) uniform sampler2D texBloomColor;

layout(push_constant) uniform Uniforms {
    float strength;
    float exposure;
} uniforms;

layout(location = 0) out vec4 outColor;

const float gamma = 2.2;

void main() {
    vec4 forwardColor = texture(texForwardColor, vs_out.texCoords);
    vec4 bloomColor = texture(texBloomColor, vs_out.texCoords);

    vec3 color = forwardColor.rgb + bloomColor.rgb * uniforms.strength;

    // tone mapping
    vec3 result = vec3(1.0) - exp(-color * uniforms.exposure);
    // also gamma correct while we're at it
    result = pow(result, vec3(1.0 / gamma));

    outColor = vec4(result, 1.0);
}
