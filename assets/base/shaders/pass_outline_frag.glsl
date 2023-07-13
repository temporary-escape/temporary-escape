#version 450
#extension GL_ARB_separate_shader_objects: enable

layout (location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout (push_constant) uniform Uniforms {
    vec4 selectedColor;
    vec4 finalColor;
} uniforms;

layout (location = 0) out vec4 outColor;

layout (binding = 0) uniform sampler2D entityColorTexture;

float thickness = 0.5;

float sampleAt(ivec2 coords) {
    vec4 entityColor = texelFetch(entityColorTexture, coords, 0);

    if (entityColor == uniforms.selectedColor) {
        return 1.0;
    } else {
        return 0.0;
    }
}

void main() {
    ivec2 size = textureSize(entityColorTexture, 0);

    float stepX = vs_out.texCoords.x * size.x;
    float stepY = vs_out.texCoords.y * size.y;

    float sum = 0.0;
    for (int n = 0; n < 9; ++n) {
        stepY = (vs_out.texCoords.y * size.y) + (thickness * float(n - 4.5));
        float h_sum = 0.0;
        h_sum += sampleAt(ivec2(stepX - (4.0 * thickness), stepY));
        h_sum += sampleAt(ivec2(stepX - (3.0 * thickness), stepY));
        h_sum += sampleAt(ivec2(stepX - (2.0 * thickness), stepY));
        h_sum += sampleAt(ivec2(stepX - thickness, stepY));
        h_sum += sampleAt(ivec2(stepX, stepY));
        h_sum += sampleAt(ivec2(stepX + thickness, stepY));
        h_sum += sampleAt(ivec2(stepX + (2.0 * thickness), stepY));
        h_sum += sampleAt(ivec2(stepX + (3.0 * thickness), stepY));
        h_sum += sampleAt(ivec2(stepX + (4.0 * thickness), stepY));
        sum += h_sum / 9.0;
    }

    sum = clamp(sum / 9.0, 0.0, 1.0);
    if (sum > 0.5) {
        sum = 0.5 - (sum - 0.5);
    }
    sum *= 2.0;

    outColor = vec4(uniforms.finalColor.rgb, uniforms.finalColor.a * sum);
}
