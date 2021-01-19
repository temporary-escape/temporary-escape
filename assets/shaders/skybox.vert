layout(location = 0) in vec3 position;

out vec3 v_texCoords;

uniform mat4 modelMatrix;
uniform mat4 transformationProjectionMatrix;

void main() {
    v_texCoords = position;
    vec4 worldPos = modelMatrix * vec4(position, 1.0);
    gl_Position = transformationProjectionMatrix * worldPos;
}
