in VS_OUT {
    vec2 texCoords;
} vsOut;

layout(location = 0) out float o_color;

layout (std140) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

uniform sampler2D depthTexture;
uniform sampler2D colorTexture;
uniform sampler2D normalTexture;
uniform sampler2D noiseTexture;

int kernelSize = 64;
float radius = 0.5;
float bias = 0.025;

uniform vec3 samples[64];
uniform vec2 noiseScale;

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
    clipSpaceLocation.z = depth * 2.0 - 1.0;
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

void main() {
    vec3 color = texture(colorTexture, vsOut.texCoords).rgb;
    float depth = texture(depthTexture, vsOut.texCoords).r;
    vec3 fragPos = getViewPos(depth, vsOut.texCoords);
    vec3 worldNormal = texture(normalTexture, vsOut.texCoords).rgb;
    vec3 normal = vec4(camera.viewMatrix * vec4(worldNormal, 0.0f)).xyz;
    if (depth > 0.999) {
        normal = vec3(0.0, 0.0, 1.0);
    }
    // fragPos.z = depth;
    // screenSpaceNormal = mix(screenSpaceNormal, vec4(0.0, 0.0, 1.0, 1.0), depth);
    // o_color = vec4(normal.rgb, 1.0);

    vec3 randomVec = normalize(texture(noiseTexture, vsOut.texCoords * noiseScale).xyz);
    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float occlusion = 0.0;
    // int i = 8;

    for (int i = 0; i < kernelSize; ++i)
    {
        // get sample position
        vec3 samplePos = TBN * samples[i];// from tangent to view-space
        samplePos = fragPos + samplePos * radius;

        // project sample position (to sample texture) (to get position on screen/texture)
        vec4 offset = vec4(samplePos, 1.0);
        offset = camera.projectionMatrix * offset;// from view to clip-space
        offset.xyz /= offset.w;// perspective divide
        offset.xyz = offset.xyz * 0.5 + 0.5;// transform to range 0.0 - 1.0

        //offset.x = 1.0 - offset.x;
        //offset.y = 1.0 - offset.y;

        // get sample depth
        float sampleDepth = texture(depthTexture, offset.xy).r;// get depth value of kernel sample
        sampleDepth = getLinearDepth(sampleDepth);
        float ldepth = getLinearDepth(depth);

        // range check & accumulate
        float rangeCheck = smoothstep(0.0, 1.0, radius / abs(ldepth - sampleDepth));
        occlusion += (ldepth >= sampleDepth + bias ? 1.0 : 0.0) * rangeCheck;
    }
    occlusion = 1.0 - (occlusion / kernelSize);
    o_color = occlusion;
}
