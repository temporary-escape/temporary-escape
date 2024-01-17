#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec2 texCoords;
    float alpha;
} vs_out;

layout (binding = 1) uniform sampler2D colorTexture;

layout (location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(colorTexture, vs_out.texCoords);
    outColor = color * vec4(1.0, 1.0, 1.0, vs_out.alpha);
}
