layout(location = 0) in vec3 vsIn_position;
layout(location = 1) in vec3 vsIn_normal;
layout(location = 2) in vec2 vsIn_texCoords;
layout(location = 3) in vec4 vsIn_tangent;

out VS_OUT {
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec3 worldpos;
} vsOut;

layout (std140) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

uniform mat3 normalMatrix;
uniform mat4 modelMatrix;

void main() {
    vsOut.texCoords = vsIn_texCoords;

    vec4 worldPos = modelMatrix * vec4(vsIn_position.xyz, 1.0);

    vec3 N = normalize(vec3(modelMatrix * vec4(vsIn_normal, 0.0)));
    vec3 T = normalize(vec3(modelMatrix * vec4(vsIn_tangent.xyz, 0.0)));
    vec3 B = cross(N, T);

    vsOut.TBN = mat3(T, B, N);
    vsOut.normal = N.xyz;
    vsOut.worldpos = worldPos.xyz;

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}