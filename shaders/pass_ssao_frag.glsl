#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout (std140, binding = 0) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout (std140, binding = 1) uniform Samples {
    vec4 weights[64];
} samples;

layout (push_constant) uniform Uniforms {
    vec2 scale;
    int kernelSize;
} uniforms;

layout (location = 0) out vec4 outColor;

layout (binding = 2) uniform sampler2D depthTexture;
layout (binding = 3) uniform sampler2D normalTexture;
layout (binding = 4) uniform sampler2D noiseTexture;

const float radius = 0.5;
const float bias = 0.025;

// ----------------------------------------------------------------------------
float getLinearDepth(float depth) {
    const float zFar = 2000.0;
    const float zNear = 0.1;

    // bias it from [0, 1] to [-1, 1]
    float linear = zNear / (zFar - depth * (zFar - zNear)) * zFar;

    return (linear * 2.0) - 1.0;
}

// ----------------------------------------------------------------------------
vec4 getClipSpacePos(float depth, vec2 texCoords) {
    // Source: https://stackoverflow.com/questions/22360810/reconstructing-world-coordinates-from-depth-buffer-and-arbitrary-view-projection
    vec4 clipSpaceLocation;
    clipSpaceLocation.xy = texCoords * 2.0f - 1.0f;
    clipSpaceLocation.z = depth;
    clipSpaceLocation.w = 1.0f;

    return clipSpaceLocation;
}

// ----------------------------------------------------------------------------
vec3 getViewPos(float depth, vec2 texCoords) {
    // Source: https://stackoverflow.com/questions/11277501/how-to-recover-view-space-position-given-view-space-depth-value-and-ndc-xy
    vec4 clipSpaceLocation = getClipSpacePos(depth, texCoords);
    vec4 homogenousLocation = inverse(camera.projectionMatrix) * clipSpaceLocation;
    vec3 viewPos = homogenousLocation.xyz / homogenousLocation.w;
    // viewPos.z = getLinearDepth(depth);
    return viewPos;
}

// ----------------------------------------------------------------------------
void main() {
    float depth = texture(depthTexture, vs_out.texCoords).r;
    vec3 fragPos = getViewPos(depth, vs_out.texCoords);
    vec3 worldNormal = texture(normalTexture, vs_out.texCoords).rgb * 2.0 - 1.0;
    vec3 normal = vec4(camera.viewMatrix * vec4(worldNormal, 0.0f)).xyz;
    if (depth > 0.999) {
        outColor = vec4(1.0);
        discard;
    }

    vec3 randomVec = normalize(texture(noiseTexture, vs_out.texCoords * uniforms.scale).xyz);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;

    float ldepth = getLinearDepth(depth);

    for (int i = 0; i < uniforms.kernelSize; ++i) {
        // get sample position
        vec3 samplePos = TBN * samples.weights[i].rgb;// from tangent to view-space
        samplePos = fragPos + samplePos * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = camera.projectionMatrix * offset;// from view to clip-space
        offset.xyz /= offset.w;// perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5;// transform to range 0.0 - 1.0

        // get sample depth
        float sampleDepth = texture(depthTexture, offset.xy).r;// get depth value of kernel sample
        sampleDepth = getLinearDepth(sampleDepth);

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(ldepth - sampleDepth));
        occlusion += (ldepth >= sampleDepth + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / uniforms.kernelSize);
    outColor = vec4(vec3(occlusion), 1.0);
}
