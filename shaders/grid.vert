layout(location = 0) in vec3 vsIn_position;
layout(location = 1) in vec3 vsIn_normal;
layout(location = 2) in vec2 vsIn_texCoords;
layout(location = 3) in vec4 vsIn_tangent;
layout(location = 4) in mat4 vsIn_instances;

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

uniform mat4 modelMatrix;

void main() {
    vsOut.texCoords = vsIn_texCoords;

    mat4 model = modelMatrix * vsIn_instances;
    vec4 worldPos = model * vec4(vsIn_position.xyz, 1.0);

    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vec3 N = normalize(normalMatrix * vsIn_normal);
    vec3 T = normalize(normalMatrix * vsIn_tangent.xyz);
    vec3 B = cross(N, T);

    vsOut.TBN = mat3(T, B, N);
    vsOut.normal = N.xyz;
    vsOut.worldpos = worldPos.xyz;

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
