layout (location = 0) in vec3 vsIn_position;
layout (location = 1) in vec3 vsIn_direction;
layout (location = 2) in vec4 vsIn_color;
layout (location = 3) in float vsIn_time;
layout (location = 4) in float vsIn_speed;

layout (std140) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

void main() {
    gl_PointSize = 2.0;
    gl_Position = camera.transformationProjectionMatrix * vec4(vsIn_position, 1.0);
}
