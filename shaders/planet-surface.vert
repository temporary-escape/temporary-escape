layout(location = 0) in vec3 vsIn_position;
layout(location = 1) in vec3 vsIn_normal;

out VS_OUT {
    vec3 normal;
    vec3 texCoords;
    vec3 worldPos;
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
    vsOut.texCoords = normalize(vsIn_position);

    vec4 worldPos = modelMatrix * vec4(vsIn_position.xyz, 1.0);
    vec3 N = normalize(normalMatrix * vsIn_normal);

    vsOut.worldPos = worldPos.xyz;
    vsOut.normal = N.xyz;

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
