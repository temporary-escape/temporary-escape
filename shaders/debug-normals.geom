layout (points) in;
layout (line_strip) out;
layout (max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
    vec3 tangent;
    vec3 bitangent;
} vsOut[];

out GS_OUT {
    vec4 color;
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
    const float scale = 0.1;
    vec3 pos = gl_in[0].gl_Position.xyz;

    /*gl_Position = camera.transformationProjectionMatrix * vec4(pos, 1.0);
    gsOut.color = vec4(0.0, 0.0, 1.0, 0.0);
    EmitVertex();

    gl_Position = camera.transformationProjectionMatrix * vec4(pos + vsOut[0].normal * scale, 1.0);
    gsOut.color = vec4(0.0, 0.0, 1.0, 0.0);
    EmitVertex();*/

    gl_Position = camera.transformationProjectionMatrix * vec4(pos + vsOut[0].normal * 0.05, 1.0);
    gsOut.color = vec4(0.0, 1.0, 0.0, 1.0);
    EmitVertex();

    gl_Position = camera.transformationProjectionMatrix * vec4(pos + vsOut[0].normal * 0.05 + vsOut[0].normal * scale, 1.0);
    gsOut.color = vec4(0.0, 1.0, 0.0, 1.0);
    EmitVertex();

    /*gl_Position = camera.transformationProjectionMatrix * vec4(pos, 1.0);
    gsOut.color = vec4(1.0, 0.0, 0.0, 0.0);
    EmitVertex();

    gl_Position = camera.transformationProjectionMatrix * vec4(pos + vsOut[0].bitangent * scale, 1.0);
    gsOut.color = vec4(1.0, 0.0, 0.0, 0.0);
    EmitVertex();*/
}
