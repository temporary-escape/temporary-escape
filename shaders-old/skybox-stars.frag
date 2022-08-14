layout(location = 0) out vec4 fragmentColor;

in float v_brightness;
in vec4 v_color;
in vec2 v_coords;

void main() {
    float dist = pow(clamp(1.0 - length(v_coords), 0.0, 1.0), 0.5);
    fragmentColor = v_color * dist * v_brightness;
}
