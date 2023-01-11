#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec3 texCoords;
} vs_out;

layout(binding = 1) uniform samplerCube texBaseColor;

layout(location = 0) out vec4 outColor;

void main() {
    vec3 color = texture(texBaseColor, vs_out.texCoords).rgb;
    outColor = vec4(pow(color, vec3(2.2)), 1.0);
}
