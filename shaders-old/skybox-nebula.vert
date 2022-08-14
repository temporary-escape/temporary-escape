layout(location = 0) in vec3 position;

uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;

out vec3 v_position;

void main() {
    vec4 worldPos = vec4(position, 1);
    v_position = worldPos.xyz;
    gl_Position = projectionMatrix * viewMatrix * worldPos;
}
