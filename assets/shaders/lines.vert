layout(location = 0) in vec3 position;
layout(location = 1) in float dummy;
layout(location = 2) in vec4 color;

uniform mat4 projectionViewMatrix;
uniform mat4 modelMatrix;

out vec4 v_color;

void main()
{
    vec4 worldPos = modelMatrix * vec4(position.xyz, 1.0);

    v_color = color;

    gl_Position = projectionViewMatrix * worldPos;
}
