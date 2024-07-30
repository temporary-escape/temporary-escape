#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec2 texCoords;
    vec4 color;
} vs_out;

layout (std140, set = 2, binding = 0) uniform ParticlesBatch {
    mat4 modelMatrix;
    float timeDelta;
    float strength;
    float alpha;
} batch;

layout (set = 1, binding = 1) uniform sampler2D colorTexture;

layout (location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(colorTexture, vs_out.texCoords);
    outColor = pow(color, vec4(2.2)) * pow(vs_out.color, vec4(2.2)) * vec4(1.0, 1.0, 1.0, batch.alpha);
}
