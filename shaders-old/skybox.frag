in VS_OUT {
    vec3 texCoords;
} vsOut;

layout(location = 0) out vec4 fragmentColor;

uniform samplerCube skyboxTexture;

void main() {
    vec3 emissive = texture(skyboxTexture, vsOut.texCoords).rgb;
    fragmentColor = vec4(emissive, 1.0);
}
