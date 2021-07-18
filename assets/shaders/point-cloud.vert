layout(location = 0) in vec3 position;
layout(location = 1) in float size;
layout(location = 2) in vec4 color;

uniform mat4 viewMatrix;
uniform mat4 modelMatrix;

out vec4 g_color;
out float g_size;

void main()
{
    vec4 worldPos = modelMatrix * vec4(position.xyz, 1.0);

    g_color = color;
    g_size = size;

    gl_Position = viewMatrix * worldPos;
}
