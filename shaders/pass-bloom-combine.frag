void main() {
    // gamma = 1.0
    // amplification = 1.0

    vec4 texColor = texture2D(baseTexture, vec2(texCoord.x, texCoord.y));
    vec4 bloom = texture2D(bloomTexture, vec2(texCoord.x, texCoord.y));

    outFragColor = texColor + bloom * amplification;
    outFragColor.rgb = pow(outFragColor.rgb, vec3(1.0 / gamma));
}
