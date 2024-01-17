#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout (push_constant) uniform Uniforms {
    mat4 modelMatrix;
    vec3 moveDirection;
} uniforms;

layout (location = 0) in vec3 in_Position;

layout (location = 0) out VS_OUT {
    vec2 texCoords;
    float alpha;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

const int totalInBlock = 1024;

float map(float value, float min1, float max1, float min2, float max2) {
    return min2 + (value - min1) * (max2 - min2) / (max1 - min1);
}

void geometrize(vec4 pos) {
    vec2 particleSize = vec2(0.05, 0.05);
    particleSize.x /= float(camera.viewport.x) / float(camera.viewport.y);

    // a: left-bottom
    if (gl_VertexIndex % 6 == 0) {
        vec2 va = pos.xy + vec2(-1.0, -1.0) * particleSize;
        gl_Position = vec4(va, pos.zw);
        vs_out.texCoords = vec2(0.0, 0.0);
    }

    // b: left-top
    if (gl_VertexIndex % 6 == 1 || gl_VertexIndex % 6 == 4) {
        vec2 vb = pos.xy + vec2(-1.0, 1.0) * particleSize;
        gl_Position = vec4(vb, pos.zw);
        vs_out.texCoords = vec2(0.0, 1.0);
    }

    // d: right-bottom
    if (gl_VertexIndex % 6 == 2 || gl_VertexIndex % 6 == 3) {
        vec2 vd = pos.xy + vec2(1.0, -1.0) * particleSize;
        gl_Position = vec4(vd, pos.zw);
        vs_out.texCoords = vec2(1.0, 0.0);
    }

    // c: right-top
    if (gl_VertexIndex % 6 == 5) {
        vec2 vc = pos.xy + vec2(1.0, 1.0) * particleSize;
        gl_Position = vec4(vc, pos.zw);
        vs_out.texCoords = vec2(1.0, 1.0);
    }
}

void main() {
    vec4 worldPos = uniforms.modelMatrix * vec4(in_Position, 1.0);

    float dist = distance(worldPos.xyz, camera.eyesPos);
    vs_out.alpha = clamp(map(dist, 0.0, 16.0, 0.0, 1.0), 0.0, 1.0);
    vs_out.alpha *= 0.02;

    float offset = float(int(gl_VertexIndex / (1024 * 6))) / 16.0;
    worldPos += vec4(uniforms.moveDirection * offset, 0.0);

    geometrize(camera.transformationProjectionMatrix * worldPos);
}
