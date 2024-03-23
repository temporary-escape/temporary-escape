#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec4 color;
    vec2 texCoords;
} vs_out;

layout (location = 0) out vec4 outColor;

const vec2 mid = vec2(0.5, 0.5);

void main() {
    float dist = distance(vs_out.texCoords, mid) * 2.0;
    float alpha = clamp(1.0 - dist, 0.0, 1.0);
    outColor = vs_out.color * vec4(1.0, 1.0, 1.0, alpha);
}
