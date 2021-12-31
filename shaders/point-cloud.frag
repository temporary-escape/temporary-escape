layout(location = 0) out vec4 o_color;

in vec2 v_coords;
in vec4 v_color;

uniform sampler2D pointTexture;

void main()
{
    o_color = texture(pointTexture, v_coords) * v_color;
}
