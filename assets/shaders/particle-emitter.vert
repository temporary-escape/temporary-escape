uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
uniform mat4 projectionMatrix;

uniform vec3 position;

void main()
{
    vec4 worldPos = modelMatrix * vec4(position.xyz, 1.0);

    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
