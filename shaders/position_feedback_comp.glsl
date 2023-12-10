#version 450

layout (local_size_x = 256) in;

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout (push_constant) uniform Uniforms {
    vec2 viewport;
    int count;
} uniforms;

struct InputData {
    vec4 position;
    vec2 size;
    int id;
};

struct OutputData {
    vec4 position;
    vec2 size;
    int id;
};

layout (binding = 1) readonly buffer InputBuffer {
    InputData objects[];
} comp_in;

layout (binding = 2) buffer OutputBuffer {
    OutputData objects[];
} comp_out;

void main() {
    uint gID = gl_GlobalInvocationID.x;

    if (gID < uniforms.count) {
        vec4 worldPos = vec4(comp_in.objects[gID].position.xyz, 1.0);
        vec4 clipSpace = camera.transformationProjectionMatrix * worldPos;
        vec3 ndcSpace = clipSpace.xyz / clipSpace.w;
        ndcSpace = vec3(ndcSpace.x, -ndcSpace.y, ndcSpace.z);
        vec2 result = ((ndcSpace.xy + 1.0) / 2.0) * uniforms.viewport;

        comp_out.objects[gID].position = vec4(result.x, uniforms.viewport.y - result.y, 0.0, 0.0);
        comp_out.objects[gID].size = comp_in.objects[gID].size;
        comp_out.objects[gID].id = comp_in.objects[gID].id;
    }
}
