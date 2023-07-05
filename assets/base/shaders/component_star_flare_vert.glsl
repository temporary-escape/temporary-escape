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

layout (location = 0) out VS_OUT {
    vec2 coords;
} vs_out;

layout (push_constant) uniform Uniforms {
    mat4 modelMatrix;
    vec2 size;
    float temp;
} uniforms;

/*void geometrize(vec4 pos) {
    vec2 particleSize = uniforms.size / camera.viewport;

    // a: left-bottom
    if (gl_VertexIndex == 0) {
        vec2 va = pos.xy + vec2(-1.0, -1.0) * particleSize;
        gl_Position = vec4(va, pos.zw);
        vs_out.coords = vec2(0.0, 0.0);
    }

    // b: left-top
    if (gl_VertexIndex == 1) {
        vec2 vb = pos.xy + vec2(-1.0, 1.0) * particleSize;
        gl_Position = vec4(vb, pos.zw);
        vs_out.coords = vec2(0.0, 1.0);
    }

    // d: right-bottom
    if (gl_VertexIndex == 2) {
        vec2 vd = pos.xy + vec2(1.0, -1.0) * particleSize;
        gl_Position = vec4(vd, pos.zw);
        vs_out.coords = vec2(1.0, 0.0);
    }

    // c: right-top
    if (gl_VertexIndex == 3) {
        vec2 vc = pos.xy + vec2(1.0, 1.0) * particleSize;
        gl_Position = vec4(vc, pos.zw);
        vs_out.coords = vec2(1.0, 1.0);
    }
}

void main() {
    vec4 worldPos = uniforms.modelMatrix * vec4(0.0, 0.0, 0.0, 1.0);
    geometrize(camera.transformationProjectionMatrix * worldPos);
}*/

void geometrize(vec4 pos) {
    float dist = -pos.z;

    // a: left-bottom
    if (gl_VertexIndex == 0) {
        vec2 va = pos.xy + vec2(-1.0, -1.0) * uniforms.size * dist;
        gl_Position = camera.projectionMatrix * vec4(va, pos.zw);
        vs_out.coords = vec2(0.0, 0.0);
    }

    // b: left-top
    if (gl_VertexIndex == 1) {
        vec2 vb = pos.xy + vec2(-1.0, 1.0) * uniforms.size * dist;
        gl_Position = camera.projectionMatrix * vec4(vb, pos.zw);
        vs_out.coords = vec2(0.0, 1.0);
    }

    // d: right-bottom
    if (gl_VertexIndex == 2) {
        vec2 vd = pos.xy + vec2(1.0, -1.0) * uniforms.size * dist;
        gl_Position = camera.projectionMatrix * vec4(vd, pos.zw);
        vs_out.coords = vec2(1.0, 0.0);
    }

    // c: right-top
    if (gl_VertexIndex == 3) {
        vec2 vc = pos.xy + vec2(1.0, 1.0) * uniforms.size * dist;
        gl_Position = camera.projectionMatrix * vec4(vc, pos.zw);
        vs_out.coords = vec2(1.0, 1.0);
    }
}

void main() {
    vec4 worldPos = uniforms.modelMatrix * vec4(0.0, 0.0, 0.0, 1.0);
    geometrize(camera.viewMatrix * worldPos);
}
