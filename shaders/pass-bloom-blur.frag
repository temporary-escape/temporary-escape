#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout (std140, binding = 0) uniform GaussianWeights {
    float weight[32];
    int count;
} gaussianWeights;

layout(push_constant) uniform Uniforms {
    int horizontal;
} uniforms;

layout(binding = 1) uniform sampler2D texSourceColor;

layout(location = 0) out vec4 outColor;

void main() {
    vec2 tex_offset = (1.0 / textureSize(texSourceColor, 0)) * 1.0;// gets size of single texel
    vec3 result = texture(texSourceColor, vs_out.texCoords).rgb * gaussianWeights.weight[0];// current fragment's contribution
    if (uniforms.horizontal > 0)
    {
        for (int i = 1; i < gaussianWeights.count; ++i)
        {
            result += texture(texSourceColor, vs_out.texCoords + vec2(tex_offset.x * i, 0.0)).rgb * gaussianWeights.weight[i];
            result += texture(texSourceColor, vs_out.texCoords - vec2(tex_offset.x * i, 0.0)).rgb * gaussianWeights.weight[i];
        }
    }
    else
    {
        for (int i = 1; i < gaussianWeights.count; ++i)
        {
            result += texture(texSourceColor, vs_out.texCoords + vec2(0.0, tex_offset.y * i)).rgb * gaussianWeights.weight[i];
            result += texture(texSourceColor, vs_out.texCoords - vec2(0.0, tex_offset.y * i)).rgb * gaussianWeights.weight[i];
        }
    }
    outColor = vec4(result, 1.0);
}
