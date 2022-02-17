layout (std140) uniform Camera {
    mat4 transformationProjectionMatrix;
    mat4 viewProjectionInverseMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
    ivec2 viewport;
    vec3 eyesPos;
} camera;

layout(std140) uniform ParticleData {
    vec4 startColor;
    vec4 endColor;
    vec3 offset;
    vec3 force;
    float startRadius;
    float endRadius;
    int count;
    float duration;
    float startSize;
    float endSize;
} data;

out VS_OUT {
    float time;
} vsOut;

uniform mat4 modelMatrix;
uniform float time;

// Simplex Noise https://gist.github.com/patriciogonzalezvivo/670c22f3966e662d2f83#simplex-noise
vec3 permute(vec3 x) { return mod(((x*34.0)+1.0)*x, 289.0); }

float snoise(vec2 v){
    const vec4 C = vec4(0.211324865405187, 0.366025403784439,
    -0.577350269189626, 0.024390243902439);
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v -   i + dot(i, C.xx);
    vec2 i1;
    i1 = (x0.x > x0.y) ? vec2(1.0, 0.0) : vec2(0.0, 1.0);
    vec4 x12 = x0.xyxy + C.xxzz;
    x12.xy -= i1;
    i = mod(i, 289.0);
    vec3 p = permute(permute(i.y + vec3(0.0, i1.y, 1.0))
    + i.x + vec3(0.0, i1.x, 1.0));
    vec3 m = max(0.5 - vec3(dot(x0, x0), dot(x12.xy, x12.xy),
    dot(x12.zw, x12.zw)), 0.0);
    m = m*m;
    m = m*m;
    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;
    m *= 1.79284291400159 - 0.85373472095314 * (a0*a0 + h*h);
    vec3 g;
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * x12.xz + h.yz * x12.yw;
    return 130.0 * dot(m, g);
}

void main() {
    // Calculate time of this particle
    float t = mod(time + (data.duration / float(data.count)) * gl_VertexID, data.duration);

    // Uniform time
    float tu = t / data.duration;
    vsOut.time = tu;

    // Random start position
    vec3 randomPos = vec3(
    snoise(vec2(gl_VertexID, 0.0)),
    snoise(vec2(gl_VertexID, 1000.0)),
    snoise(vec2(gl_VertexID, 9000.0))
    );

    // Particles start in a random position and move towards the "end"
    vec3 startPos = randomPos * data.startRadius;
    vec3 endPos = randomPos * data.endRadius;
    vec3 position = mix(startPos, endPos, tu) + data.offset;

    // Apply directional force
    position += tu * data.force;

    // To world position
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    gl_Position = worldPos;
}
