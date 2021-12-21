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

uniform Camera {
    mat4 transformationProjectionMatrix;
} camera;

uniform mat3 normalMatrix;
uniform mat4 modelMatrix;

void main() {
    vsOut.texCoords = vsIn_texCoords;

    vec4 worldPos = modelMatrix * vec4(vsIn_position.xyz, 1.0);

    vec3 N = normalize(normalMatrix * vsIn_normal);
    vec3 T = normalize(normalMatrix * vsIn_tangent.xyz);
    vec3 B = cross(N, T);

    vsOut.TBN = mat3(T, B, N);
    vsOut.normal = N.xyz;
    vsOut.worldpos = worldPos.xyz;

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
