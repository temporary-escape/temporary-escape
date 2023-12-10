#version 450
layout(location = 0) in vec2 in_Position;
layout(location = 1) in vec4 in_TexCoords;
layout(location = 2) in vec4 in_Color;

layout(location = 0) out VS_OUT {
    vec4 color;
    vec4 texCoords;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

layout(push_constant) uniform Constants {
    mat4 mvp;
} constants;

void main() {
    vs_out.color = in_Color;
    vs_out.texCoords = in_TexCoords;
    gl_Position = constants.mvp * vec4(in_Position, 0.0, 1.0);
}
