in VS_OUT {
    vec2 texCoords;
} vsOut;

layout(location = 0) out vec4 o_color;

uniform sampler2D colorTexture;
uniform sampler2D bloomTexture;
uniform float exposure = 1.0;

void main() {
    const float gamma = 2.2;
    vec4 raw = texture(colorTexture, vsOut.texCoords);
    vec3 hdrColor = raw.rgb;
    vec3 bloomColor = texture(bloomTexture, vsOut.texCoords).rgb;
    hdrColor += bloomColor.rgb * 1.5;// additive blending

    o_color = vec4(hdrColor, raw.a);
}
