in VS_OUT {
    vec2 texCoords;
} vsOut;

layout(location = 0) out vec3 o_color;

uniform sampler2D colorTexture;
uniform sampler2D bloomTexture;
uniform float exposure = 1.0;

void main() {
    const float gamma = 2.2;
    vec3 hdrColor = texture(colorTexture, vsOut.texCoords).rgb;
    vec3 bloomColor = texture(bloomTexture, vsOut.texCoords).rgb;
    hdrColor += bloomColor * 1.5;// additive blending

    o_color = hdrColor;
}
