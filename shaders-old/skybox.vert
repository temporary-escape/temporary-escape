layout(location = 0) in vec3 position;

out VS_OUT {
    vec3 texCoords;
} vsOut;

layout (std140) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

uniform mat4 modelMatrix;

void main() {
    vsOut.texCoords = position;
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
