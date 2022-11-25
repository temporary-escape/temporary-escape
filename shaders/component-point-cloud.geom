#version 450
#extension GL_ARB_separate_shader_objects : enable

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

layout(location = 0) in VS_OUT {
    vec2 size;
    vec4 color;
} vs_out[];

layout(location = 0) out GS_OUT {
    vec4 color;
    vec2 texCoords;
} gs_out;

void main (void) {
    /*gs_out.color = vs_out[0].color;

    vec2 particleSize = vec2(2.0, 2.0);

    vec4 P = gl_in[0].gl_Position;

    // a: left-bottom
    vec2 va = P.xy + vec2(-0.5, -0.5) * particleSize;
    vec4 vap = camera.projectionMatrix * vec4(va, P.zw);
    gl_Position = vap;
    vap /= P.z;
    EmitVertex();

    // b: left-top
    vec2 vb = P.xy + vec2(-0.5, 0.5) * particleSize;
    vec4 vbp = camera.projectionMatrix * vec4(vb, P.zw);
    vbp /= P.z;
    gl_Position = vbp;
    EmitVertex();

    // d: right-bottom
    vec2 vd = P.xy + vec2(0.5, -0.5) * particleSize;
    vec4 vdp = camera.projectionMatrix * vec4(vd, P.zw);
    vdp /= P.z;
    gl_Position = vdp;
    EmitVertex();

    // c: right-top
    vec2 vc = P.xy + vec2(0.5, 0.5) * particleSize;
    vec4 vcp = camera.projectionMatrix * vec4(vc, P.zw);
    vcp /= P.z;
    gl_Position = vcp;
    EmitVertex();

    EndPrimitive();*/

    vec4 P = gl_in[0].gl_Position;

    gs_out.color = vs_out[0].color;
    // vec2 particleSize = vs_out[0].size;
    vec2 particleSize = vs_out[0].size / camera.viewport;

    // a: left-bottom
    vec2 va = P.xy + vec2(-1.0, -1.0) * particleSize;
    gl_Position = vec4(va, P.zw);
    gs_out.texCoords = vec2(0.0, 0.0);
    EmitVertex();

    // b: left-top
    vec2 vb = P.xy + vec2(-1.0, 1.0) * particleSize;
    gl_Position = vec4(vb, P.zw);
    gs_out.texCoords = vec2(0.0, 1.0);
    EmitVertex();

    // d: right-bottom
    vec2 vd = P.xy + vec2(1.0, -1.0) * particleSize;
    gl_Position = vec4(vd, P.zw);
    gs_out.texCoords = vec2(1.0, 0.0);
    EmitVertex();

    // c: right-top
    vec2 vc = P.xy + vec2(1.0, 1.0) * particleSize;
    gl_Position = vec4(vc, P.zw);
    gs_out.texCoords = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}
