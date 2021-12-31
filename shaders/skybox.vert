layout(location = 0) in vec3 position;

out VS_OUT {
    vec3 texCoords;
} vsOut;

uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

uniform mat4 modelMatrix;

void main() {
    vsOut.texCoords = position;
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
