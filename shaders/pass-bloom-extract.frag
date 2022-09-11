#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout(binding = 0) uniform sampler2D texSourceColor;

layout(location = 0) out vec4 outColor;

const float threshold = 0.5f;

void main() {
    vec4 sourceColor = texture(texSourceColor, vs_out.texCoords);
    outColor = vec4(max(vec3(0.0), (sourceColor.rgb - vec3(threshold)) * 2.0f), sourceColor.a);
}
