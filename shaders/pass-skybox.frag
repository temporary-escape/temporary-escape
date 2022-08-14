#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec3 texCoords;
} vs_out;

//layout(binding = 0) uniform samplerCube texBaseColor;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(0.3, 0.3, 0.3, 1.0);
    //outColor = vec4(texture(texBaseColor, vs_out.texCoords).rgb, 1.0);
}
