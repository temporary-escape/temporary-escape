layout(location = 0) out vec4 o_color;

in GS_OUT {
    vec4 color;
} gsOut;

void main()
{
    o_color = gsOut.color;
}