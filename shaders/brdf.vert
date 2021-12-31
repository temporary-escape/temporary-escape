// Source: https://github.com/JoeyDeVries/LearnOpenGL/blob/master/src/6.pbr/2.2.2.ibl_specular_textured/2.2.2.brdf.vs

layout (location = 0) in vec2 vsIn_position;
layout (location = 1) in vec2 vsIn_texCoords;

out VS_OUT {
    vec2 texCoords;
} vsOut;

void main()
{
    vsOut.texCoords = vsIn_texCoords;
	gl_Position = vec4(vsIn_position, 0.0, 1.0);
}
