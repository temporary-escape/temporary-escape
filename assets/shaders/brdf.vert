// Source: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/6.pbr/2.2.2.ibl_specular_textured/2.2.2.brdf.vs

layout (location = 0) in vec2 position;
layout (location = 1) in vec2 texCoords;

out vec2 v_texCoords;

void main()
{
    v_texCoords = texCoords;
	gl_Position = vec4(position, 0.0, 1.0);
}
