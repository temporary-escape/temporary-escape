#version 450
#extension GL_ARB_separate_shader_objects : enable

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

void main() {
    vec3 position;
    if (gl_VertexIndex % 2 == 0) {
        position = in_Origin - in_Direction * in_Size * 0.0;
    } else {
        position = in_Origin + in_Direction * in_Size * 1.0;
    }

    gl_Position = camera.transformationProjectionMatrix * vec4(position, 1.0);
}
