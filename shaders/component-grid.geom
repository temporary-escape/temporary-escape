#version 450
#extension GL_ARB_separate_shader_objects : enable

layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

layout(push_constant) uniform Uniforms {
    mat4 modelMatrix;
    mat3 normalMatrix;
} uniforms;

layout(location = 0) in VS_OUT {
    vec3 normal;
    vec2 texCoords;
    vec3 worldpos;
} vs_out[];

layout(location = 0) out GS_OUT {
    vec3 normal;
    vec2 texCoords;
    mat3 TBN;
    vec3 worldpos;
} gs_out;

void main() {
    vec3 pos1 = vs_out[0].worldpos;
    vec3 pos2 = vs_out[1].worldpos;
    vec3 pos3 = vs_out[2].worldpos;

    vec2 uv1 = vs_out[0].texCoords;
    vec2 uv2 = vs_out[1].texCoords;
    vec2 uv3 = vs_out[2].texCoords;

    vec3 edge1 = pos2 - pos1;
    vec3 edge2 = pos3 - pos1;
    vec2 deltaUV1 = uv2 - uv1;
    vec2 deltaUV2 = uv3 - uv1;

    float f = 1.0f / (deltaUV1.x * deltaUV2.y - deltaUV2.x * deltaUV1.y);

    vec3 T;
    T.x = f * (deltaUV2.y * edge1.x - deltaUV1.y * edge2.x);
    T.y = f * (deltaUV2.y * edge1.y - deltaUV1.y * edge2.y);
    T.z = f * (deltaUV2.y * edge1.z - deltaUV1.y * edge2.z);

    vec3 N = vs_out[0].normal;
    vec3 B = cross(N, T);
    mat3 TBN = mat3(T, B, N);

    gl_Position = gl_in[0].gl_Position;
    gs_out.normal = vs_out[0].normal;
    gs_out.texCoords = vs_out[0].texCoords;
    gs_out.TBN = TBN;
    gs_out.worldpos = vs_out[0].worldpos;
    EmitVertex();

    N = vs_out[1].normal;
    B = cross(N, T);
    TBN = mat3(T, B, N);

    gl_Position = gl_in[1].gl_Position;
    gs_out.normal = vs_out[1].normal;
    gs_out.texCoords = vs_out[1].texCoords;
    gs_out.TBN = TBN;
    gs_out.worldpos = vs_out[1].worldpos;
    EmitVertex();

    N = vs_out[2].normal;
    B = cross(N, T);
    TBN = mat3(T, B, N);

    gl_Position = gl_in[2].gl_Position;
    gs_out.normal = vs_out[2].normal;
    gs_out.texCoords = vs_out[2].texCoords;
    gs_out.TBN = TBN;
    gs_out.worldpos = vs_out[2].worldpos;
    EmitVertex();

    EndPrimitive();
}