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
} uniforms;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_Size;
layout (location = 2) in vec4 in_Color;
layout (location = 3) in vec2 in_Uv;
layout (location = 4) in vec2 in_St;
layout (location = 5) in vec2 in_Offset;

layout (location = 0) out VS_OUT {
    vec4 color;
    vec2 texCoords;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void geometrize(vec4 pos) {
    vs_out.color = in_Color;
    vec2 particleSize = in_Size;
    vec2 offset = in_Offset / camera.viewport;
    particleSize.x /= float(camera.viewport.x) / float(camera.viewport.y);

    // a: left-bottom
    if (gl_VertexIndex == 0) {
        vec2 va = pos.xy + vec2(-1.0, -1.0) * particleSize + offset;
        gl_Position = vec4(va, pos.zw);
        vs_out.texCoords = in_Uv;
    }

    // b: left-top
    if (gl_VertexIndex == 1) {
        vec2 vb = pos.xy + vec2(-1.0, 1.0) * particleSize + offset;
        gl_Position = vec4(vb, pos.zw);
        vs_out.texCoords = vec2(in_Uv.x, in_Uv.y + in_St.y);
    }

    // d: right-bottom
    if (gl_VertexIndex == 2) {
        vec2 vd = pos.xy + vec2(1.0, -1.0) * particleSize + offset;
        gl_Position = vec4(vd, pos.zw);
        vs_out.texCoords = vec2(in_Uv.x + in_St.x, in_Uv.y);
    }

    // c: right-top
    if (gl_VertexIndex == 3) {
        vec2 vc = pos.xy + vec2(1.0, 1.0) * particleSize + offset;
        gl_Position = vec4(vc, pos.zw);
        vs_out.texCoords = vec2(in_Uv.x + in_St.x, in_Uv.y + in_St.y);
    }
}

void main() {
    vec4 worldPos = uniforms.modelMatrix * vec4(in_Position, 1.0);
    geometrize(camera.transformationProjectionMatrix * worldPos);
}
