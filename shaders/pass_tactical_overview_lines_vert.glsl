#version 450
#extension GL_ARB_separate_shader_objects: enable
#include "includes/common.glsl"

layout (std140, binding = 0) uniform Camera {
    SCamera camera;
};

layout (push_constant) uniform Uniforms {
    mat4 modelMatrix;
    vec4 color;
    vec3 playerPos;
} uniforms;

layout (location = 0) in vec3 in_Position;
layout (location = 1) in vec2 in_Size;
layout (location = 2) in vec4 in_Color;
layout (location = 3) in vec2 in_Uv;
layout (location = 4) in vec2 in_St;
layout (location = 5) in vec2 in_Offset;

layout (location = 0) out VS_OUT {
    vec4 color;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

void main() {
    vs_out.color = in_Color * uniforms.color;

    vec3 modelPos = in_Position;

    if (gl_VertexIndex == 0) {
        modelPos.y = uniforms.playerPos.y;
    }

    vec4 worldPos = uniforms.modelMatrix * vec4(modelPos, 1.0);
    gl_Position = camera.transformationProjectionMatrix * worldPos;
}
