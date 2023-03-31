#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec3 texcoords;
} vs_out;

layout(binding = 1) uniform samplerCube texSkybox;

layout(location = 0) out vec4 outColor;

const float PI = 3.14159265359;

// The following is an implementation of
// https://github.com/JoeyDeVries/LearnOpenGL
void main() {
    vec3 N = normalize(vs_out.texcoords);
    vec3 irradiance = vec3(0.0);

    // tangent space calculation from origin point
    vec3 up = vec3(0.0, 1.0, 0.0);
    vec3 right = cross(up, N);
    up = cross(N, right);

    float sampleDelta = 0.025;
    float nrSamples = 0.0f;
    for (float phi = 0.0; phi < 2.0 * PI; phi += sampleDelta) {
        for (float theta = 0.0; theta < 0.5 * PI; theta += sampleDelta) {
            // spherical to cartesian (in tangent space)
            vec3 tangentSample = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
            // tangent space to world
            vec3 sampleVec = tangentSample.x * right + tangentSample.y * up + tangentSample.z * N;

            irradiance += texture(texSkybox, sampleVec).rgb * cos(theta) * sin(theta);
            nrSamples++;
        }
    }

    irradiance = PI * irradiance * (1.0 / float(nrSamples));

    outColor = vec4(irradiance, 1.0);
}
