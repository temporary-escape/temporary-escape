layout(location = 0) out vec4 o_color;

in GS_OUT {
    vec4 color;
    vec2 texCoords;
} gsOut;

uniform sampler2D pointTexture;

void main()
{
    o_color = texture(pointTexture, gsOut.texCoords) * gsOut.color;
}
