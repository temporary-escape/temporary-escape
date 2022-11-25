#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec3 texCoords;
} vs_out;

layout(binding = 1) uniform samplerCube texBaseColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(pow(texture(texBaseColor, vs_out.texCoords).rgb, vec3(2.2)), 1.0);
    // outColor = vec4(vs_out.texCoords, 1.0);
}
