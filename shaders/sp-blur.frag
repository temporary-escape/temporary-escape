in VS_OUT {
    vec2 texCoords;
} vsOut;

layout(location = 0) out vec4 o_color;

uniform sampler2D colorTexture;

void main()
{
    vec2 texelSize = 1.0 / vec2(textureSize(colorTexture, 0));
    vec3 result = vec3(0.0);
    for (int x = -2; x < 2; ++x)
    {
        for (int y = -2; y < 2; ++y)
        {
            vec2 offset = vec2(float(x), float(y)) * texelSize;
            result += texture(colorTexture, vsOut.texCoords + offset).rgb;
        }
    }
    o_color = vec4(result / (4.0 * 4.0), 1.0);
}
