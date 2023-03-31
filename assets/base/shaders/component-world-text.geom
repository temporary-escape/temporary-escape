#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout(location = 0) in VS_OUT {
    vec2 size;
    vec2 offset;
    vec2 uv;
    vec2 st;
} vs_out[];

layout(location = 0) out GS_OUT {
    vec2 texCoords;
} gs_out;

void main (void) {
    vec4 P = gl_in[0].gl_Position;
    vec2 uv = vs_out[0].uv;
    vec2 st = vs_out[0].st;

    // vec2 particleSize = vs_out[0].size;
    vec2 particleSize = vs_out[0].size / camera.viewport;
    vec2 offset = vs_out[0].offset / camera.viewport;

    // a: left-bottom
    vec2 va = P.xy + vec2(0.0, 0.0) * particleSize;
    gl_Position = vec4(va + offset, P.zw);
    gs_out.texCoords = uv;
    EmitVertex();

    // b: left-top
    vec2 vb = P.xy + vec2(0.0, 1.0) * particleSize;
    gl_Position = vec4(vb + offset, P.zw);
    gs_out.texCoords = vec2(uv.x, uv.y + st.y);
    EmitVertex();

    // d: right-bottom
    vec2 vd = P.xy + vec2(1.0, 0.0) * particleSize;
    gl_Position = vec4(vd + offset, P.zw);
    gs_out.texCoords = vec2(uv.x + st.x, uv.y);
    EmitVertex();

    // c: right-top
    vec2 vc = P.xy + vec2(1.0, 1.0) * particleSize;
    gl_Position = vec4(vc + offset, P.zw);
    gs_out.texCoords = vec2(uv.x + st.x, uv.y + st.y);
    EmitVertex();

    EndPrimitive();
}
