layout(location = 0) in vec3 position;

out vec3 v_texcoords;

uniform mat4 projection;
uniform mat4 view;

void main() {
    v_texcoords = position;
    vec4 pos = projection * view * vec4(position * 1.0, 1.0);
    gl_Position = pos.xyww;
}
