#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(location = 0) in VS_OUT {
    vec2 texCoords;
} vs_out;

layout(push_constant) uniform Uniforms {
    float resolution;
    float waterLevel;
} uniforms;

layout(location = 0) out vec4 outColor;

layout(binding = 0) uniform sampler2D textureHeightmap;

void main() {
    float x = vs_out.texCoords.x;
    float y = vs_out.texCoords.y;
    float pixelSize = 1.0 / uniforms.resolution;

    float tl = texture(textureHeightmap, vec2(x-pixelSize, y-pixelSize)).r;
    float l = texture(textureHeightmap, vec2(x-pixelSize, y)).r;
    float bl = texture(textureHeightmap, vec2(x-pixelSize, y+pixelSize)).r;
    float b = texture(textureHeightmap, vec2(x, y+pixelSize)).r;
    float br = texture(textureHeightmap, vec2(x+pixelSize, y+pixelSize)).r;
    float r = texture(textureHeightmap, vec2(x+pixelSize, y)).r;
    float tr = texture(textureHeightmap, vec2(x+pixelSize, y-pixelSize)).r;
    float t = texture(textureHeightmap, vec2(x, y-pixelSize)).r;

    // Compute dx using Sobel:
    //           -1 0 1
    //           -2 0 2
    //           -1 0 1
    float dX = tr + 2.0 * r + br - tl - 2.0 * l - bl;

    // Compute dy using Sobel:
    //           -1 -2 -1
    //            0  0  0
    //            1  2  1
    float dY = bl + 2.0 * b + br - tl - 2.0 * t - tr;

    float strength = 0.8;
    vec3 normal = normalize(vec3(dX, dY, 1.0 / strength));

    outColor = vec4(normal * 0.5 + 0.5, 1.0);
}
