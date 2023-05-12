#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Position;

layout(push_constant) uniform Uniforms {
    mat4 projectionViewMatrix;
} uniforms;

layout(location = 0) out VS_OUT {
    vec3 texcoords;
} vs_out;

void main() {
    vs_out.texcoords = in_Position;
    vec4 pos = uniforms.projectionViewMatrix * vec4(in_Position, 1.0);
    gl_Position = pos.xyww;
}
