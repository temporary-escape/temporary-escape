#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec2 texCoords;
    vec4 color;
} vs_out;

layout (std140, binding = 2) uniform ParticlesBatch {
    mat4 modelMatrix;
    int type;
    float timeDelta;
    float strength;
    float alpha;
} batch;

layout (binding = 3) uniform sampler2D colorTexture;

layout (location = 0) out vec4 outColor;

void main() {
    vec4 color = texture(colorTexture, vs_out.texCoords);
    outColor = pow(color, vec4(2.2)) * pow(vs_out.color, vec4(2.2)) * vec4(1.0, 1.0, 1.0, batch.alpha);
}
