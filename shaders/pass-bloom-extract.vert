void main() {
    vec4 texColor = texture2D(sourceTexture, vec2(texCoord.x, texCoord.y));
    outFragColor = max(vec4(0.0), (texColor - threshold) * 2.0f);
}
