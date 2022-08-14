#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (std140, binding = 0) uniform CameraMatrices {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout(push_constant) uniform Uniforms {
    mat4 modelMatrix;
    mat3 normalMatrix;
} uniforms;

layout(location = 0) in vec3 in_Position;
layout(location = 1) in vec3 in_Normal;
layout(location = 2) in vec4 in_Tangent;

layout(location = 0) out VS_OUT {
    vec3 normal;
    vec2 texCoords;
    vec3 worldpos;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

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
    vec4 worldPos = uniforms.modelMatrix * vec4(in_Position, 1.0);
    vs_out.normal = uniforms.normalMatrix * in_Normal;
    vs_out.worldpos = worldPos.xyz;
    vs_out.texCoords = boxProjection(in_Normal, in_Position);
    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
