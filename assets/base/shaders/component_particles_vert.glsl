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

layout (push_constant) uniform Uniforms {
    mat4 modelMatrix;
    float timeDelta;
} uniforms;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec3 in_Direction;
layout (location = 2) in vec4 in_StartColor;
layout (location = 3) in vec4 in_EndColor;

layout (location = 0) out VS_OUT {
    vec2 texCoords;
    vec4 color;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

const int verticesPerFace = 6;

void geometrize(vec4 pos, float time) {
    vs_out.color = mix(in_StartColor, in_EndColor, time);
    vec2 particleSize = vec2(0.2, 0.2);
    particleSize.x /= float(camera.viewport.x) / float(camera.viewport.y);

    // a: left-bottom
    if (gl_VertexIndex % 6 == 0) {
        vec2 va = pos.xy + vec2(-1.0, -1.0) * particleSize;
        gl_Position = vec4(va, pos.zw);
        vs_out.texCoords = vec2(0.0, 0.0);
    }

    // b: left-top
    if (gl_VertexIndex % 6 == 1 || gl_VertexIndex % 6 == 4) {
        vec2 vb = pos.xy + vec2(-1.0, 1.0) * particleSize;
        gl_Position = vec4(vb, pos.zw);
        vs_out.texCoords = vec2(0.0, 1.0);
    }

    // d: right-bottom
    if (gl_VertexIndex % 6 == 2 || gl_VertexIndex % 6 == 3) {
        vec2 vd = pos.xy + vec2(1.0, -1.0) * particleSize;
        gl_Position = vec4(vd, pos.zw);
        vs_out.texCoords = vec2(1.0, 0.0);
    }

    // c: right-top
    if (gl_VertexIndex % 6 == 5) {
        vec2 vc = pos.xy + vec2(1.0, 1.0) * particleSize;
        gl_Position = vec4(vc, pos.zw);
        vs_out.texCoords = vec2(1.0, 1.0);
    }
}

const int count = 10;
const float duration = 5.0;
const float distance = 2.0;

void main() {
    float step = duration / float(count);
    int index = gl_VertexIndex / 6;

    float time = mod(uniforms.timeDelta + step * index, duration) * (1.0 / duration);

    vec3 modelPos = in_Position + in_Direction * (time * distance);
    // modelPos.x += index * 0.5;
    // vec3 modelPos = in_Position + in_Direction * index;

    vec4 worldPos = uniforms.modelMatrix * vec4(modelPos, 1.0);
    geometrize(camera.transformationProjectionMatrix * worldPos, time);
}
