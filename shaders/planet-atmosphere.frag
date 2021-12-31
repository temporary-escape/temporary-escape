in VS_OUT {
    vec3 normal;
    vec3 texCoords;
    vec3 worldPos;
} vsOut;

layout(location = 0) out vec4 fragmentColor;

layout (std140) uniform DirectionalLights {
    vec4 colors[4];
    vec4 directions[4];
    int count;
} directionalLights;

layout (std140) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

void main() {
    vec4 atmosphere = vec4(0.0);

    vec4 atmosphereColorStart = vec4(0.1, 0.7, 1.0, 1.0);
    vec4 atmosphereColorEnd = vec4(0.8, 0.9, 1.0, 1.0);

    // Light scattering
    for (int i = 0; i < directionalLights.count; ++i) {
        vec3 light = normalize(directionalLights.directions[i].xyz);
        vec3 radiance = directionalLights.colors[i].xyz;

        float d = clamp(dot(vsOut.normal, light), 0.0, 1.0);
        // d = pow(d, 1.0);

        vec3 eyesDir = normalize(camera.eyesPos - vsOut.worldPos);

        float f = 1.0 - clamp(dot(vsOut.normal, eyesDir), 0.0, 1.0);
        f = pow(f, 1.3);

        float factor = clamp(f * d, 0.0, 1.0);

        vec4 atmosphereColor = mix(atmosphereColorStart, atmosphereColorEnd, pow(f, 1.5));
        // atmosphereColor *= vec4(radiance, 1.0);

        vec4 scattering = atmosphereColor * vec4(vec3(1.0), factor * 1.0);
        atmosphere += scattering;
    }

    fragmentColor = atmosphere;
}
