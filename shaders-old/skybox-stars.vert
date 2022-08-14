layout(location = 0) in vec3 position;
layout(location = 1) in float brightness;
layout(location = 2) in vec4 color;

uniform mat4 viewMatrix;

out float g_brightness;
out vec4 g_color;

void main() {
    g_brightness = brightness;
    g_color = color;
    gl_Position = viewMatrix * vec4(position, 1.0);
}
