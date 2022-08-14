layout (location = 0) in vec2 vsIn_position;
layout (location = 1) in vec2 vsIn_texCoords;

out VS_OUT {
    vec2 texCoords;
} vsOut;

void main() {
    vsOut.texCoords = vec2(vsIn_texCoords.x, 1.0 - vsIn_texCoords.y);
    gl_Position = vec4(vsIn_position, 1.0, 1.0);
}
