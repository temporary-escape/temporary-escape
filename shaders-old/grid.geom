layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec3 normal;
    vec2 texCoords;
//mat3 TBN;
    vec3 worldpos;
} vsOut[];

out GS_OUT {
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec3 worldpos;
} gsOut;

uniform mat3 normalMatrix;
uniform mat4 modelMatrix;

void main() {
    vec3 pos1 = gl_in[0].gl_Position.xyz;
    vec3 pos2 = gl_in[1].gl_Position.xyz;
    vec3 pos3 = gl_in[2].gl_Position.xyz;

    vec2 uv1 = vsOut[0].texCoords;
    vec2 uv2 = vsOut[1].texCoords;
    vec2 uv3 = vsOut[2].texCoords;

    vec3 edge1 = pos2 - pos1;
    vec3 edge2 = pos3 - pos1;
    vec2 deltaUV1 = uv2 - uv1;
    vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    vec3 T;
    T.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    T.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    T.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    vec3 N = vsOut[0].normal;
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    gl_Position = gl_in[0].gl_Position;
    gsOut.normal = vsOut[0].normal;
    gsOut.texCoords = vsOut[0].texCoords;
    gsOut.TBN = TBN;
    gsOut.worldpos = vsOut[0].worldpos;
    EmitVertex();

    N = vsOut[1].normal;
    B = cross(N, T);
    TBN = mat3(T, B, N);

    gl_Position = gl_in[1].gl_Position;
    gsOut.normal = vsOut[1].normal;
    gsOut.texCoords = vsOut[1].texCoords;
    gsOut.TBN = TBN;
    gsOut.worldpos = vsOut[1].worldpos;
    EmitVertex();

    N = vsOut[2].normal;
    B = cross(N, T);
    TBN = mat3(T, B, N);

    gl_Position = gl_in[2].gl_Position;
    gsOut.normal = vsOut[2].normal;
    gsOut.texCoords = vsOut[2].texCoords;
    gsOut.TBN = TBN;
    gsOut.worldpos = vsOut[2].worldpos;
    EmitVertex();

    EndPrimitive();
}