layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoords;
layout(location = 3) in vec4 tangent;

uniform mat4 transformationProjectionMatrix;
uniform mat4 projectionMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelMatrix;

out vec3 v_normal;
out vec2 v_texCoords;
out mat3 v_TBN;
out vec3 v_worldpos;

void main()
{
    v_texCoords = texCoords;

    vec4 worldPos = modelMatrix * vec4(position.xyz, 1.0);

    vec3 N = normalize(normalMatrix * normal);
    vec3 T = normalize(normalMatrix * tangent.xyz);
    vec3 B = cross(N, T);

    v_TBN = mat3(T, B, N);
    v_normal = N.xyz;
    v_worldpos = worldPos.xyz;

    gl_Position = transformationProjectionMatrix * worldPos;
}
