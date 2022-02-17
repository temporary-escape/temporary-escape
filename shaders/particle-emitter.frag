layout (location = 0) out vec4 fragColor;

in GS_OUT {
    vec2 texCoords;
    float time;
} gsOut;

layout(std140) uniform ParticleData {
    vec4 startColor;
    vec4 endColor;
    vec3 offset;
    vec3 force;
    float startRadius;
    float endRadius;
    int count;
    float duration;
    float startSize;
    float endSize;
} data;

uniform sampler2D particleTexture;

void main() {
    // vec4(0.3, 0.4, 1.0, 0.6)
    vec4 baseColor = texture(particleTexture, gsOut.texCoords);
    vec4 color = baseColor * mix(data.startColor, data.endColor, gsOut.time);
    fragColor = vec4(color.rgb, color.a * pow(1.0 - gsOut.time, 2.2));
}
