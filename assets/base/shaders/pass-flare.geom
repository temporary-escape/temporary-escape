#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout (std140, binding = 0) uniform CameraMatrices {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout (push_constant) uniform Uniforms {
    vec2 size;
    float temp;
} uniforms;

layout (location = 0) out GS_OUT {
    vec2 coords;
} gs_out;

void main() {
    vec4 P = gl_in[0].gl_Position;
    float dist = -P.z;

    // a: left-bottom
    vec2 va = P.xy + vec2(-1.0, -1.0) * uniforms.size * dist;
    gl_Position = camera.projectionMatrix * vec4(va, P.zw);
    gs_out.coords = vec2(-1.0, -1.0);
    EmitVertex();

    // b: left-top
    vec2 vb = P.xy + vec2(-1.0, 1.0) * uniforms.size * dist;
    gl_Position = camera.projectionMatrix * vec4(vb, P.zw);
    gs_out.coords = vec2(-1.0, 1.0);
    EmitVertex();

    // d: right-bottom
    vec2 vd = P.xy + vec2(1.0, -1.0) * uniforms.size * dist;
    gl_Position = camera.projectionMatrix * vec4(vd, P.zw);
    gs_out.coords = vec2(1.0, -1.0);
    EmitVertex();

    // c: right-top
    vec2 vc = P.xy + vec2(1.0, 1.0) * uniforms.size * dist;
    gl_Position = camera.projectionMatrix * vec4(vc, P.zw);
    gs_out.coords = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}
