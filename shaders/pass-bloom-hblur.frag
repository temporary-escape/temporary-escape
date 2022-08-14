#define STEPSIZE_X 0.002f
#define RADIUS 10

void main() {
    float half_pixel_offset = STEPSIZE_X * 0.5f;

    vec4 finalColor = vec4(0);
    for (int i = -RADIUS; i <= RADIUS; i += 2) {
        float x = texCoord.x + STEPSIZE_X * i + half_pixel_offset;
        finalColor += texture2D(sourceTexture, vec2(x, texCoord.y)) * step(-0.5f * RADIUS * STEPSIZE_X, x) * step(-1.0f - 0.5f * RADIUS * STEPSIZE_X, -x);
    }
    outFragColor = finalColor / (RADIUS + 1);
}
