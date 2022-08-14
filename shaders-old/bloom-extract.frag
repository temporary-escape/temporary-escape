in VS_OUT {
    vec2 texCoords;
} vsOut;

layout(location = 0) out vec4 o_color;

uniform sampler2D inputTexture;
uniform float brightness;

void main() {
    vec4 raw = texture(inputTexture, vsOut.texCoords);
    float alpha = raw.a;
    vec3 color = raw.rgb;
    float test = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (test > 0.7) {
        o_color = vec4(color, alpha);
    } else {
        o_color = vec4(vec3(0.0), alpha);
    }
}
