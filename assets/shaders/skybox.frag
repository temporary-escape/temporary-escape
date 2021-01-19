in vec3 v_texCoords;

out vec4 fragmentColor;

uniform samplerCube skyboxTexture;

void main() {
    vec3 emissive = texture(skyboxTexture, v_texCoords).rgb;
    fragmentColor = vec4(emissive, 1.0);
}
