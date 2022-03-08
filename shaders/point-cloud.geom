layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in VS_OUT {
    vec4 color;
    vec2 size;
} vsOut[];

out GS_OUT {
    vec4 color;
    vec2 texCoords;
} gsOut;

layout (std140) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

void main (void) {
    vec4 P = gl_in[0].gl_Position;

    gsOut.color = vsOut[0].color;
    vec2 particleSize = vsOut[0].size;

    // a: left-bottom 
    vec2 va = P.xy + vec2(-1.0, -1.0) * particleSize;
    gl_Position = camera.projectionMatrix * vec4(va, P.zw);
    gsOut.texCoords = vec2(0.0, 0.0);
    EmitVertex();

    // b: left-top
    vec2 vb = P.xy + vec2(-1.0, 1.0) * particleSize;
    gl_Position = camera.projectionMatrix * vec4(vb, P.zw);
    gsOut.texCoords = vec2(0.0, 1.0);
    EmitVertex();

    // d: right-bottom
    vec2 vd = P.xy + vec2(1.0, -1.0) * particleSize;
    gl_Position = camera.projectionMatrix * vec4(vd, P.zw);
    gsOut.texCoords = vec2(1.0, 0.0);
    EmitVertex();

    // c: right-top
    vec2 vc = P.xy + vec2(1.0, 1.0) * particleSize;
    gl_Position = camera.projectionMatrix * vec4(vc, P.zw);
    gsOut.texCoords = vec2(1.0, 1.0);
    EmitVertex();

    EndPrimitive();
}   
