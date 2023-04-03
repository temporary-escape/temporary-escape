#version 450
#extension GL_ARB_separate_shader_objects: enable

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

layout (push_constant) uniform Uniforms {
    mat4 modelMatrix;
    vec2 size;
    float temp;
} uniforms;

void main() {
    vec4 worldPos = uniforms.modelMatrix * vec4(0.0, 0.0, 0.0, 1.0);
    gl_Position = camera.viewMatrix * worldPos;
}
