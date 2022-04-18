layout (points) in;
layout (triangle_strip) out;
layout (max_vertices = 4) out;

in VS_OUT {
    float time;
} vsOut[];

out GS_OUT {
    vec2 texCoords;
    float time;
} gsOut;

layout (std140) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout(std140) uniform ParticleData {
    vec4 startColor;
    vec4 endColor;
    vec3 offset;
    vec3 force;
    float startRadius;
    float endRadius;
    int count;
    float duration;
    float startSize;
    float endSize;
} data;

uniform mat4 modelViewMatrix;

void main (void) {
    float size = mix(data.startSize, data.endSize, vsOut[0].time) / 2.0;

    vec3 right = vec3(
    modelViewMatrix[0][0],
    modelViewMatrix[1][0],
    modelViewMatrix[2][0]
    );

    vec3 up = vec3(
    modelViewMatrix[0][1],
    modelViewMatrix[1][1],
    modelViewMatrix[2][1]
    );

    vec3 P = gl_in[0].gl_Position.xyz;

    vec3 va = P + (-right - up) * size;
    gl_Position = camera.transformationProjectionMatrix * vec4(va, 1.0);
    gsOut.texCoords = vec2(0.0, 0.0);
    gsOut.time = vsOut[0].time;
    EmitVertex();

    vec3 vb = P + (-right + up) * size;
    gl_Position = camera.transformationProjectionMatrix * vec4(vb, 1.0);
    gsOut.texCoords = vec2(0.0, 1.0);
    gsOut.time = vsOut[0].time;
    EmitVertex();

    vec3 vd = P + (right - up) * size;
    gl_Position = camera.transformationProjectionMatrix * vec4(vd, 1.0);
    gsOut.texCoords = vec2(1.0, 0.0);
    gsOut.time = vsOut[0].time;
    EmitVertex();

    vec3 vc = P + (right + up) * size;
    gl_Position = camera.transformationProjectionMatrix * vec4(vc, 1.0);
    gsOut.texCoords = vec2(1.0, 1.0);
    gsOut.time = vsOut[0].time;
    EmitVertex();

    EndPrimitive();
}

