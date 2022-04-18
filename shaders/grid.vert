layout(location = 0) in vec3 vsIn_position;
layout(location = 1) in vec3 vsIn_normal;
layout(location = 2) in vec4 vsIn_tangent;

out VS_OUT {
    vec3 normal;
    vec2 texCoords;
//mat3 TBN;
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

vec2 boxProjection(vec3 normal, vec3 position) {
    const float uvScale = 0.25;
    const float offset = 0.5;

    vec3 absnorm = abs(normal);
    vec2 texCoords = vec2(0.0, 0.0);

    if (absnorm.x > absnorm.y && absnorm.x > absnorm.z) {
        // x major
        if (normal.x >= 0.0) {
            texCoords = position.yz * uvScale + offset;
            texCoords.y = 1.0 - texCoords.y;
        } else {
            texCoords = position.yz * uvScale + offset;
        }
    } else if (absnorm.y > absnorm.z) {
        // y major
        if (normal.y >= 0.0) {
            texCoords = position.zx * uvScale + offset;
            //texCoords.x = 1.0 - texCoords.x;
            texCoords.y = 1.0 - texCoords.y;
        } else {
            texCoords = position.xz * uvScale + offset;
            texCoords.y = 1.0 - texCoords.y;
        }
    } else {
        // z major
        if (normal.z >= 0.0) {
            texCoords = position.yx * uvScale + offset;
        } else {
            texCoords = position.yx * uvScale + offset;
            texCoords.y = 1.0 - texCoords.y;
        }
    }

    return texCoords;
}

void main() {
    vec4 worldPos = modelMatrix * vec4(vsIn_position, 1.0);

    //vec3 N = normalize(normalMatrix * vsIn_normal);
    //vec3 T = normalize(normalMatrix * vsIn_tangent.xyz);
    //vec3 B = cross(N, T);

    //vsOut.TBN = mat3(T, B, N);
    vsOut.normal = vsIn_normal;
    vsOut.worldpos = worldPos.xyz;
    vsOut.texCoords = boxProjection(vsIn_normal, vsIn_position);

    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
