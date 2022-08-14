layout(location = 0) out vec4 o_color;

in VS_OUT {
    vec4 color;
} vsOut;

void main()
{
    o_color = vsOut.color;
}
