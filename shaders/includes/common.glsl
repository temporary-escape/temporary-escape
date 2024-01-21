struct SCamera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
};

struct SBlockMaterial {
    int baseColorTexture;
    int emissiveTexture;
    int normalTexture;
    int ambientOcclusionTexture;
    int metallicRoughnessTexture;
    int maskTexture;

    vec4 baseColorFactor;
    vec4 emissiveFactor;
    vec4 normalFactor;
    vec4 ambientOcclusionFactor;
    vec4 metallicRoughnessFactor;
};
