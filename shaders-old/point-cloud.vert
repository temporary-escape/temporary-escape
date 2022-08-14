layout(location = 0) in vec3 vsIn_position;
layout(location = 1) in vec2 vsIn_size;
layout(location = 2) in vec4 vsIn_color;

out VS_OUT {
    vec4 color;
    vec2 size;
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
    vec4 worldPos = modelMatrix * vec4(vsIn_position.xyz, 1.0);
    vsOut.color = vsIn_color;
    vsOut.size = vsIn_size;

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
