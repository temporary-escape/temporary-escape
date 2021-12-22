layout(location = 0) in vec3 position;

out VS_OUT {
    vec3 texCoords;
} vs_out;

uniform Camera {
    mat4 transformationProjectionMatrix;
} camera;

uniform mat4 modelMatrix;

void main() {
    vs_out.texCoords = position;
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
