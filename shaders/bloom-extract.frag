in VS_OUT {
    vec2 texCoords;
} vsOut;

layout(location = 0) out vec3 o_color;

uniform sampler2D inputTexture;
uniform float brightness;

void main() {
    vec3 color = texture(inputTexture, vsOut.texCoords).rgb;
    float test = dot(color.rgb, vec3(0.2126, 0.7152, 0.0722));
    if (test > 0.7) {
        o_color = color;
    } else {
        o_color = vec3(0.0);
    }
}
