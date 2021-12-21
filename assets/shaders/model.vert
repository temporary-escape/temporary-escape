in VS_IN {
    vec3 position;
    vec3 normal;
    vec2 texCoords;
    vec4 tangent;
} vs_in;

out VS_OUT {
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec3 worldpos;
} vs_out;

uniform mat3 normalMatrix;
uniform mat4 modelMatrix;

void main()
{
    vs_out.texCoords = vs_in.texCoords;

    vec4 worldPos = modelMatrix * vec4(vs_in.position.xyz, 1.0);

    vec3 N = normalize(normalMatrix * vs_in.normal);
    vec3 T = normalize(normalMatrix * vs_in.tangent.xyz);
    vec3 B = cross(N, T);

    vs_out.TBN = mat3(T, B, N);
    vs_out.normal = N.xyz;
    vs_out.worldpos = worldPos.xyz;

    gl_Position = transformationProjectionMatrix * worldPos;
}
