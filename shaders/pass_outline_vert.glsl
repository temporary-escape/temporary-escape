#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in vec2 in_Position;

layout (location = 0) out VS_OUT {
    vec2 texCoords;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vs_out.texCoords = in_Position * 0.5 + 0.5;
    gl_Position = vec4(in_Position, 0.0, 1.0);
}
