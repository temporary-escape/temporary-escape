layout(location = 0) in vec3 position;

uniform mat4 transformationProjectionMatrix;
uniform mat4 modelMatrix;
uniform vec3 eyesPos;

void main()
{
    vec4 worldPos = modelMatrix * vec4(position.xyz, 1.0);
    vec3 viewDir = normalize(worldPos.xyz - eyesPos);
    worldPos += vec4(viewDir * -0.025, 0.0);
    gl_Position = transformationProjectionMatrix * worldPos;
}
