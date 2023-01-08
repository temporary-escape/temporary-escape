#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout (std140, binding = 0) uniform CameraUniform {
    mat4 viewMatrix;
    mat4 projectionMatrix;
} camera;

layout(push_constant) uniform Uniforms {
    vec2 particleSize;
} uniforms;

layout(location = 0) in VS_OUT {
    float brightness;
    vec4 color;
} vs_out[];

layout(location = 0) out GS_OUT {
    vec2 coords;
    float brightness;
    vec4 color;
} gs_out;

void main() {
    vec4 P = gl_in[0].gl_Position;

    gs_out.brightness = vs_out[0].brightness;
    gs_out.color = vs_out[0].color;

    // a: left-bottom
    vec2 va = P.xy + vec2(-1.0, -1.0) * uniforms.particleSize;
    gl_Position = camera.projectionMatrix * vec4(va, P.zw);
    gs_out.coords = vec2(-1.0, -1.0);
    EmitVertex();

    // b: left-top
    vec2 vb = P.xy + vec2(-1.0, 1.0) * uniforms.particleSize;
    gl_Position = camera.projectionMatrix * vec4(vb, P.zw);
    gs_out.coords = vec2(-1.0, 1.0);
    EmitVertex();

    // d: right-bottom
    vec2 vd = P.xy + vec2(1.0, -1.0) * uniforms.particleSize;
    gl_Position = camera.projectionMatrix * vec4(vd, P.zw);
    gs_out.coords = vec2(1.0, -1.0);
    EmitVertex();

    // c: right-top
    vec2 vc = P.xy + vec2(1.0, 1.0) * uniforms.particleSize;
    gl_Position = camera.projectionMatrix * vec4(vc, P.zw);
    gs_out.coords = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}
