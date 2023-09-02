#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Origin;
layout(location = 2) in vec3 in_Direction;
layout(location = 3) in float in_Lifetime;
layout(location = 4) in float in_Size;
layout(location = 5) in float in_Speed;

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

out gl_PerVertex {
    vec4 gl_Position;
};

// Source: https://forum.derivative.ca/t/look-at-vertex-shader-for-instancing-glsl/299286
mat4 lookAtPoint(vec3 eye, vec3 at, float size) {
    vec3 localUp = vec3(0, 1, 0);
    vec3 fwd = normalize(at - eye);
    vec3 right = normalize(cross(localUp, fwd));
    vec3 up = normalize(cross(fwd, right));
    return mat4(vec4(right * size, 0.0), vec4(up * size, 0.0), vec4(fwd * size, 0.0), vec4(eye, 1.0));
}

void main() {
    mat4 lookAtMat = lookAtPoint(in_Origin, in_Origin + in_Direction, in_Size);
    vec3 offset = vec3(0.0, 0.0, 0.5);
    gl_Position = camera.transformationProjectionMatrix * lookAtMat * vec4(in_Position + offset, 1.0);
}
