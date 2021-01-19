layout(location = 0) in vec2 position;

out vec2 v_texCoords;

void main() {
    vec2 coords = (position + 1.0) * 0.5;
    v_texCoords = vec2(coords.x, coords.y);
    gl_Position = vec4(position, 1.0, 1.0);
}
