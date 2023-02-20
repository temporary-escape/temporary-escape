#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 0) uniform CameraMatrices {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout(push_constant) uniform Uniforms {
    mat4 modelMatrix;
    vec4 color;
} uniforms;

layout(location = 0) in vec3 in_Position;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vec4 worldPos = uniforms.modelMatrix * vec4(in_Position, 1.0);
    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
