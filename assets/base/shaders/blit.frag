#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D colorTexture;

void main() {
    outColor = texture(colorTexture, vs_out.texCoords);
}
