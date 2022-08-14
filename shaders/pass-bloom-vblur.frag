#define STEPSIZE_X 0.002f
#define RADIUS 10

void main() {
    float half_pixel_offset = STEPSIZE_Y * 0.5f;

    vec4 finalColor = vec4(0);
    for (int i = -RADIUS; i <= RADIUS; i += 2) {
        float y = texCoord.y + STEPSIZE_Y * i + half_pixel_offset;
        finalColor += texture2D(sourceTexture, vec2(texCoord.x, y)) * step(-0.5f * RADIUS * STEPSIZE_Y, y) * step(-1.0f - 0.5f * RADIUS * STEPSIZE_Y, -y);
    }
    outFragColor = finalColor / (RADIUS + 1);
}
