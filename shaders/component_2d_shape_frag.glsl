#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform Uniforms {
    mat4 modelMatrix;
    vec4 color;
} uniforms;

layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(pow(uniforms.color.rgb, vec3(2.2)), uniforms.color.a);
}
