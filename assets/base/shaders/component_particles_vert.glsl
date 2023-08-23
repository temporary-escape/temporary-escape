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

layout (std140, binding = 1) uniform ParticlesType {
    vec4 startColor;
    vec4 endColor;
    float duration;
    int count;
    vec3 direction;
    vec3 startSpawn;
    vec3 endSpawn;
    vec2 startSize;
    vec2 endSize;
} particlesType;

layout (push_constant) uniform Uniforms {
    mat4 modelMatrix;
    float timeDelta;
} uniforms;

layout (location = 0) out VS_OUT {
    vec2 texCoords;
    vec4 color;
} vs_out;

out gl_PerVertex {
    vec4 gl_Position;
};

// Simplex Noise 2D
//      Author : Ian McEwan, Ashima Arts.
//  Maintainer : ijm
//     Lastmod : 20110822 (ijm)
//     License : Copyright (C) 2011 Ashima Arts. All rights reserved.
//               Distributed under the MIT License. See LICENSE file.
//               https://github.com/ashima/webgl-noise
//
vec3 mod289(vec3 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec2 mod289(vec2 x) {
    return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec3 permute(vec3 x) {
    return mod289(((x*34.0)+1.0)*x);
}

float snoise(vec2 v) {
    const vec4 C = vec4(0.211324865405187, // (3.0-sqrt(3.0))/6.0
    0.366025403784439, // 0.5*(sqrt(3.0)-1.0)
    -0.577350269189626, // -1.0 + 2.0 * C.x
    0.024390243902439);// 1.0 / 41.0
    // First corner
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v -   i + dot(i, C.xx);

    // Other corners
    vec2 i1;
    //i1.x = step( x0.y, x0.x ); // x0.x > x0.y ? 1.0 : 0.0
    //i1.y = 1.0 - i1.x;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    // x0 = x0 - 0.0 + 0.0 * C.xx ;
    // x1 = x0 - i1 + 1.0 * C.xx ;
    // x2 = x0 - 1.0 + 2.0 * C.xx ;
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;

    // Permutations
    i = mod289(i);// Avoid truncation effects in permutation
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
    + i.x + vec3(0.0, i1.x, 1.0));

    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy), dot(x12.zw, x12.zw)), 0.0);
    m = m*m;
    m = m*m;

    // Gradients: 41 points uniformly over a line, mapped onto a diamond.
    // The ring size 17*17 = 289 is close to a multiple of 41 (41*7 = 287)

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt( a0*a0 + h*h );
    m *= 1.79284291400159 - 0.85373472095314 * (a0*a0 + h*h);

    // Compute final noise value at P
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

void geometrize(vec4 pos, float time) {
    vs_out.color = mix(particlesType.startColor, particlesType.endColor, time);

    vec2 particleSize = mix(particlesType.startSize, particlesType.endSize, time);
    particleSize.x /= float(camera.viewport.x) / float(camera.viewport.y);

    // a: left-bottom
    if (gl_VertexIndex % 6 == 0) {
        vec2 va = pos.xy + vec2(-1.0, -1.0) * particleSize;
        gl_Position = vec4(va, pos.zw);
        vs_out.texCoords = vec2(0.0, 0.0);
    }

    // b: left-top
    if (gl_VertexIndex % 6 == 1) {
        vec2 vb = pos.xy + vec2(-1.0, 1.0) * particleSize;
        gl_Position = vec4(vb, pos.zw);
        vs_out.texCoords = vec2(0.0, 1.0);
    }

    // d: right-bottom
    if (gl_VertexIndex % 6 == 2) {
        vec2 vd = pos.xy + vec2(1.0, -1.0) * particleSize;
        gl_Position = vec4(vd, pos.zw);
        vs_out.texCoords = vec2(1.0, 0.0);
    }

    // c: right-top
    if (gl_VertexIndex % 6 == 3) {
        vec2 vc = pos.xy + vec2(1.0, 1.0) * particleSize;
        gl_Position = vec4(vc, pos.zw);
        vs_out.texCoords = vec2(1.0, 1.0);
    }
}

void main() {
    float step = particlesType.duration / float(particlesType.count);
    int index = gl_InstanceIndex;

    float time = mod(uniforms.timeDelta + step * index, particlesType.duration) * (1.0 / particlesType.duration);

    vec3 spawn = mix(particlesType.startSpawn, particlesType.endSpawn, time);
    // vec3 spawn = particlesType.startSpawn;

    float ox = snoise(vec2(float(gl_InstanceIndex), 1.0)) * spawn.x;
    float oy = snoise(vec2(float(gl_InstanceIndex), 10.0)) * spawn.y;
    float oz = snoise(vec2(float(gl_InstanceIndex), 100.0)) * spawn.z;
    // vec3 modelPos = in_Position + in_Direction * (time * distance);
    // modelPos.x += index * 0.5;
    // vec3 modelPos = in_Position + in_Direction * index;
    vec3 direction = (uniforms.modelMatrix * vec4(particlesType.direction, 0.0)).xyz;
    vec4 worldPos = uniforms.modelMatrix * vec4(ox, oy, oz, 1.0);
    worldPos += vec4(direction * time, 0.0);
    geometrize(camera.transformationProjectionMatrix * worldPos, time);
}
